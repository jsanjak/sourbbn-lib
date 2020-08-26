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
    
    std::vector<double> test_means = test_bbn.read_means();

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
    std::string diamond_path = "test/data/diamond.sqlite";
    std::string diamond_var = "d";
    evidence_vars = {"a","c","b"};
    query_1 = {1,1,1};
    
    std::vector<double> first_means;
    std::vector<double> first_std;
    std::vector<std::string> first_names;

    std::string net_type = "tbd";
    if(net_type=="diamond"){
        sourbbn::Sourbbn oy1_bbn(diamond_path,false);
        std::vector<std::string> cptable_names = oy1_bbn.read_cptable_names();
        
        oy1_bbn.set_query(evidence_vars,query_1,diamond_var);
        //  oy1_bbn.calc_means();
        oy1_bbn.calc_standard_devs();
        first_means = oy1_bbn.read_means();
        first_std = oy1_bbn.read_standard_devs();
        first_names= oy1_bbn.read_query_names();
    
    } else if (net_type=="tbd"){

        sourbbn::Sourbbn oy1_bbn(oy1_path,false);
        std::vector<std::string> cptable_names = oy1_bbn.read_cptable_names();
        
        //std::vector<std::string> header_values = {"state","JAUN"};
        //std::string disease_var  = "disease";
        //std::vector<int> data_values = {1,1};

        oy1_bbn.set_query(header_values,data_values,disease_var);
        //oy1_bbn.calc_means();
        oy1_bbn.calc_standard_devs();
        first_means = oy1_bbn.read_means();
        first_std = oy1_bbn.read_standard_devs();
        first_names= oy1_bbn.read_query_names();
    }
    std::cout << "Mean, Std. Dev: " << std::endl;;
    for (std::size_t i=0;i!=first_means.size(); ++i){
        std::cout << first_means.at(i) << ',';
        std::cout << first_std.at(i) << std::endl;
    }


    //Confidence metric stuff
    std::vector<double> test_case = 
    {
        0.8406321, 2.41000e-21, 0.1284195650, 4.76000e-15, 2.765143e-02, 9.23e-21,
        1.854498e-03, 4.78e-15, 5.941360e-04, 4.95000e-20, 4.275380e-04, 5.32e-23
    };
    std::vector<std::vector<std::vector<double>>> model_weights = 
    {
        {
            {2.276943,-1.79556889, 1.134883},
            {-5.823954,  1.47627497,   -1.614081},
            { 107.369620, -0.70660432,  -17.518970},
            {1677.840421, -0.65770028,  43.798467},
            {-85.417529, -0.24298551,   22.376288},
            {111.514040, -0.19916359,   88.143804},
            {-136.394348, -0.27057173,   13.092243},
            {923.591191, -1.83785209,  174.817614},
            {-275.894449, -0.07940933,  51.257397},
            {499.876273, -2.97093154, -289.789083},
            {-419.212281, -0.50587292,  -14.753628},
            {1668.422618,  5.25742057,  177.055637},
            {-31.064140,  0.57697315,   27.215548}
        },
        {
            { 2.802455},
            {-2.696183},
            {10.131903},
            {-3.764107}
        }
    };    
    
    
    double test_confidence = sourbbn::model_confidence(test_case,model_weights);
    std::cout << "Confidence metric: " << test_confidence << std::endl;
    /*
    std::ifstream gerfile ("test/data/germany_query_example.txt");
    header_values.clear();
    data_values.clear();
    std::getline(gerfile, header_line);
    std::getline(gerfile, data_line);

    split(header_line, '\t', header_values);
    split_toint(data_line, '\t', data_values);
    
    header_values.erase(header_values.begin());
    data_values.erase(data_values.begin());

    std::string ger_path = "test/data/germany_disease_db.sqlite";
    sourbbn::Sourbbn ger_bbn(ger_path,false);
    std::vector<std::string> cptable_names = ger_bbn.read_cptable_names();

    ger_bbn.set_query(header_values,data_values,disease_var);
    ger_bbn.calc_means();
    ger_bbn.calc_standard_devs();
    first_means = ger_bbn.read_means();
    first_std = ger_bbn.read_standard_devs();
    first_names= ger_bbn.read_query_names();
    
    std::cout << "Germany DB Query: " << std::endl;
    std::cout << "Mean, Std. Dev: " << std::endl;
    for (std::size_t i=0;i!=first_means.size(); ++i){
        std::cout << first_means.at(i) << ',';
        std::cout << first_std.at(i) << std::endl;
    }*/

}