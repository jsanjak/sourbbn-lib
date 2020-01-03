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
            std::vector<std::string> cptable_names;

            //Potentially unneccessary
            //std::unordered_map<std::string,CPTable> cptables = {};
            std::unordered_map<std::string,BucketList> query_bucket_lists = {};
            std::vector<std::string> evidence_vars = {};
            std::vector<int> evidence_values = {};
            std::string query_var = {};
            std::vector<std::string> query_var_levels = {};
            std::vector<float> means = {};
            std::vector<float> standard_devs = {};

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

                        //std::string link_table_query("SELECT name FROM sqlite_master WHERE \
                        //     type ='table' AND name NOT LIKE 'sqlite_%' \
                        //     AND name LIKE '%_link';");

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
                            
                            query_var_levels.clear();

                            std::vector<std::string>::iterator qvar_it;
                            std::vector<std::string>::iterator evar_it;

                            //Confirm that evidence variables are in the database
                            //Confirm  q_var are in the database
                            qvar_it = find(cptable_names.begin(),cptable_names.end(),query_var);

                            assert(qvar_it != cptable_names.end());
                        
                            for(auto var : evidence_vars){
                                
                                evar_it = find(cptable_names.begin(),cptable_names.end(),var);
                                assert(evar_it != cptable_names.end());
                                /*if(evar_it == cptable_names.end() ){
                                    
                                    throw std::invalid_argument("Evidence variable " + var +" not in network");

                                }*/

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
                                
                                qvar_it = find(temp_table_names.begin(),temp_table_names.end(),query_var);

                                for (auto && q_lev : query_var_levels ){
                                    //copy the evidence subset to add on the additional query subset
                                    std::string query_subset_query(evidence_subset_query);

                                    if (qvar_it != temp_table_names.end()){

                                        query_temp = " AND " + query_var + " = " + q_lev + ";";
                                        query_subset_query += query_temp;
                                        
                                        temp_table.m_rows.clear();
                                
                                        //std::cout << query_subset_query <<std::endl;
                                        sqlite3_exec(DB, 
                                        query_subset_query.c_str(), 
                                        temp_table.data_callback, 
                                        &temp_table, &zErrMsg);
                                        //print_cptable(temp_table,false);

                                    } else if (q_lev == query_var_levels[0] ){

                                        query_subset_query += ";";

                                        //std::cout << query_subset_query <<std::endl;

                                        sqlite3_exec(DB, 
                                        query_subset_query.c_str(), 
                                        temp_table.data_callback, 
                                        &temp_table, &zErrMsg);
                                        //print_cptable(temp_table,false);

                                    }
                                    
                                    
                                    std::string temp_table_index = max_index(temp_table,cptable_names);
                                    
                                    //if this query level is already in the list
                                    auto qb_it = query_bucket_lists.find(q_lev);

                                    if (qb_it != query_bucket_lists.end()){
                                        //temp_named_table = std::make_pair(tbl_name,temp_table);
                                        
                                        qb_it->second.buckets[temp_table_index].append(temp_table);

                                    } else {
                                        //Can be moved constructed?
                                        query_bucket_lists[q_lev] = BucketList(cptable_names);
                                        
                                        std::vector<CPTable> temp_vec = {temp_table};
                                        
                                        Bucket temp_bucket = Bucket(tbl_name,temp_vec);

                                        query_bucket_lists[q_lev].buckets[temp_table_index] = temp_bucket;
                                    } 
                                    //std::cout << tbl_name << std::endl;
                                    //std::cout << query_bucket_lists[q_lev].buckets[temp_table_index].bucket_tables.size() << std::endl;
                                }
                                
                                temp_table = CPTable();
                            }

                        }
                    } else {

                        throw std::invalid_argument("Invalid SQLite3 database");

                    }
                    
                    std::unordered_map<std::string,Bucket>::iterator check_it;
                    /*
                    for(std::string q_lev : query_var_levels){
                        
                        std::cout << "Query bucket list: " << q_lev << std::endl; 
                           
                        for (std::string var : query_bucket_lists[q_lev].variable_order_pi){

                            std::cout << "Bucket variable: " << var << std::endl;
                            check_it = query_bucket_lists[q_lev].buckets.find(var);
                            if(check_it != query_bucket_lists[q_lev].buckets.end()){

                                for(CPTable cpvar : (check_it->second).bucket_tables){
                                    print_cptable(cpvar,true);
                                }
                            }
                            
                        }
                    
                    }*/
                    sqlite3_close(DB);
                } else {

                    throw std::invalid_argument("Evidence variables and values are of different length");

                }      
            
            };
            
            
            void calc_means(){
                //TODO ERROR AND STATE HANDLING
                /*
                Key states:
                    - Cannot be run without setting a query variable
                    - Should be capable of being updated from unknown -> known without starting over
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
                    float sum = std::accumulate(means.begin(), means.end(), 0.0);
                   
                    for ( int iv = 0; iv < means.size(); ++iv ){
                        means[iv] /= sum;
                    }

                };
                
            };

           void calc_standard_devs(){
                //TODO ERROR AND STATE HANDLING
                if(fake){

                    standard_devs = {0.1,0.05,0.01,0.005};
                
                } else {
                    //Put real implementation her
                    standard_devs = {0.1,0.1};
                
                };

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