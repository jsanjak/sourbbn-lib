//
// Created by jsanjak on 11/04/2019.


#ifndef SOURBBN_LIB_BUCKETS_HPP
#define SOURBBN_LIB_BUCKETS_HPP

#include <unordered_map>
#include <string>
#include <vector>


namespace sourbbn {

/*
Potentially if casting table IDs to strings is a
performance drag
typedef std::pair<std::string,std::size_t> table_id;
struct tableid_hash
{
	template <class T1, class T2>
	std::size_t operator() (const std::pair<T1, T2> &table_id) const
	{
    // use hash mixer
		return std::hash<T1>()(table_id.first) ^ std::hash<T2>()(table_id.second);
	}
};
*/
struct Bucket {

  std::string bucket_id;
  std::unordered_map<std::string,CPTable> bucket_tables;
  
  Bucket();
  Bucket(std::string & bid);
  Bucket(std::string & bid,CPTable b_tbl);
  Bucket(std::string & bid,std::vector<CPTable> & b_tbls);
  //TODO Add table to bucket
};

struct BucketList {

  std::vector<std::string> variable_order_pi;
  std::unordered_map<std::string,Bucket> buckets;

  BucketList();
  std::string max_index(std::string bucket, std::string table);

};

}
#endif //SOURBBN_LIB_BUCKETS_HPP