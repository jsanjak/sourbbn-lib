//Header only 
#ifndef SOURBBN_LIB_UTILS_HPP
#define SOURBBN_LIB_UTILS_HPP

#include <string>
#include <vector>
#include <utility>  
#include <functional>
#include <iterator>
#include <algorithm>

namespace sourbbn {

////////////////////////////////////////////////////////////////////////////////////////

inline int standard_sqlite_callback(void* data, int argc, char** argv, char** azColName);

void print_cptable(CPTable &cp_table, bool print);

CPTable elim(CPTable &cp_table,std::string var);
CPTable elim(CPTable &cp_table,std::vector<std::string> vars);

CPTable join(CPTable &t1,CPTable &t2);
CPTable join(std::vector<std::reference_wrapper<CPTable>> &tables);

////////////////////////////////////////////////////////////////////////////////////////
inline int standard_sqlite_callback(void* data, int argc, char** argv, char** azColName) 
{ 
    int i; 
    std::vector<std::string> *p = static_cast<std::vector<std::string>*>(data);
    for (i = 0; i < argc; i++) { 
        p->push_back(argv[i]);
    } 
    return 0; 
} 


void print_cptable(CPTable &cp_table, bool print){

    if (print){
        std::cout << "Table data are:" << std::endl;
    };
    for (auto & row: cp_table.m_rows) {

        for (int j=0; j<cp_table.m_schema.m_fields.size(); j++) {

            if (cp_table.m_schema.get(j).get_type() == sourbbn::FieldType::Integer){

                std::cout << std::to_string(row.get(j).m_integer) << " ";
            
            } else if (cp_table.m_schema.get(j).get_type() == sourbbn::FieldType::FloatingPoint) {

                std::cout << std::to_string(row.get(j).m_floatingpoint) << " ";

            } else if (cp_table.m_schema.get(j).get_type() == sourbbn::FieldType::Boolean) {
            
                std::cout << std::to_string( (int) row.get(j).m_boolean) << " ";

            };
  
        };
        std::cout << std::endl;
	};

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
            new_row.push_check<float>(row.get(0).m_floatingpoint);

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
            
            float row_prob = row1.get(0).m_floatingpoint * row2.get(0).m_floatingpoint;

            //Search for this row key in the set of existing keys
            std::unordered_map<std::string,int>::const_iterator unique_row = new_row_keys.find(row_key);

            //if the row is new
            if(unique_row == new_row_keys.end()){

                new_row_keys[row_key] = (int)(new_row_keys.size());
                
                //make the new RowValue
                RowValue new_row(new_table.m_schema);
                
                //push the probability value in
                new_row.push_check<float>(row_prob);

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

//CPTable join(std::vector<std::reference_wrapper<CPTable>> &tables);

}

#endif //SOURBBN_LIB_UTILS_HPP