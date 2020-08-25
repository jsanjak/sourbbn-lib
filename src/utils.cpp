#include <string>
#include <vector>
#include <utility>  
#include <functional>
#include <iterator>
#include <algorithm>
#include <iostream>
#include <math.h>

#include "sourbbn/cptable.hpp"

namespace sourbbn {

int standard_sqlite_callback(void* data, int argc, char** argv, char** azColName) 
{ 

    int i; 
    std::vector<std::string> *p = static_cast<std::vector<std::string>*>(data);
    for (i = 0; i < argc; i++) { 
        p->push_back(argv[i]);
    } 
    return 0; 
} 

void print_row(RowValue &row, bool print){
    for( auto && nm : (*row.m_schema).field_names()){
        std::cout << nm << " ";
    };
    for (int j=0; j<(*row.m_schema).m_fields.size(); j++) {

        if ((*row.m_schema).get(j).get_type() == sourbbn::FieldType::Integer){

            std::cout << std::to_string(row.get(j).m_integer) << " ";
        
        } else if ((*row.m_schema).get(j).get_type() == sourbbn::FieldType::FloatingPoint) {

            std::cout << row.get(j).m_floatingpoint << " ";

        } else if ((*row.m_schema).get(j).get_type() == sourbbn::FieldType::Boolean) {
        
            std::cout << std::to_string( (int) row.get(j).m_boolean) << " ";

        };

    };
    std::cout << std::endl;
};

void print_cptable(CPTable &cp_table, bool print){

    if (print){
        std::cout << "Table data are:" << std::endl;
    };
    for( auto && nm : cp_table.m_schema.field_names()){
        std::cout << nm << " ";
    }
    std::cout << std::endl;
    for (auto & row: cp_table.m_rows) {

        for (int j=0; j<cp_table.m_schema.m_fields.size(); j++) {

            if (cp_table.m_schema.get(j).get_type() == sourbbn::FieldType::Integer){

                std::cout << std::to_string(row.get(j).m_integer) << " ";
            
            } else if (cp_table.m_schema.get(j).get_type() == sourbbn::FieldType::FloatingPoint) {

                std::cout << row.get(j).m_floatingpoint << " ";

            } else if (cp_table.m_schema.get(j).get_type() == sourbbn::FieldType::Boolean) {
            
                std::cout << std::to_string( (int) row.get(j).m_boolean) << " ";

            };
  
        };
        std::cout << std::endl;
	};

};


std::vector<std::string> scheme_diff(RowSchema & s1, RowSchema & s2 ){

    std::vector<std::string> s1_names =  s1.field_names();
    std::vector<std::string> s2_names =  s2.field_names();

    std::vector<std::string> diff_names = {};
    std::vector<std::string>::iterator it;

    for(auto && s1_var : s1_names){
        
        it = find(s2_names.begin(),s2_names.end(),s1_var);
        if ( it == s2_names.end() ){
            diff_names.push_back(s1_var);
        }
    }
    return(diff_names);
};

std::vector<std::string> scheme_overlap(RowSchema & s1, RowSchema & s2 ){

    std::vector<std::string> s1_names =  s1.field_names();
    std::vector<std::string> s2_names =  s2.field_names();

    std::vector<std::string> overlap_names = {};
    std::vector<std::string>::iterator it;

    for(auto && s1_var : s1_names){
        
        it = find(s2_names.begin(),s2_names.end(),s1_var);
        if ( it != s2_names.end() ){
            overlap_names.push_back(s1_var);
        }
    }
    return(overlap_names);
};

//TODO: this is currently very fragile
CPTable expand(CPTable & t1, CPTable & t2){
    
    RowSchema t1_schema = t1.m_schema;
    RowSchema t2_schema = t2.m_schema;

    RowSchema t1_scheme = t1.scheme();
    RowSchema t2_scheme = t2.scheme();
    std::string p_string = "p";
    int t1_p_index = 0;
    FieldValue f1_val;

    std::vector<std::string> overlap_vars =  scheme_overlap(t2_scheme,t1_scheme);
    std::vector<std::string> expand_vars =  scheme_diff(t2_scheme,t1_scheme);

    //a little sloppy, but okay
    CPTable new_table = CPTable(t2);
    
    std::size_t match_indicator;
    std::vector<RowValue>::iterator m_row_it;
    int ov_t1;
    int ov_t2;
    for ( m_row_it = new_table.m_rows.begin();  m_row_it != new_table.m_rows.end(); ++m_row_it){

        for (auto && cp_row_t1 : t1.m_rows){
            
            match_indicator = 0;
            for (auto && ov_var : overlap_vars){

                ov_t1 = t1_schema.get_index(ov_var);
                ov_t2 = t2_schema.get_index(ov_var);

                auto t1_val = cp_row_t1.get(ov_t1);
                auto t2_val = (*m_row_it).get(ov_t2);
                
                if(t1_val.m_integer == t2_val.m_integer){
                    match_indicator += 1;
                }
            }
            if(overlap_vars.size() == match_indicator ){
                f1_val = cp_row_t1.get(t1_p_index);
                (*m_row_it).reassign_field(p_string,f1_val);
            }
        }
    }
    return(new_table);
};

//TODO: ELIM ERROR HANDLING
CPTable elim(CPTable &cp_table,std::string var){

    //find other variables that
    //are neither var, nor, p, m or dist
    int elim_var_index = cp_table.m_schema.get_index(var);
    int var_index;
    std::vector<int> keep_var_index;

    RowSchema new_schema;

    new_schema.append(cp_table.m_schema.get(cp_table.m_schema.get_index(std::string("p"))));

    for (auto fs : cp_table.m_schema.m_fields ){

        std::unordered_map<std::string,int>::const_iterator prot = cp_table.protected_names.find(fs.get_name());
        
        if (prot == cp_table.protected_names.end()){
            var_index = cp_table.m_schema.get_index(fs.get_name());
            if (var_index != elim_var_index){
                keep_var_index.emplace_back(var_index);
                new_schema.append(fs);
            }


        };
    };

    //Iterate through rows and dynamically create the new table
    //We assume that the evidence subset has been applied prior to
    //making a query
    
    CPTable new_table = CPTable(new_schema);
    
    std::unordered_map<std::string,int> new_row_keys;
    for (auto & row : cp_table.m_rows ){

        std::string row_key; 
        for (int & i : keep_var_index){
            
            row_key.append(std::to_string(row.get(i).m_integer));
            row_key.append(",");
        
        };

        std::unordered_map<std::string,int>::const_iterator unique_row = new_row_keys.find(row_key);
        
        //if the row is new
        if(unique_row == new_row_keys.end()){

            new_row_keys[row_key] = (int)(new_row_keys.size());
            
            //make the new RowValue
            RowValue new_row(new_table.m_schema);
            
            //push the probability value in
            new_row.push_check<double>(row.get(0).m_floatingpoint);

            //push the remaining values
            for (int & i : keep_var_index){

                new_row.push_check<int>(row.get(i).m_integer);

            };

            new_table.m_rows.emplace_back(new_row);

        } else {
            //if the row is already in there
            //add the probability to the existing one
            
            new_table.m_rows.at(unique_row->second).add(0,row.get(0).m_floatingpoint);

        };


    };

    return ( new_table );
};

CPTable elim(CPTable &cp_table,std::vector<std::string> vars){

    CPTable new_table = cp_table;

    for(std::string &var: vars){

        new_table = elim(new_table,var);

    };

    return( new_table );

};

CPTable join(CPTable &t1,CPTable &t2){

    std::vector<FieldSchema>::iterator schema_it;
    int var_index;
    std::vector<std::pair<int,int>> keep_var_indexes;

    RowSchema new_schema;

    new_schema.append(t1.m_schema.get(t1.m_schema.get_index(std::string("p"))));

    for (const FieldSchema & fs : t1.m_schema.m_fields ){

        std::unordered_map<std::string,int>::const_iterator prot = t1.protected_names.find(fs.get_name());
        
        if (prot == t1.protected_names.end()){
            
            var_index = t1.m_schema.get_index(fs.get_name());

            schema_it = find(new_schema.m_fields.begin(), new_schema.m_fields.end(), fs.get_name());

            if(schema_it == new_schema.m_fields.end()){
                new_schema.append(fs);
            }
            

        };
    };

    for (const FieldSchema & fs : t2.m_schema.m_fields ){
        
        std::unordered_map<std::string,int>::const_iterator prot = t2.protected_names.find(fs.get_name());
        
        if (prot == t2.protected_names.end()){
            
            var_index = t2.m_schema.get_index(fs.get_name());
            
            schema_it = find(new_schema.m_fields.begin(), new_schema.m_fields.end(), fs.get_name());
            
            if(schema_it == new_schema.m_fields.end()){
                new_schema.append(fs);
            };
            
        };
    };

    CPTable new_table = CPTable(new_schema);
    
    //Create a multi-index from new_schema -> <table 1 index, table 2 index>
    std::vector<FieldSchema>::iterator s1_it;
    std::vector<FieldSchema>::iterator s2_it;
    for (auto new_field_it = new_schema.m_fields.begin(); new_field_it!=new_schema.m_fields.end(); ++new_field_it){
    
        std::string schema_fname = new_field_it->get_name();
        if (schema_fname=="p"){
            continue;
        }

        s1_it = find(t1.m_schema.m_fields.begin(),t1.m_schema.m_fields.end(), schema_fname);
        s2_it = find(t2.m_schema.m_fields.begin(),t2.m_schema.m_fields.end(), schema_fname);

        //if name is not in table 1
        if(s1_it==t1.m_schema.m_fields.end()){
            
            //and it's not in table 2
            if(s2_it==t2.m_schema.m_fields.end()){
                //this shouldn't happen
                keep_var_indexes.emplace_back(std::make_pair(-1,-1));

            } else {
            //is in table 2
                keep_var_indexes.emplace_back(std::make_pair(-1,s2_it-t2.m_schema.m_fields.begin()));

            };

        } else {
            //is not in table 2
            if(s2_it==t2.m_schema.m_fields.end()){

                keep_var_indexes.emplace_back(std::make_pair(s1_it-t1.m_schema.m_fields.begin(),-1));

            } else {
                //is in both
                keep_var_indexes.emplace_back(std::make_pair(s1_it-t1.m_schema.m_fields.begin(),s2_it-t2.m_schema.m_fields.begin()));

            };

        };

    };
    
    //Dyanmically insert into new table
    std::unordered_map<std::string,int> new_row_keys;
    for ( auto & row1 : t1.m_rows ){

        for ( auto & row2 : t2.m_rows ){
            
            //loop over schema index pairs -- in order of the field schema
            std::string row_key;
            std::vector<int> row_key_vec;
            bool trigger = false;
            for ( auto & schema_ind : keep_var_indexes){

                if(schema_ind.first!=-1){

                    if(schema_ind.second!=-1){
                        
                        //Variable is in both tables, so ensure the values match
                        if(row1.get(schema_ind.first).m_integer == row2.get(schema_ind.second).m_integer){
                           
                            int var_lev = row1.get(schema_ind.first).m_integer;
                            row_key_vec.emplace_back(var_lev);
                            row_key.append(std::to_string(var_lev));
                            row_key.append(",");

                        } else {
                            trigger=true;
                            continue;
                            
                        }
                        
                    } else {
                        //Just in the first and not in the second then use the first table value
                        int var_lev = row1.get(schema_ind.first).m_integer;
                        row_key_vec.emplace_back(var_lev);
                        row_key.append(std::to_string(var_lev));
                        row_key.append(",");
                    }  

                } else {
                    
                    if(schema_ind.second!=-1){
                        //Just in the second and not in the first then use the second table value
                        int var_lev = row2.get(schema_ind.second).m_integer;
                        row_key_vec.emplace_back(var_lev);
                        row_key.append(std::to_string(var_lev));
                        row_key.append(",");

                    } else {
                         //In neither table -- shouldn't happen
                        trigger=true;
                        continue;                 
                    }   
                }

            };
            
            if (trigger){
                continue;
            }
            
            double row_prob = row1.get(0).m_floatingpoint * row2.get(0).m_floatingpoint;

            //Search for this row key in the set of existing keys
            std::unordered_map<std::string,int>::const_iterator unique_row = new_row_keys.find(row_key);

            //if the row is new
            if(unique_row == new_row_keys.end()){

                new_row_keys[row_key] = (int)(new_row_keys.size());
                
                //make the new RowValue
                RowValue new_row(new_table.m_schema);
                
                //push the probability value in
                new_row.push_check<double>(row_prob);

                //push the remaining key values
                for (int & i : row_key_vec){

                    new_row.push_check<int>(i);

                };

                new_table.m_rows.emplace_back(new_row);

            } else {
                //if the row is already in there
                //this shouldn't happen
                throw std::string("Non-unique rows");
            };

        };

    };
    return ( new_table );
};

CPTable join(const std::vector<CPTable> & tables){

    if (tables.empty()){

        return CPTable();

    } else if (tables.size() == 1){

        CPTable new_table(tables.at(0));
        return new_table ;

    } else {

        CPTable tbl_a = tables.at(0);
        CPTable tbl_b;
        for(std::size_t i=1; i < tables.size(); i++){
            tbl_b = tables.at(i);
            tbl_a = join(tbl_a,tbl_b);
        }
        return tbl_a;

    }

};


CPTable d_join(CPTable dg_table,const std::vector<CPTable> & tables, std::size_t & ij){

    if (tables.empty()){

        return dg_table;

    } else if (tables.size() == 1){

        return dg_table ;

    } else {
        if(ij == 0 ){

            if (tables.size() == 2){
                
                CPTable tbl_a = tables.at(1);
                
                tbl_a = join(dg_table,tbl_a);

                return tbl_a ;
                
            } else {
                //size must be greater than 2
                CPTable tbl_a = tables.at(1);
                
                tbl_a = join(dg_table,tbl_a);

                CPTable tbl_b;
                for(std::size_t i=2; i < tables.size(); i++){
                    if (i != ij ){
                        tbl_b = tables.at(i);
                        tbl_a = join(tbl_a,tbl_b);
                    }
                }
                
                return tbl_a;

            }

        } else {

            CPTable tbl_a = tables.at(0);

            tbl_a = join(dg_table,tbl_a);
            
            CPTable tbl_b;
            for(std::size_t i=1; i < tables.size(); i++){
                if (i != ij ){
                    tbl_b = tables.at(i);
                    tbl_a = join(tbl_a,tbl_b);
                }
              
            }

            return tbl_a;

        }

    }

};


std::string max_index(CPTable & b_tbl, std::vector<std::string> & variable_order_pi){

    std::string m_index = "naught";
    auto tbl_scheme = b_tbl.m_schema.field_names();
    for(auto && var : variable_order_pi){
        
        auto it = std::find(tbl_scheme.begin(), tbl_scheme.end(), var);

        if (it != tbl_scheme.end()){

            m_index = *it;

        }
    }
    return(m_index);

};


double sigmoid(double & x){
    
    double negx = 0 - x;
    double sigx = 1.0 / (1.0 + exp ( negx ));
    return(sigx);

};

}