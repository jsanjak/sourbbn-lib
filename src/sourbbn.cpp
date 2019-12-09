//
// Created by jsanjak on 9/12/2019.
//
#include <iostream>
#include <vector>
#include <string>
#include <exception>
#include "sourbbn/sourbbn.hpp"

namespace sourbbn {
    //Here for compatibility with the mobile example
    std::string from_sourbbn(const std::string &s1 ) {

        return s1;

    }

    class Sourbbn::sourbbn_impl {

        private:

            std::string db_path = {};
            bool fake = false;
            std::vector<std::string> evidence_vars= {};
            std::vector<int> evidence_values = {};
            std::string query_var = {};
            std::vector<std::string> query_var_levels = {};
            std::vector<float> means = {};
            std::vector<float> standard_devs = {};

        public:

            sourbbn_impl(const std::string &db) : db_path(db) {}
            sourbbn_impl(const std::string &db, const bool & f) : db_path(db), fake(f) {}

            void set_query(
                const std::vector<std::string> & e_vars, 
                const std::vector<int> & e_values, 
                const std::string & q_var){
                        
                if ( e_vars.size()==e_values.size()){
                    
                    //Confirm can open and read db
                    //Confirm e_vars, and q_var are in the database
                    //Confirm that e_values are valid values of e_vars

                    evidence_vars = e_vars;
                    evidence_values = e_values;
                    query_var = query_var;
                    std::cout << "Set some members" << std::endl;

                    if (fake){
                        query_var_levels = {"anaplasmosis","rickettsiosis","lyme_disease", "ehrlichiosis"};
                    } else {
                        //should read from database for this
                        query_var_levels = {"0","1"};
                    };

                } else {

                    throw std::invalid_argument("Evidence variables and values are of different length");

                }      

            };
            
            
            void calc_means(){
                //TODO ERROR AND STATE HANDLING
                if(fake){

                    means = {0.7,0.24,0.05,0.01};

                } else {
                    //Put real implementation here
                    means = {0.5,0.5};

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
    std::vector<std::string> Sourbbn::read_query_names(){ return sourbbn_pimpl->read_query_names();}
    std::vector<float> Sourbbn::read_standard_devs(){ return sourbbn_pimpl->read_standard_devs();}

    

    Sourbbn::~Sourbbn() = default;
}