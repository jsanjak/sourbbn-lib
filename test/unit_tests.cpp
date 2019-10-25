#include <string>
#include <typeinfo>
#include <iostream>
#include <memory>

#include "sourbbn/sourbbn.hpp"
#include "sourbbn/cptable.hpp"
#include "sourbbn/utils.hpp"


#define CATCH_CONFIG_MAIN
#include  <sqlite3.h>
#include "catch2/catch.hpp"

TEST_CASE("simple") {

    std::string to_repeat = "Hello";
    
    std::string repeated_string = sourbbn::from_sourbbn(to_repeat);

    REQUIRE( repeated_string == "Hello" );

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
    
    std::string repeated_field = string_field.getName();
    
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
    REQUIRE( ( typeid(string_field.getType())==typeid(ft) ) );

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
    
    row_values.m_fields.emplace_back(true);
    row_values.m_fields.emplace_back(10);
    row_values.m_fields.emplace_back(10.10);
    
    REQUIRE( (*row_values.m_schema).get(0) == bool_field);
    REQUIRE( (*row_values.m_schema).get(0).getType() == sourbbn::FieldType::Boolean);
    REQUIRE( (*row_values.m_schema).get_index("test_boolean") == 0);
    
    bool failed_test = false;
    try{
       
       int rv = (*row_values.m_schema).get_index("bad_column");

    } catch(const std::out_of_range& oor) {

        failed_test = true;

    };
    REQUIRE( failed_test );

    REQUIRE( (*row_values.m_schema).get(1) == int_field);
    REQUIRE( (*row_values.m_schema).get(1).getType() == sourbbn::FieldType::Integer);

    REQUIRE( (*row_values.m_schema).get(2) == float_field);
    REQUIRE( (*row_values.m_schema).get(2).getType() == sourbbn::FieldType::FloatingPoint);
    
}

TEST_CASE("Create a row schema"){

    sqlite3* DB;
    
    int rc;
    char *zErrMsg = 0;
    int exit = 0; 
    std::vector<std::string> data_table_list;
    std::vector<std::string> link_table_list;
    std::vector<std::string> example_data;
    sourbbn::CPTable basic_table = sourbbn::CPTable();
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
    
    ///////////// TODO write assertion here
    std::cout << "Data table names are:" << std::endl;

    for (auto const& i: data_table_list) {
		std::cout << i << " ";
	}
    std::cout << std::endl;


    ///////////// TODO write assertion here
    sqlite3_exec(DB, link_table_query.c_str(), sourbbn::standard_sqlite_callback, &link_table_list, &zErrMsg);
    
    std::cout << "Link table names are:" << std::endl;
    
    for (auto const& i: link_table_list) {
		std::cout << i << " ";
	}
    std::cout << std::endl;


    ///////////// TODO write assertion here
    std::string example_header_query("SELECT * FROM " + data_table_list.at(0) + " LIMIT 1;");
    
    sqlite3_exec(DB, example_header_query.c_str(), basic_table.SchemaCallback, &basic_table.m_schema, &zErrMsg);

    std::cout << "Table column names are:" << std::endl;
    for (auto const& i: basic_table.m_schema.m_fields) {
		std::cout << i.getName() << " ";
	}
    std::cout << std::endl;

    /////////////
    sqlite3_close(DB);

}

