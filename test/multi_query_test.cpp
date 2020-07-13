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

    std::string header_line;
    std::string data_line;

    std::vector<std::string> header_values;
    std::vector<int> data_values;

    std::ifstream gerfile ("test/data/germany_query_example.txt");
    header_values.clear();
    
    std::getline(gerfile, header_line);
    split(header_line, '\t', header_values);
    header_values.erase(header_values.begin());

    std::string disease_var = "disease";
    std::string ger_path = "test/data/germany_disease_db.sqlite";
    sourbbn::Sourbbn ger_bbn(ger_path,false);
    std::vector<std::string> cptable_names = ger_bbn.read_cptable_names();

    std::vector<float> first_means;
    std::vector<float> first_std;
    std::vector<std::string> first_names;
    std::cout << "Germany DB Queries: " << std::endl;

    int case_num = 1;
    while(std::getline(gerfile, data_line)){
    
        data_values.clear();
        split_toint(data_line, '\t', data_values);
        
        data_values.erase(data_values.begin());

        ger_bbn.set_query(header_values,data_values,disease_var);
        ger_bbn.calc_means();

        //ger_bbn.calc_standard_devs();
        first_means = ger_bbn.read_means();
        //first_std = ger_bbn.read_standard_devs();
        first_names= ger_bbn.read_query_names();
        
        
        std::cout << "Case " << case_num << ": ";
        for (std::size_t i=0;i!=first_means.size(); ++i){
            std::cout << first_means.at(i) << ',';
            //std::cout << first_std.at(i) << std::endl;
        }
        case_num +=1;
        std::cout << std::endl;
    }
    
    
    
}