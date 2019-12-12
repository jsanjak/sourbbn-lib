#include <unordered_map>
#include <utility>
#include <string>
#include <vector>

#include "sourbbn/cptable.hpp"
#include "sourbbn/buckets.hpp"

namespace sourbbn {

Bucket::Bucket(){
 
    std::string bucket_id;
    std::unordered_map<std::string,CPTable> bucket_tables;

};


Bucket::Bucket(std::string & bid,std::vector<CPTable> & b_tbls) : bucket_id(bid) {
    
    std::unordered_map<std::string,CPTable> bucket_tables = {};

    for (CPTable & cpt : b_tbls ){
        
        std::string length_s = std::to_string(bucket_tables.size());
        
        std::pair<std::string,CPTable> bucket_entry(bucket_id+"_"+length_s,cpt);
        
        bucket_tables.insert(bucket_entry);
    
    }

};



}