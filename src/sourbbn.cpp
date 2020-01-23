//
// Created by jsanjak on 9/12/2019.
//
#include <string>
#include <vector>
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

                        if (fake){

                            query_var_levels = {"anaplasmosis","rickettsiosis","lyme_disease", "ehrlichiosis"};

                        } else {
                            
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
                                
                                evidence_subset_query = "SELECT * FROM " + tbl_name + " WHERE ";

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

                                                condition_temp = evidence_vars[ev_i] + " = " + std::to_string(evidence_values[ev_i]);
                                                
                                            } else {
                                                //TODO: add in OR values for soft evidence
                                                condition_temp = " AND " + evidence_vars[ev_i] + " = " + std::to_string(evidence_values[ev_i]);
                                            
                                            }
                                            ev_in_table+=1;
                                            evidence_subset_query += condition_temp;

                                    }

                                }
                                
                                //Evidence Only BucketList for  P( E=e )
                                std::string evidence_only_query(evidence_subset_query);

                                evidence_only_query += ";";

                                //std::cout << evidence_only_query <<std::endl;

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
                                
                                } else {

                                    std::vector<CPTable> ev_temp_vec = {ev_temp_table};
                                    
                                    Bucket ev_temp_bucket = Bucket(tbl_name,ev_temp_vec);

                                    evidence_bucket_list.buckets[ev_table_index] = ev_temp_bucket;
                                    evidence_bucket_list.original_size[ev_table_index] = 1;

                                }
                                
                                //Query level BucketLists for each P( H = h_i | E = e )
                                qvar_it = find(temp_table_names.begin(),temp_table_names.end(),query_var);

                                for (auto && q_lev : query_var_levels ){
                                    //copy the evidence subset to add on the additional query subset
                                    std::string query_subset_query(evidence_subset_query);

                                    if (qvar_it != temp_table_names.end()){

                                        query_temp = " AND " + query_var + " = " + q_lev + ";";
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
                                    } else {
                                        //Can be moved constructed?
                                        query_bucket_lists[q_lev] = BucketList(cptable_names);
                                        
                                        std::vector<CPTable> temp_vec = {temp_table};
                                        
                                        Bucket temp_bucket = Bucket(tbl_name,temp_vec);

                                        query_bucket_lists[q_lev].buckets[temp_table_index] = temp_bucket;
                                        query_bucket_lists[q_lev].original_size[temp_table_index] = 1;
                                    } 
                                }
                                ev_temp_table = CPTable();
                                temp_table = CPTable();
                            }
                            
                        }
                    } else {

                        throw std::invalid_argument("Invalid SQLite3 database");

                    }
                    
                     
                    sqlite3_close(DB);
                } else {

                    throw std::invalid_argument("Evidence variables and values are of different length");

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

                    means = {0.7,0.24,0.05,0.01};

                } else {
                    //Put real implementation here

                    //#For each level of the query variable
                    //calulate P( H=q_lev | E=evidence)
                    //Normalize to obtain conditional probabilities
                    //BucketList
                    means.clear();
                    float p_h_e;
                    for (auto && q_lev : query_var_levels){
                        
                        p_h_e = query_bucket_lists[q_lev].BuckElim();

                        means.push_back(p_h_e);

                    }
                    p_e = std::accumulate(means.begin(), means.end(), 0.0);
                    //std::cout << "Elim Answer:" << sum << std::endl;
                    for ( int iv = 0; iv < means.size(); ++iv ){
                        means[iv] /= p_e;
                        
                    }
                   
                };
                means_calc = true;

            };

           void calc_standard_devs(){
                //TODO ERROR AND STATE HANDLING
                if(fake){

                    standard_devs = {0.1,0.05,0.01,0.005};
                
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
                    float der_p_h_e;
                    float p_h_e;
                    float theta;
                    float var_theta_a;
                    float var_theta_b;
                    float var_theta;
                    float sigma_sq_h_e;
                    std::vector<float> sigma;
                    float m_f;

                    evidence_bucket_list.BuckElimPlus();
                    
                    for (auto && q_lev : query_var_levels){
                        
                        //BuckElim+
                        query_bucket_lists[q_lev].BuckElimPlus();
                        
                        //Now process the results
                        auto q_lev_it = std::find(query_var_levels.begin(),query_var_levels.end(),q_lev);
                        auto q_lev_i = q_lev_it - query_var_levels.begin();

                        p_h_e = means.at(q_lev_i);
                        
                        sigma_sq_h_e = 0;
                        for(std::string rv : evidence_bucket_list.variable_order_pi){

                            auto der_bucket = query_bucket_lists[q_lev].deriv_buckets[rv];
                            auto orig_bucket = query_bucket_lists[q_lev].buckets[rv];

                            for(int table_i=0; table_i!=query_bucket_lists[q_lev].original_size[rv]; ++table_i){
                                
                                auto der_table  = der_bucket.bucket_tables.at(table_i);
                                auto orig_table  = orig_bucket.bucket_tables.at(table_i);

                                int p_index_der = der_table.m_schema.get_index("p");
                                int p_index_orig = orig_table.m_schema.get_index("p");
                                int m_index_orig = orig_table.m_schema.get_index("m");
                                int dist_index_orig = orig_table.m_schema.get_index("dist");
                                
                                std::vector<RowValue>::iterator row_it;
                                int dist_val=-1;

                                for (row_it = orig_table.m_rows.begin(); row_it != orig_table.m_rows.end(); ++row_it){
                                    
                                    int current_dist = (*row_it).get(dist_index_orig).m_integer;
                                    int current_row = row_it - orig_table.m_rows.begin();

                                    theta = (*row_it).get(p_index_orig).m_floatingpoint;
                                        //Not sure why this is working right now...should require match to der(P(E)) tables
                                    der_joint = der_table.m_rows.at(current_row).get(p_index_der).m_floatingpoint;

                                    der_p_h_e = (1.0/p_e)*(der_joint*(1 - p_h_e));
                                   
                                    if(dist_val == -1){

                                        dist_val = current_dist;
                                        var_theta_a = std::pow(der_p_h_e,2.0)*theta;
                                        var_theta_b = der_p_h_e*theta;
                                        m_f = (*row_it).get(m_index_orig).m_floatingpoint;

                                        //if last element -- implies 1 row
                                        if(row_it == (orig_table.m_rows.end() - 1)  ){
                                            var_theta = var_theta_a + std::pow(var_theta_b,2.0);
                                            sigma_sq_h_e += var_theta/(1.0 + m_f);
                                            /*if (q_lev_i == 0){
                                                std::cout << "deriv joint: " << der_joint << std::endl;
                                                std::cout << "deriv condit: " << der_p_h_e << std::endl;
                                                std::cout << "First var theta: " << var_theta_a << std::endl;
                                                std::cout << "Seconds var theta: " << var_theta_b << std::endl;
                                                std::cout << "m_f: " << m_f << std::endl;
                                            }*/
                                    
                                        }
                                    } else {
                                        if(dist_val==current_dist){
                                            //Still in current dist
                                            var_theta_a += std::pow(der_p_h_e,2.0)*theta;
                                            var_theta_b += der_p_h_e*theta;
                                            m_f = (*row_it).get(m_index_orig).m_floatingpoint;

                                            //if last element -- implies multiple of same dist
                                            if(row_it == (orig_table.m_rows.end() - 1)  ){
                                                var_theta = var_theta_a + std::pow(var_theta_b,2.0);
                                                sigma_sq_h_e += var_theta/(1.0 + m_f);
                                            }
                                        } else {
                                            //In new dist now -- finalize variance sum from prior dist
                                            dist_val = current_dist;

                                            var_theta = var_theta_a + std::pow(var_theta_b,2.0);
                                            
                                            sigma_sq_h_e += var_theta/(1.0 + m_f);
                                            
                                            m_f = (*row_it).get(m_index_orig).m_floatingpoint;
                                            //if last element -- implies finished on new dist
                                            if(row_it == (orig_table.m_rows.end() - 1)  ){
                                                //Perform whole variance sum
                                                var_theta_a = std::pow(der_p_h_e,2.0)*theta;
                                                var_theta_b = der_p_h_e*theta;
                                                var_theta = var_theta_a + std::pow(var_theta_b,2.0);
                                                sigma_sq_h_e += var_theta/(1.0 + m_f);
                                        
                                            } else {
                                                //if not last element -- implies starting a new dist
                                                var_theta_a = std::pow(der_p_h_e,2.0)*theta;
                                                var_theta_b = der_p_h_e*theta;
                                            }
                                        }
                                    }
                                }
                            }
                        }
                        sigma.push_back(std::sqrt(sigma_sq_h_e));
                    }
                    
                    standard_devs = sigma;//{0.2,0.2};

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

            /*CPTable& get_table(std::string & tbl_name){

                return(cptables.at(tbl_name));

            };

            CPTable& get_table(std::string && tbl_name){

                return(cptables.at(tbl_name));

            };*/
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
    
    //CPTable& Sourbbn::get_table(std::string & tbl_name){ return sourbbn_pimpl->get_table(tbl_name);}
    //CPTable& Sourbbn::get_table(std::string && tbl_name){ return sourbbn_pimpl->get_table(tbl_name);}

    //For now, but in the long run we may need custom
    Sourbbn::~Sourbbn() = default;
}