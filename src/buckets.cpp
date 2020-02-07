#include <unordered_map>
#include <utility>
#include <string>
#include <vector>
#include <iostream>
#include <iterator>
#include <algorithm>

#include "sourbbn/buckets.hpp"
#include "sourbbn/utils.hpp"

namespace sourbbn {

Bucket::Bucket(){
 
    std::string bucket_id;
    std::vector<CPTable> bucket_tables;

};

Bucket::Bucket(std::string & bid) : bucket_id(bid){

    std::vector<CPTable> bucket_tables;

};

Bucket::Bucket(std::string && bid) : bucket_id(bid){

    std::vector<CPTable> bucket_tables;

};

Bucket::Bucket(std::string & bid,CPTable & b_tbl) : bucket_id(bid){

    std::vector<CPTable> bucket_tables = {b_tbl};

};

Bucket::Bucket(std::string && bid,CPTable & b_tbl) : bucket_id(bid){

    std::vector<CPTable> bucket_tables = {b_tbl};

};

Bucket::Bucket(std::string && bid,CPTable && b_tbl) : bucket_id(bid){

    std::vector<CPTable> bucket_tables = {b_tbl};

};

Bucket::Bucket(std::string & bid,CPTable && b_tbl) : bucket_id(bid){

    std::vector<CPTable> bucket_tables = {b_tbl};

};


Bucket::Bucket(std::string & bid,std::vector<CPTable> & b_tbls) : bucket_id(bid), bucket_tables(b_tbls) {
    
};

Bucket::Bucket(std::string && bid,std::vector<CPTable> & b_tbls) : bucket_id(bid), bucket_tables(b_tbls) {
    
};

Bucket::Bucket(std::string && bid,std::vector<CPTable> && b_tbls) : bucket_id(bid), bucket_tables(b_tbls) {
    
};

void Bucket::append(CPTable & b_tbl){

    bucket_tables.push_back(b_tbl);
}


BucketList::BucketList(){

    std::vector<std::string> variable_order_pi;
    std::unordered_map<std::string,Bucket> buckets;
    std::unordered_map<std::string,int> original_size;
    std::unordered_map<std::string,std::vector<std::string>> g_table_mark;
    std::unordered_map<std::string,CPTable> deriv_g;
    std::unordered_map<std::string,Bucket> deriv_buckets;
};


BucketList::BucketList(std::vector<std::string> & pi): variable_order_pi(pi){

    variable_order_pi.emplace(variable_order_pi.begin(),"naught");
    std::unordered_map<std::string,Bucket> buckets;
    std::unordered_map<std::string,int> original_size;
    std::unordered_map<std::string,std::vector<std::string>> g_table_mark;
    std::unordered_map<std::string,CPTable> deriv_g;
    std::unordered_map<std::string,Bucket> deriv_buckets; 

};

float BucketList::BuckElim(){

    if (!eliminated){

        CPTable temp_gj;
        CPTable temp_big_table;
        std::string gj_index;

        std::vector<std::string>::iterator g_var_it;

        for (std::size_t rv = variable_order_pi.size()-1; rv != 0; --rv ) { 

            auto rvar = variable_order_pi[rv];
            
            CPTable temp_big_table = join(buckets[rvar].bucket_tables);
            
            temp_gj = elim(temp_big_table,rvar);   
            
            gj_index = max_index(temp_gj,variable_order_pi);
           
            buckets[gj_index].append(temp_gj);
            
            g_var_it = find(variable_order_pi.begin(),variable_order_pi.end(),gj_index);
                    
            g_table_mark[*g_var_it].push_back(rvar);

        } 
    }
    
    FieldValue final_value = buckets[variable_order_pi[0]].bucket_tables[0].m_rows[0].get(0); 
    eliminated = true;
    return(final_value.m_floatingpoint);
};


void BucketList::BuckElimPlus(){

    //So this can be called alone
    if(!eliminated){
        
        float final = this->BuckElim();

    }

    //Actual BuckElim+
    RowSchema scheme_fij;
    RowSchema scheme_J;
    std::vector<std::string> elim_vars;
    std::vector<std::string> expand_vars;
    CPTable deriv_ij;
    CPTable J;
    CPTable expand_d_ij;
    std::size_t which_g;
    float unity = 1.0;
    for (std::size_t i = 0; i != variable_order_pi.size(); ++i ) {
        
        auto var_i = variable_order_pi[i];
        
        if(i == 0){

            auto var_i_plus = variable_order_pi[i+1];
            deriv_g[var_i_plus] = buckets[var_i].bucket_tables[0];
            deriv_g[var_i_plus].m_rows[0].reassign_field("p",FieldValue(unity));//at(0) = 1.0;//add(0,1.0);

        } else {
            
            for (std::size_t j = 0; j < buckets[var_i].bucket_tables.size(); ++j ){
                                
                J = d_join(deriv_g[var_i], buckets[var_i].bucket_tables, j);
                
                scheme_fij = buckets[var_i].bucket_tables.at(j).scheme();
                
                scheme_J = J.scheme();
                
                elim_vars = scheme_diff(scheme_J,scheme_fij);
                
                deriv_ij = elim(J,elim_vars);

                auto d_scheme = deriv_ij.scheme();
                expand_vars = scheme_overlap(scheme_fij,d_scheme);
                expand_d_ij = expand(deriv_ij,buckets[var_i].bucket_tables.at(j));
                deriv_ij = expand_d_ij;
            
                if (j < original_size[var_i]){

                    if(j==0){
                        
                        std::vector<CPTable> temp_vec = {deriv_ij};

                        Bucket temp_bucket = Bucket(var_i,temp_vec);

                        deriv_buckets[var_i] = temp_bucket;
                    
                    } else {

                        deriv_buckets[var_i].append(deriv_ij);

                    }

                } else {

                    //For remaining buckets -- identify where g came from
                    which_g = j - original_size[var_i]; 

                    auto where_g_from = g_table_mark[var_i].at(which_g);

                    deriv_g[where_g_from] = deriv_ij;

                } 
                    
            }
        
        }
    
    } 

};

void Bucket::print_bucket(){

    for (auto && cpt : bucket_tables ){
        print_cptable(cpt,true);
    }
};

void BucketList::print_buckets(){

    for (auto && rv : variable_order_pi ){
        std::cout << "Bucket " << rv << ":" << std::endl;
        buckets[rv].print_bucket();
    }
};

void BucketList::print_deriv_buckets(){

    for (auto && rv : variable_order_pi ){
        std::cout << "Bucket " << rv << ":" << std::endl;
        deriv_buckets[rv].print_bucket();
    }
};

}