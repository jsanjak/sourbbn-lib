#include <string>
#include <typeinfo>
#include <iostream>
#include <memory>
#include "sourbbn/sourbbn.hpp"
#include "sourbbn/cptable.hpp"

#define CATCH_CONFIG_MAIN
#include "catch2/catch.hpp"

TEST_CASE("simple") {

    std::string to_repeat = "Hello";
    
    std::string repeated_string = sourbbn::from_sourbbn(to_repeat);

    REQUIRE( repeated_string == "Hello" );

}

//TODO: write tests for other types
TEST_CASE("Field Schema") {

    std::string field_name = "field_name";
    
    sourbbn::FieldType ft = sourbbn::FieldType::String;

    sourbbn::FieldSchema string_field(ft, field_name);

    sourbbn::RowSchema row_fields;

    std::string repeated_field = string_field.getName();
    
    row_fields.append(string_field);
    
    //Confirm Field Name 
    REQUIRE( repeated_field == "field_name" );

    //Confirm Field Type
    REQUIRE( ( typeid(string_field.getType())==typeid(ft) ) );

    //Confirm schema equality from within a RowSchema
    REQUIRE( string_field==row_fields.get(0) );
    
}

