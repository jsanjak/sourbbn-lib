//
// Created by jsanjak on 11/04/2019.


#ifndef SOURBBN_LIB_BUCKETS_HPP
#define SOURBBN_LIB_BUCKETS_HPP

#include <unordered_map>
#include <string>
#include <vector>

namespace sourbbn {

struct Bucket {

  std::unordered_map<std::string,CPTable> bucket_tables;
  
  Bucket();
  Bucket(std::vector<CPTable> bt);
  
};

struct BucketList {

  std::vector<std::string> variable_order_pi;
  std::unordered_map<std::string,Bucket> buckets;

  BucketList();
  std::string max_index(std::string bucket, std::string table);

};

}
#endif //SOURBBN_LIB_BUCKETS_HPP