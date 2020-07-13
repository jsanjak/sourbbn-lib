#include <string>
#include <vector>
#include <iterator>
#include <typeinfo>
#include <memory>
#include <iostream>
#include <fstream>
#include <sstream>
#include <ctime>

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

    std::string oy1_path = "test/data/oy1_diseases.sqlite";
    std::string disease_var = "disease";
    std::time_t start, finish;
    
    for (std::size_t i=0;i!=35; ++i){
        std::size_t n_vars = header_values.size();
        sourbbn::Sourbbn oy1_bbn(oy1_path,false);

        std::time(&start);

        oy1_bbn.set_query(header_values,data_values,disease_var);
        //oy1_bbn.calc_means();
        oy1_bbn.calc_standard_devs();
        
        std::time(&finish);

	    std::cout << n_vars << " variables set = " << std::difftime(finish, start) << " seconds" << std::endl;

        header_values.pop_back();
        data_values.pop_back();
        //std::cout << header_values.at(i) << ": " << data_values.at(i) << std::endl;

    }

    

   
    
    //std::vector<std::string> header_values = {"state","JAUN"};
    //std::string disease_var  = "disease";
    //std::vector<int> data_values = {1,1};

    


}