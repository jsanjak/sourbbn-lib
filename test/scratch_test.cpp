#include <string>
#include <vector>
#include <iterator>
#include <typeinfo>
#include <memory>
#include <iostream>
#include <fstream>
#include <sstream>

#include <sqlite3.h>

#include "sourbbn/cptable.hpp"
#include "sourbbn/utils.hpp"
#include "sourbbn/buckets.hpp"
#include "sourbbn/sourbbn.hpp"

// take from http://stackoverflow.com/a/236803/248823
void split(const std::string &s, char delim, std::vector<std::string> &elems) {
    std::stringstream ss;
    ss.str(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
        elems.push_back(item);
    }
}

void split_toint(const std::string &s, char delim, std::vector<int> &elems) {
    std::stringstream ss;
    ss.str(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
        elems.push_back(atoi(item.c_str()));
    }
}


int main(){

    std::string db_path = "test/data/diamond.sqlite";
    std::vector<std::string> static_data_table{"a","b","c","d"};
    std::vector<std::string> evidence_vars = {"a","c"};
    std::string query_var  = "d";
    std::vector<int> query_1 = {1,1};
    
    sourbbn::CPTable temp_table = sourbbn::CPTable();
    
    int can_read = 0;
    char *zErrMsg = 0;
    sqlite3* DB;
    can_read = sqlite3_open(db_path.c_str(), &DB);

    std::string a_header_query = "SELECT * FROM a LIMIT 1;";
    std::string d_header_query = "SELECT * FROM d LIMIT 1;";
    std::string query_a = "SELECT * FROM a WHERE a = 1;";
    std::string query_d1 = "SELECT * FROM d WHERE b = 1;";
    std::string query_d2 = "SELECT * FROM d WHERE c = 1;";

    sqlite3_exec(DB, 
    a_header_query.c_str(), 
    temp_table.schema_callback, 
    &temp_table.m_schema, &zErrMsg);

    sqlite3_exec(DB, 
                query_a.c_str(), 
                temp_table.data_callback, 
                &temp_table, &zErrMsg);

    temp_table = sourbbn::CPTable();
    //temp_table.m_rows.clear();
    
    sqlite3_exec(DB, 
    d_header_query.c_str(), 
    temp_table.schema_callback, 
    &temp_table.m_schema, &zErrMsg);

    sqlite3_exec(DB, 
                query_d1.c_str(), 
                temp_table.data_callback, 
                &temp_table, &zErrMsg);
    
    temp_table.m_rows.clear();

    sqlite3_exec(DB,    
                query_d2.c_str(), 
                temp_table.data_callback, 
                &temp_table, &zErrMsg);

    sqlite3_close(DB);
    ///////////////////////////////////////
    /*
    sourbbn::Sourbbn test_bbn(db_path,false);
    
    test_bbn.set_query(evidence_vars,query_1,query_var);
    
    test_bbn.calc_means();
    
    std::vector<float> test_means = test_bbn.read_means();

    std::cout << "test means: ";
    for (auto & m : test_means){
        std::cout << m << ' ';
    }
    std::cout << std::endl;
    */
    ////////////////////////////
    std::ifstream infile ("test/data/oy1_query_examples.txt");

    std::string header_line;
    std::string data_line;

    std::vector<std::string> header_values;
    std::vector<int> data_values;

    std::getline(infile, header_line);
    std::getline(infile, data_line);

    split(header_line, '\t', header_values);
    split_toint(data_line, '\t', data_values);
    
    header_values.erase(header_values.begin());
    data_values.erase(data_values.begin());
    /*
    for(auto h: header_values){
        std::cout << h << '\t';
    }
    std::cout << std::endl;
    for(auto d: data_values){
        std::cout << d << '\t';
    }
    std::cout << std::endl;
    */
    ////////////////////////////////////////
    std::string oy1_path = "test/data/oy1_diseases.sqlite";
    std::string disease_var = "disease";
    
    sourbbn::Sourbbn oy1_bbn(oy1_path,false);
    std::vector<std::string> cptable_names = oy1_bbn.read_cptable_names();

    oy1_bbn.set_query(header_values,data_values,disease_var);
    
    oy1_bbn.calc_means();
    
    std::vector<float> oy1_means = oy1_bbn.read_means();
    
    std::cout << "oy1 means: ";
    for (auto & m : oy1_means){
        std::cout << m << ' ';
    }
    std::cout << std::endl;
    
}