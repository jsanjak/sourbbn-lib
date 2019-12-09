#include <string>
#include <typeinfo>
#include <iostream>
#include <memory>


#include "sourbbn/sourbbn.hpp"
#include "sourbbn/cptable.hpp"
#include "sourbbn/utils.hpp"


#define CATCH_CONFIG_MAIN
#include <sqlite3.h>
#include "catch2/catch.hpp"

TEST_CASE("simple") {

    std::string to_repeat = "Hello";
    
    std::string repeated_string = sourbbn::from_sourbbn(to_repeat);

    REQUIRE( repeated_string == "Hello" );

}

TEST_CASE("Public API"){
    
    std::string db_path = "test/data/diamond.sqlite";
    std::vector<std::string> evidence_vars = {"a","b","c"};
    std::string query_var  = "d";
    std::vector<int> query_1 = {0,0,0};
    
    sourbbn::Sourbbn test_bbn(db_path,false);

    REQUIRE_THROWS(test_bbn.set_query({"a","b"},{0,0,0},"d"));
    
    test_bbn.set_query(evidence_vars,query_1,query_var);
    
    test_bbn.calc_means();
    std::vector<float> test_means = test_bbn.read_means();
    
    test_bbn.calc_standard_devs();
    std::vector<float> test_standard_devs = test_bbn.read_standard_devs();
    
    std::vector<std::string> test_query_names = test_bbn.read_query_names();

    //TODO: Actual tests for these functions

    std::cout << "Means are: ";
    for (auto & m: test_means) {
       std::cout<< m << " ";
    };
    std::cout << std::endl;

    std::cout << "Std. Devs are: ";
    for (auto & s: test_standard_devs) {
       std::cout<< s << " ";
    };
    std::cout << std::endl;

    std::cout << "Vars are: ";
    for (auto & qn: test_query_names) {
       std::cout<< qn << " ";
    };
    std::cout << std::endl;
    
}

//TODO: write tests for other types
TEST_CASE("Test FieldSchema and RowSchema") {

    std::string field_name = "field_name";
    
    sourbbn::FieldType ft = sourbbn::FieldType::String;

    sourbbn::FieldSchema string_field(ft, field_name);

    //Can copy and assign
    sourbbn::FieldSchema copy_constructed_field(string_field);
    sourbbn::FieldSchema copy_assigned_field = string_field;
    sourbbn::FieldSchema copy_assigned_constructed_field = copy_assigned_field;

    sourbbn::RowSchema row_fields;

    REQUIRE( string_field == copy_constructed_field );
    REQUIRE( &string_field != &copy_constructed_field );

    REQUIRE( string_field == copy_assigned_field );
    REQUIRE( &string_field != &copy_assigned_field );

    REQUIRE( copy_assigned_constructed_field == copy_assigned_field );
    REQUIRE( &copy_assigned_constructed_field != &copy_assigned_field );    
    
    std::string repeated_field = string_field.get_name();
    
    row_fields.append(string_field);
    
    //Confirm Field Name 
    REQUIRE( repeated_field == "field_name" );

    bool failed_test = false;
    try{
       
       row_fields.append(copy_constructed_field); 

    } catch(const std::string& ex) {

        failed_test = true;

    };

    REQUIRE(failed_test);

    //Confirm Field Type
    REQUIRE( ( typeid(string_field.get_type())==typeid(ft) ) );

    //Confirm schema equality from within a RowSchema
    REQUIRE( string_field==row_fields.get(0));
    REQUIRE( string_field==field_name);
}

TEST_CASE("Test RowValue ") {

    sourbbn::FieldType my_bool = sourbbn::FieldType::Boolean;
    sourbbn::FieldType my_int = sourbbn::FieldType::Integer;
    sourbbn::FieldType my_float = sourbbn::FieldType::FloatingPoint;

    sourbbn::FieldSchema bool_field(my_bool,"test_boolean");
    sourbbn::FieldSchema int_field(my_int,"test_int");
    sourbbn::FieldSchema float_field(my_float,"test_float");

    sourbbn::RowSchema row_fields;

    row_fields.append(bool_field);
    row_fields.append(int_field);
    row_fields.append(float_field);
    
    sourbbn::RowValue row_values(row_fields);
    
    row_values.push_check<bool>(true);
    row_values.push_check<int>(10);
    row_values.push_check<float>(10.10);
    
    //Tests on the shared schema pointer
    REQUIRE( (*row_values.m_schema).get(0) == bool_field) ;
    REQUIRE( (*row_values.m_schema).get(0).get_type() == sourbbn::FieldType::Boolean );
    REQUIRE( (*row_values.m_schema).get_index("test_boolean") == 0 );
    
    bool failed_test = false;
    try{
       
       int rv = (*row_values.m_schema).get_index("bad_column");

    } catch(const std::out_of_range& oor) {

        failed_test = true;

    };
    REQUIRE( failed_test );

    REQUIRE( (*row_values.m_schema).get(1) == int_field );
    REQUIRE( (*row_values.m_schema).get(1).get_type() == sourbbn::FieldType::Integer );

    REQUIRE( (*row_values.m_schema).get(2) == float_field );
    REQUIRE( (*row_values.m_schema).get(2).get_type() == sourbbn::FieldType::FloatingPoint );
    
    //Tests on the data access
    REQUIRE( row_values.get(0) == true );
    REQUIRE( row_values.get(1) == 10 );
    REQUIRE( row_values.get(2) == float(10.10) );
    
}

TEST_CASE("Test CPTable Features"){

    sqlite3* DB;
    
    int rc;
    char *zErrMsg = 0;
    int exit = 0; 
    std::vector<std::string> data_table_list;
    std::vector<std::string> link_table_list;
    std::vector<std::string> static_data_table{"a","b","c","d"};
    std::vector<std::string> static_link_table{"a_link","b_link","c_link","d_link"};
    std::vector<std::string> static_column_names{"p","m","dist","d_id","b_id","c_id"};

    float total_prob = 4.0;
    int n_col_elim = 3;
    size_t n_row = 8;
    size_t n_row_elim = 4;

    sourbbn::CPTable a_table = sourbbn::CPTable();
    sourbbn::CPTable b_table = sourbbn::CPTable();
    sourbbn::CPTable c_table = sourbbn::CPTable();
    sourbbn::CPTable d_table = sourbbn::CPTable();
    
    /////////////////////////////////////////////
    exit = sqlite3_open("test/data/diamond.sqlite", &DB);
    
    REQUIRE(exit==0);
    
    std::string data_table_query("SELECT name FROM sqlite_master WHERE \
                             type ='table' AND name NOT LIKE 'sqlite_%' \
                             AND name NOT LIKE '%_link';");

    std::string link_table_query("SELECT name FROM sqlite_master WHERE \
                             type ='table' AND name NOT LIKE 'sqlite_%' \
                             AND name LIKE '%_link';");

    
    sqlite3_exec(DB, data_table_query.c_str(), sourbbn::standard_sqlite_callback, &data_table_list, &zErrMsg);
    

    REQUIRE(data_table_list==static_data_table);
    
    REQUIRE(static_link_table==static_link_table);


    /////////////Table Headers
    std::string a_header_query("SELECT * FROM " + data_table_list.at(0) + " LIMIT 1;");
    std::string b_header_query("SELECT * FROM " + data_table_list.at(1) + " LIMIT 1;");
    std::string c_header_query("SELECT * FROM " + data_table_list.at(2) + " LIMIT 1;");
    std::string d_header_query("SELECT * FROM " + data_table_list.at(3) + " LIMIT 1;");

    sqlite3_exec(DB, a_header_query.c_str(), a_table.schema_callback, &a_table.m_schema, &zErrMsg);
    sqlite3_exec(DB, b_header_query.c_str(), b_table.schema_callback, &b_table.m_schema, &zErrMsg);
    sqlite3_exec(DB, c_header_query.c_str(), c_table.schema_callback, &c_table.m_schema, &zErrMsg);
    sqlite3_exec(DB, d_header_query.c_str(), d_table.schema_callback, &d_table.m_schema, &zErrMsg);


    REQUIRE(d_table.m_schema.field_names()==static_column_names);
    

    /////////////Table Eliminations
    std::string a_table_query("SELECT * FROM " + data_table_list.at(0) + ";");
    std::string b_table_query("SELECT * FROM " + data_table_list.at(1) + ";");
    std::string c_table_query("SELECT * FROM " + data_table_list.at(2) + ";");
    std::string d_table_query("SELECT * FROM " + data_table_list.at(3) + ";");

    sqlite3_exec(DB, a_table_query.c_str(), a_table.data_callback, &a_table, &zErrMsg);
    sqlite3_exec(DB, b_table_query.c_str(), b_table.data_callback, &b_table, &zErrMsg);
    sqlite3_exec(DB, c_table_query.c_str(), c_table.data_callback, &c_table, &zErrMsg);
    sqlite3_exec(DB, d_table_query.c_str(), d_table.data_callback, &d_table, &zErrMsg);

    REQUIRE(d_table.column_sum("p") == total_prob);
    REQUIRE(d_table.m_rows.size() == n_row);

    sourbbn::CPTable d_elim_b = sourbbn::elim(d_table,"b_id");
    sourbbn::CPTable d_elim_c = sourbbn::elim(d_table,"c_id");
    sourbbn::CPTable d_elim_d = sourbbn::elim(d_table,"d_id");

    REQUIRE(d_elim_b.column_sum("p") == total_prob);
    REQUIRE(d_elim_b.m_rows.size() == n_row/2);

    REQUIRE(d_elim_c.column_sum("p") == total_prob);
    REQUIRE(d_elim_c.m_rows.size() == n_row/2);

    REQUIRE(d_elim_d.column_sum("p") == total_prob);
    REQUIRE(d_elim_d.m_rows.size() == n_row/2);
    

    /////////////Table Joins
    sourbbn::print_cptable(b_table,true);
    sourbbn::print_cptable(c_table,true);
    sourbbn::CPTable b_c_join = sourbbn::join(b_table,c_table);
    std::cout << "Joined Table:" << std::endl;
    sourbbn::print_cptable(b_c_join,false);
    std::cout << b_c_join.column_sum("p")<< std::endl;
    sqlite3_close(DB);

}

