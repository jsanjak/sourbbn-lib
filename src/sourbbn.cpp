//
// Created by jsanjak on 9/12/2019.
//
#include <string>
#include <vector>
#include <tuple>
#include <memory>
#include <iostream>
#include <exception>
#include <iterator>
#include <algorithm>
#include <functional>
#include <numeric>
#include <unordered_map>
#include <utility>
#include <assert.h>
#include <sqlite3.h>
#include <math.h> 

#include "sourbbn/sourbbn.hpp"
#include "sourbbn/buckets.hpp"
#include "sourbbn/utils.hpp"

namespace sourbbn {
    //Here for compatibility with the mobile example
    std::string from_sourbbn(const std::string &s1 ) {

        return s1;

    }

    class Sourbbn::sourbbn_impl {

        private:

            std::string db_path = {};
            bool fake = false;
            bool means_calc = false;
            bool query_set = false;
            std::vector<std::string> cptable_names;
            std::vector<std::tuple<std::string,std::size_t>> evi_buck_cptable_coords;
            std::vector<std::tuple<std::string,std::size_t>> query_buck_cptable_coords;

            std::unordered_map<std::string,BucketList> query_bucket_lists = {};
            BucketList evidence_bucket_list = {};
            std::vector<std::string> evidence_vars = {};
            std::vector<int> evidence_values = {};
            std::string query_var = {};
            std::vector<std::string> query_var_levels = {};
            std::vector<float> means = {};
            std::vector<float> standard_devs = {};
            float p_e;

        public:

            sourbbn_impl(const std::string &db) : sourbbn_impl(db,false) {}

            sourbbn_impl(const std::string &db, const bool & f) : db_path(db), fake(f) {

                if (fake){

                    cptable_names = {"state","ASPL","disease"};

                } else {

                    int can_read = 0;
                    char *zErrMsg = 0;
                    sqlite3* DB;
                    can_read = sqlite3_open(db_path.c_str(), &DB);

                    if (can_read==0){

                        //Obtain the list of data tables
                        std::string data_table_query("SELECT name FROM sqlite_master WHERE \
                                                type ='table' AND name NOT LIKE 'sqlite_%' \
                                                AND name NOT LIKE '%_link';");


                        sqlite3_exec(DB, 
                        data_table_query.c_str(), 
                        standard_sqlite_callback, 
                        &cptable_names, &zErrMsg);

                        sqlite3_close(DB);
                    } else {

                        throw std::invalid_argument("Invalid SQLite3 database");

                    }

                }
            
            }

            void set_query(
                const std::vector<std::string> & e_vars, 
                const std::vector<int> & e_values, 
                const std::string & q_var){
                if (fake){

                    query_var_levels = {"Anaplasmosis","Babesiosis","Bmi","CTF","Ehrlichiosis","LD","Powassan","SFGR","TBRF","Tularemia"};
                    evidence_vars = e_vars;
                    evidence_values = e_values;
                } else {

                    if ( ! query_bucket_lists.empty() ){
                        query_bucket_lists.clear();
                    }
                    if ( e_vars.size()==e_values.size()){
                        
                        int can_read = 0;
                        char *zErrMsg = 0;
                        sqlite3* DB;
                        can_read = sqlite3_open(db_path.c_str(), &DB);

                        if (can_read==0){
                            //Confirm can open and read db
                            evidence_vars = e_vars;
                            evidence_values = e_values;
                            query_var = q_var;

                            
                            evidence_bucket_list = BucketList(cptable_names);

                            query_var_levels.clear();

                            std::vector<std::string>::iterator qvar_it;
                            std::vector<std::string>::iterator evar_it;

                            //Confirm that evidence variables are in the database
                            //Confirm  q_var are in the database
                            qvar_it = find(cptable_names.begin(),cptable_names.end(),query_var);

                            assert(qvar_it != cptable_names.end());
                        
                            for(auto var : evidence_vars){
                                
                                evar_it = find(cptable_names.begin(),cptable_names.end(),var);

                                if(evar_it == cptable_names.end() ){
                                    
                                    throw std::invalid_argument("Evidence variable " + var +" not in network");

                                }

                            }
                            std::string qvar_level_query="SELECT DISTINCT " + query_var + " FROM " + query_var + ";";
                            
                            sqlite3_exec(DB, 
                            qvar_level_query.c_str(), 
                            standard_sqlite_callback, 
                            &query_var_levels, &zErrMsg);

                            //Read in tables 
                            std::string header_query;
                            std::string table_query;
                            std::string evidence_subset_query;
                            
                            std::string condition_temp;
                            std::string query_temp;
                            std::pair<std::string,CPTable> temp_named_table;
                            
                            CPTable temp_table = CPTable();
                            CPTable ev_temp_table = CPTable();

                            int ev_in_table = 0;
                            //For each CPtable
                                //Subset based on evidence 
                                //For each query level
                                //  Subset based on query levels
                                //  Calculate maxIndex() w.r.t CPtable order
                                //  Assign reduced table to maxIndex bucket for the bucket list for the query level

                            for(auto && tbl_name : cptable_names){
                                
                                header_query = "SELECT * FROM " + tbl_name + " LIMIT 1;";
                                
                                evidence_subset_query = "SELECT * FROM " + tbl_name;

                                sqlite3_exec(DB, 
                                header_query.c_str(), 
                                temp_table.schema_callback, 
                                &temp_table.m_schema, &zErrMsg);

                                sqlite3_exec(DB, 
                                header_query.c_str(), 
                                ev_temp_table.schema_callback, 
                                &ev_temp_table.m_schema, &zErrMsg);

                                std::vector<std::string> temp_table_names = temp_table.m_schema.field_names();
                                
                                ev_in_table = 0;
                                for (std::size_t ev_i=0; ev_i < evidence_vars.size(); ++ev_i){
                                    
                                    //Is this evidence variable part of the schema of this table
                                    evar_it = find(temp_table_names.begin(),temp_table_names.end(),evidence_vars[ev_i]);

                                    if (evar_it != temp_table_names.end()){

                                            if (ev_in_table == 0){

                                                condition_temp = " WHERE " + evidence_vars[ev_i] + " = " + std::to_string(evidence_values[ev_i]);
                                                
                                            } else {
                                                //TODO: add in OR values for soft evidence
                                                condition_temp = " AND " + evidence_vars[ev_i] + " = " + std::to_string(evidence_values[ev_i]);
                                            
                                            }
                                            ev_in_table+=1;
                                            evidence_subset_query += condition_temp ;

                                    }

                                }
                                
                                //Evidence Only BucketList for  P( E=e )
                                std::string evidence_only_query(evidence_subset_query);

                                evidence_only_query += ";";

                                ev_temp_table.m_rows.clear();
                                    
                                sqlite3_exec(DB, 
                                        evidence_only_query.c_str(), 
                                        ev_temp_table.data_callback, 
                                        &ev_temp_table, &zErrMsg);
                                
                                std::string ev_table_index = max_index(ev_temp_table,cptable_names); 
                                
                                auto eb_it = evidence_bucket_list.buckets.find(ev_table_index);
                                
                                
                                if (eb_it != evidence_bucket_list.buckets.end() ){
                                
                                    evidence_bucket_list.buckets[ev_table_index].append(ev_temp_table);
                                    evidence_bucket_list.original_size[ev_table_index] += 1;

                                    evi_buck_cptable_coords.push_back(std::make_tuple(ev_table_index, evidence_bucket_list.original_size[ev_table_index]-1));
                                
                                } else {

                                    std::vector<CPTable> ev_temp_vec = {ev_temp_table};
                                    
                                    Bucket ev_temp_bucket = Bucket(tbl_name,ev_temp_vec);

                                    evidence_bucket_list.buckets[ev_table_index] = ev_temp_bucket;
                                    evidence_bucket_list.original_size[ev_table_index] = 1;
                                    evi_buck_cptable_coords.push_back(std::make_tuple(ev_table_index, 0));
                                }
                                
                                //Query level BucketLists for each P( H = h_i | E = e )
                                qvar_it = find(temp_table_names.begin(),temp_table_names.end(),query_var);

                                for (auto && q_lev : query_var_levels ){
                                    //copy the evidence subset to add on the additional query subset
                                    std::string query_subset_query(evidence_subset_query);

                                    if (qvar_it != temp_table_names.end()){
                                        
                                        if(ev_in_table == 0){
                                            query_temp = " WHERE " + query_var + " = " + q_lev + ";";
                                        } else {
                                            query_temp = " AND " + query_var + " = " + q_lev + ";";
                                        }
                                        
                                        query_subset_query += query_temp;
                                        
                                        temp_table.m_rows.clear();

                                        sqlite3_exec(DB, 
                                        query_subset_query.c_str(), 
                                        temp_table.data_callback, 
                                        &temp_table, &zErrMsg);

                                    } else if (q_lev == query_var_levels[0] ){

                                        query_subset_query += ";";

                                        sqlite3_exec(DB, 
                                        query_subset_query.c_str(), 
                                        temp_table.data_callback, 
                                        &temp_table, &zErrMsg);

                                    }
                                    
                                    
                                    std::string temp_table_index = max_index(temp_table,cptable_names);
                                    
                                    //if this query level is already in the list
                                    auto qb_it = query_bucket_lists.find(q_lev);

                                    if (qb_it != query_bucket_lists.end()){                                        
                                        qb_it->second.buckets[temp_table_index].append(temp_table);
                                        qb_it->second.original_size[temp_table_index] += 1;
                                        if(q_lev == *query_var_levels.begin()){
                                            query_buck_cptable_coords.push_back(std::make_tuple(temp_table_index, qb_it->second.original_size[temp_table_index]-1));
                                        }
                                    } else {
                                        //Can be moved constructed?
                                        query_bucket_lists[q_lev] = BucketList(cptable_names);
                                        
                                        std::vector<CPTable> temp_vec = {temp_table};
                                        
                                        Bucket temp_bucket = Bucket(tbl_name,temp_vec);

                                        query_bucket_lists[q_lev].buckets[temp_table_index] = temp_bucket;
                                        query_bucket_lists[q_lev].original_size[temp_table_index] = 1;

                                        if(q_lev == *query_var_levels.begin()){
                                            query_buck_cptable_coords.push_back(std::make_tuple(temp_table_index, 0));
                                        }
                                    } 
                                }
                                ev_temp_table = CPTable();
                                temp_table = CPTable();
                            }
                            
                        } else {

                            throw std::invalid_argument("Invalid SQLite3 database");

                        }
                        
                        sqlite3_close(DB);
                    } else {

                        throw std::invalid_argument("Evidence variables and values are of different length");

                    }
                }
                //If we didn't fail, then state that new means 
                //have not be determined
                //query_bucket_lists[ query_var_levels[0]].print_buckets();   
                means_calc = false;
                query_set = true;
            
            };
            
            
            void calc_means(){
                //TODO ERROR AND STATE HANDLING
                /*
                Key states:
                    - Cannot be run without setting a query variable
                */
                if(fake){
                    
                    //query_var_levels = {"Anaplasmosis","Babesiosis","Bmi","CTF","Ehrlichiosis","LD","Powassan","SFGR","TBRF","Tularemia"};
                    if( evidence_values[0] == 0){
                        //Pure fake data
                        
                        means = {0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1};
                    
                    } else if ( evidence_values[0] == 21 ) {

                        //Maryland cases
                        if( evidence_values[1] == 0){
                            //1a
                             means = {0.012,0.486,0.071,0.000,0.097,0.02,0.012,0.042,0.003,0.077};

                        } else {
                            //1b
                            means = {0.002,0.945,0.030,0.000,0.017,0.004,0.002,0.000,0.000,0.000};
                            
                        }
                        
                    } else if (evidence_values[0] == 26 ){

                        means = {0.000,0.000,0.000,0.000,0.024,0.004,0.000,0.000,0.000,0.972};

                    } else {

                        means = {0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1};
                    
                    }
                   

                } else {
                    //For each level of the query variable
                    //calulate P( H=q_lev | E=evidence)
                    //Normalize to obtain conditional probabilities
                    means.clear();
                    float p_h_e;
                    for (auto && q_lev : query_var_levels){
                        
                        p_h_e = query_bucket_lists[q_lev].BuckElim();

                        means.push_back(p_h_e);

                    }
                    p_e = std::accumulate(means.begin(), means.end(), 0.0);
                    for ( int iv = 0; iv < means.size(); ++iv ){
                        means[iv] /= p_e;
                        
                    }
                   
                };
                means_calc = true;

            };

           void calc_standard_devs(){
                //TODO ERROR AND STATE HANDLING
                if(fake){
                    
                    standard_devs.clear();
                    if( ! means_calc ){
                        if(query_set){

                            this->calc_means();
                    
                        } else {

                            throw std::invalid_argument("Query not set yet.");
                    
                        }
                    }
                        //query_var_levels = {"Anaplasmosis","Babesiosis","Bmi","CTF","Ehrlichiosis","LD","Powassan","SFGR","TBRF","Tularemia"};
                        
                            //query_var_levels = {"Anaplasmosis","Babesiosis","Bmi","CTF","Ehrlichiosis","LD","Powassan","SFGR","TBRF","Tularemia"};
                        if( evidence_values[0] == 0){
                            //Pure fake data
                            standard_devs = {0.025,0.025,0.025,0.025,0.025,0.025,0.025,0.025,0.025,0.025};
                        
                        } else if ( evidence_values[0] == 21 ) {

                            //Maryland cases
                            if( evidence_values[1] == 0){
                                //1a
                                standard_devs = {0.025,0.436,0.209,0.001,0.145,0.252,0.038,0.066,0.011,0.163};

                            } else {
                                //1b
                                standard_devs = {0.006,0.136,0.11,0.000,0.04,0.01,0.006,0.001,0.001,0.001};
                                
                            }
                            
                        } else if (evidence_values[0] == 26){
                            //Missouri case
                            standard_devs = {0.000,0.000,0.000,0.000,0.036,0.009,0.000,0.000,0.000,0.042};

                        } else {

                            standard_devs = {0.025,0.025,0.025,0.025,0.025,0.025,0.025,0.025,0.025,0.025};
                        
                        }
                } else {
                    //Put real implementation here
                    standard_devs.clear();
                    if( ! means_calc ){

                        if(query_set){
                            this->calc_means();
                    
                        } else {
                            throw std::invalid_argument("Query not set yet.");
                    
                        }
                    
                    }

                    
                    std::vector<float> temp_probs;
                    float der_joint;
                    float der_p_h_cond_e;
                    float der_p_e;
                    float der_p_he;
                    float p_h_cond_e;
                    float theta;
                    float var_theta_a;
                    float var_theta_b;
                    float var_theta;
                    float sigma_sq_h_e;
                    std::vector<float> sigma;
                    float m_f;
                    int q_ind;
                    int e_ind;
                    std::size_t match_indicator;
                    int hyp_index_der;
                    std::string q_hyp_lev;

                    evidence_bucket_list.BuckElimPlus();
                    
                    for (auto && q_lev : query_var_levels){
                        
                        //BuckElim+
                        query_bucket_lists[q_lev].BuckElimPlus();
                        
                        //Now process the results
                        auto q_lev_it = std::find(query_var_levels.begin(),query_var_levels.end(),q_lev);
                        auto q_lev_i = q_lev_it - query_var_levels.begin();

                        p_h_cond_e = means.at(q_lev_i);
                        
                        sigma_sq_h_e = 0;
                        
                        for(std::string rv : evidence_bucket_list.variable_order_pi){

                            auto der_bucket_q = query_bucket_lists[q_lev].deriv_buckets[rv];
                            auto orig_bucket_q = query_bucket_lists[q_lev].buckets[rv];

                            auto der_bucket_e = evidence_bucket_list.deriv_buckets[rv];
                            auto orig_bucket_e = evidence_bucket_list.buckets[rv];

                            for(int table_i=0; table_i!=query_bucket_lists[q_lev].original_size[rv]; ++table_i){
                                
                                auto der_table_q  = der_bucket_q.bucket_tables.at(table_i);
                                auto orig_table_q  = orig_bucket_q.bucket_tables.at(table_i);

                                auto der_table_e  = der_bucket_e.bucket_tables.at(table_i);
                                auto orig_table_e  = orig_bucket_e.bucket_tables.at(table_i);

                                int p_index_der = der_table_e.m_schema.get_index("p");
                                int p_index_orig = orig_table_e.m_schema.get_index("p");
                                int m_index_orig = orig_table_e.m_schema.get_index("m");
                                int dist_index_orig = orig_table_e.m_schema.get_index("dist");

                                std::vector<std::string> table_i_vars =  der_table_e.scheme().field_names();
                                std::vector<std::string>::iterator qe_it = std::find(table_i_vars.begin(),table_i_vars.end(),query_var);
                                if(qe_it!=table_i_vars.end()){
                                    hyp_index_der = der_table_e.m_schema.get_index(query_var);
                                } else {
                                    hyp_index_der = -1;
                                }
                                
                                std::vector<RowValue>::iterator row_it;
                                int dist_val=-1;

                                //Iterate through evidence only tables, which are by default greater
                                //than or equal to evidence + hypothesis tables
                                
                                for (row_it = orig_table_e.m_rows.begin(); row_it != orig_table_e.m_rows.end(); ++row_it){
                                    
                                    int current_dist = (*row_it).get(dist_index_orig).m_integer;
                                    int current_row = row_it - orig_table_e.m_rows.begin();
                                    
                                    if(hyp_index_der == -1 ){
                                        q_hyp_lev = q_lev;
                                    } else {
                                        q_hyp_lev = std::to_string((*row_it).get(hyp_index_der).m_integer);
                                    }
                                    

                                    theta = (*row_it).get(p_index_orig).m_floatingpoint;
                                    
                                    der_p_e = der_table_e.m_rows.at(current_row).get(p_index_der).m_floatingpoint;
                                    der_p_he =  0;
                                    if(q_hyp_lev == q_lev){

                                        //get der_p_he by matching
                                        for (auto && der_table_q_row : der_table_q.m_rows ){
                                            
                                            match_indicator = 0;
                                            for(auto && tiv : table_i_vars){

                                                e_ind = der_table_e.m_schema.get_index(tiv);

                                                q_ind = der_table_q.m_schema.get_index(tiv);
                                                
                                                auto dte_val = der_table_e.m_rows.at(current_row).get(e_ind);

                                                auto dtq_val = der_table_q_row.get(q_ind);

                                                if(dte_val.m_integer == dtq_val.m_integer){
                                                    match_indicator += 1;
                                                }
                                            }
                                            if(table_i_vars.size() == match_indicator ){

                                                der_p_he = der_table_q_row.get(p_index_der).m_floatingpoint;
                                                break;
                                            
                                            }

                                        }

                                    }
                                    //This is the key calculation
                                    //der_p_he can be zero, if the current row
                                    //does not correspond to the current query level
                                    der_p_h_cond_e = (1.0/p_e)*(der_p_he - p_h_cond_e*der_p_e);

                                    if(dist_val == -1){

                                        dist_val = current_dist;
                                        var_theta_a = std::pow(der_p_h_cond_e,2.0)*theta;
                                        var_theta_b = der_p_h_cond_e*theta;
                                        m_f = (*row_it).get(m_index_orig).m_floatingpoint;

                                        //if last element -- implies 1 row
                                        if(row_it == (orig_table_e.m_rows.end() - 1)  ){

                                            var_theta = var_theta_a - std::pow(var_theta_b,2.0);

                                            sigma_sq_h_e += var_theta/(1.0 + m_f);

                                        }
                                    } else {
                                        if(dist_val==current_dist){
                                            //Still in current dist
                                            var_theta_a += std::pow(der_p_h_cond_e,2.0)*theta;
                                            var_theta_b += der_p_h_cond_e*theta;
                                            m_f = (*row_it).get(m_index_orig).m_floatingpoint;

                                            //if last element -- implies multiple of same dist
                                            if(row_it == (orig_table_e.m_rows.end() - 1)  ){

                                                var_theta = var_theta_a - std::pow(var_theta_b,2.0);
                                                
                                                sigma_sq_h_e += var_theta/(1.0 + m_f);

                                            }

                                        } else {
                                            //In new dist now -- finalize variance sum from prior dist
                                            dist_val = current_dist;

                                            var_theta = var_theta_a - std::pow(var_theta_b,2.0);
                                            
                                            sigma_sq_h_e += var_theta/(1.0 + m_f);

                                            m_f = (*row_it).get(m_index_orig).m_floatingpoint;

                                            //if last element -- implies finished on new dist
                                            if(row_it == (orig_table_e.m_rows.end() - 1)  ){

                                                //Perform whole variance sum
                                                var_theta_a = std::pow(der_p_h_cond_e,2.0)*theta;
                                                var_theta_b = der_p_h_cond_e*theta;
                                                var_theta = var_theta_a - std::pow(var_theta_b,2.0);
                                                sigma_sq_h_e += var_theta/(1.0 + m_f);
                                        
                                            } else {
                                                //if not last element -- implies starting a new dist
                                                var_theta_a = std::pow(der_p_h_cond_e,2.0)*theta;
                                                var_theta_b = der_p_h_cond_e*theta;
                                            }
                                        }
                                    }
                                }
                            }
                        }
                        sigma.push_back(std::sqrt(sigma_sq_h_e));
                    }
                    
                    standard_devs = sigma;

                } 

            };

            std::vector<std::string> read_query_names(){

                return(query_var_levels);

            };

            std::vector<std::string> read_cptable_names(){

                return(cptable_names);

            };

            std::vector<float> read_means(){
                //TODO ERROR AND STATE HANDLING
                return(means);
            };
            
            std::vector<float> read_standard_devs(){
                //TODO ERROR AND STATE HANDLING
                return(standard_devs);
            };

    };

    Sourbbn::Sourbbn(const std::string &db_path) : sourbbn_pimpl {  new sourbbn_impl(db_path) }{}
    Sourbbn::Sourbbn(const std::string &db_path, const bool & is_fake) : sourbbn_pimpl {  new sourbbn_impl(db_path,is_fake) }{}
    
    void Sourbbn::set_query(
            const std::vector<std::string> & evidence_vars,
            const std::vector<int> & evidence_values,
            const std::string & query_var) 
            { 
                sourbbn_pimpl->set_query(evidence_vars,evidence_values,query_var); 
            }
    
    void Sourbbn::calc_means(){sourbbn_pimpl->calc_means();}
    void Sourbbn::calc_standard_devs(){sourbbn_pimpl->calc_standard_devs();}

    std::vector<float> Sourbbn::read_means(){ return sourbbn_pimpl->read_means();}
    std::vector<std::string> Sourbbn::read_cptable_names(){ return sourbbn_pimpl->read_cptable_names();}
    std::vector<std::string> Sourbbn::read_query_names(){ return sourbbn_pimpl->read_query_names();}
    std::vector<float> Sourbbn::read_standard_devs(){ return sourbbn_pimpl->read_standard_devs();}
    
    //Default for now, but in the long run we may need custom
    Sourbbn::~Sourbbn() = default;
}