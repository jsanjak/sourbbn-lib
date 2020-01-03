#include <unordered_map>
#include <utility>
#include <string>
#include <vector>
#include <iostream>

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

};


BucketList::BucketList(std::vector<std::string> & pi): variable_order_pi(pi){

    variable_order_pi.emplace(variable_order_pi.begin(),"naught");
    std::unordered_map<std::string,Bucket> buckets;

};

float BucketList::BuckElim(){

    CPTable temp_gj;
    CPTable temp_big_table;
    std::string gj_index;
    /*for (std::vector<std::string>::reverse_iterator rvar_it = variable_order_pi.rbegin(); 
    rvar_it != (variable_order_pi.rend()+1); ++rvar_it ) { 

        CPTable temp_big_table = join(buckets[*rvar_it].bucket_tables);
        std::cout << "var is: "<< *rvar_it << std::endl;
        temp_gj = elim(temp_big_table,*rvar_it);   
        gj_index = max_index(temp_gj,variable_order_pi);
        buckets[gj_index].append(temp_gj);
        buckets[*rvar_it].bucket_tables.clear();
    } */

    for (std::size_t rv = variable_order_pi.size()-1; rv != 0; --rv ) { 

        auto rvar = variable_order_pi[rv];
        
        /*std::cout << "Bucket tables:" << std::endl;
        for( auto bt : buckets[rvar].bucket_tables){
            
            print_cptable( bt,false);

        }*/
        
        CPTable temp_big_table = join(buckets[rvar].bucket_tables);
        
        //std::cout << "Joined table:" << std::endl;
        //print_cptable(temp_big_table,false);
        //std::cout << "var is: "<< rvar << std::endl;

        temp_gj = elim(temp_big_table,rvar);  

        //std::cout << "Eliminated table:" << std::endl;
        //print_cptable(temp_gj,false);    
        
        gj_index = max_index(temp_gj,variable_order_pi);

        buckets[gj_index].append(temp_gj);

        buckets[rvar].bucket_tables.clear();

    } 
    
    FieldValue final_value = buckets[variable_order_pi[0]].bucket_tables[0].m_rows[0].get(0); 
    
    return(final_value.m_floatingpoint);
};

}


/* Parking lot of unused code
    std::unordered_map<std::string,CPTable> bucket_tables = {};

    for (CPTable & cpt : b_tbls ){
        
        std::string length_s = std::to_string(bucket_tables.size());
        
        std::pair<std::string,CPTable> bucket_entry(bucket_id+"_"+length_s,cpt);
        
        bucket_tables.insert(bucket_entry);
    
    }
*/