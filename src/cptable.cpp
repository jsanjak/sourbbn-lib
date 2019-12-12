//
// Created by jsanjak on 10/1/2019.
//

#include <memory>
#include <string>
#include <type_traits>
#include <algorithm> 
#include <stdexcept>
#include <stdlib.h>
#include <iostream>
#include <iterator>
#include <unordered_map>    
#include <utility>
#include "sourbbn/cptable.hpp"

namespace sourbbn {

FieldSchema::FieldSchema(){

    FieldType m_type;
    std::string m_name;
    
};

FieldSchema::FieldSchema(FieldType mt, std::string mn){

    m_type = mt;
    m_name = mn;
    
};


std::string FieldSchema::get_name() const {

    return m_name;

};

FieldType FieldSchema::get_type() const {

    return m_type;

};

bool FieldSchema::operator==(const FieldSchema &other) const {
    //If name and type are equal, then the field schemas are equivalent
    return ( m_name == other.get_name() && ( typeid(m_type)==typeid(other.get_type()) ) );

};

bool FieldSchema::operator==(const std::string &s) const {
    //If name matches, then this works
    //Rely on RowSchema to enforce unique column names
    return ( m_name == s );

};

RowSchema::RowSchema(){
    std::vector<FieldSchema> m_fields;
};

void RowSchema::append(const FieldSchema &fs){
    //Enforces unique columns by name, even if you try to add
    //a column with same name but different type
    decltype(m_fields)::iterator it;
    it = find(m_fields.begin(), m_fields.end(), fs.get_name());
    if(it == m_fields.end()){

        m_fields.emplace_back(fs);

    } else {

        throw std::string("Require unique columns names");
    
    };
    
  
};

FieldSchema& RowSchema::get(const int &i){

    return m_fields.at(i);  

};

int RowSchema::get_index(const std::string fname){

    decltype(m_fields)::iterator it;
    it = std::find(m_fields.begin(),m_fields.end(),fname);
    if(it !=  m_fields.end()){
        return (it - m_fields.begin());
    } else {
        throw std::out_of_range(fname + " not in schema");
    };
    
};

std::vector<std::string> RowSchema::field_names(){
    std::vector<std::string>f_names;
    for (auto & f : m_fields){
        f_names.emplace_back(f.get_name());
    }
    return (f_names);
};


FieldValue::FieldValue(){};

FieldValue::FieldValue(bool b){

    //field_type = FieldType::Boolean;
    m_boolean = b;

};

FieldValue::FieldValue(int i){

    //field_type = FieldType::Integer;
    m_integer = i;

};

FieldValue::FieldValue(float f){

    //field_type = FieldType::FloatingPoint;
    m_floatingpoint = f;

};

bool FieldValue::operator==(const bool &b) const {

    return ( m_boolean == b );

};

bool FieldValue::operator==(const int &i) const {

    return ( m_integer == i );

};

bool FieldValue::operator==(const float &f) const {

    return ( m_floatingpoint == f );

};

//Row value member functions
RowValue::RowValue() : m_schema(std::make_shared<RowSchema>()) {

    std::vector<FieldValue>  m_fields;

};

RowValue::RowValue(RowSchema &ms): m_schema(std::make_shared<RowSchema>(ms)){

    std::vector<FieldValue>  m_fields;

};

//Push and check arbitrary type against schema
template<typename T> void RowValue::push_check(T val){

    if ( (*m_schema).m_fields.empty() ){

        //throw exception for trying to push with an empty schema

    } else {

        std::size_t i = m_fields.size();
        
        if (i>=(*m_schema).m_fields.size()){

            //throw out of range error

        } else {
            
            FieldType ft = (*m_schema).get(i).get_type();

            if ( ft == FieldType::Boolean ){

                if ( typeid(val)==typeid(bool) ){

                    m_fields.emplace_back(val);

                } else {

                    //schema error

                };

            } else if ( ft == FieldType::Integer ){

                if ( typeid(val)==typeid(int) ){

                     m_fields.emplace_back(val);

                } else {

                    //schema error

                };

            } else if (  ft == FieldType::FloatingPoint ){

                if ( typeid(val)==typeid(float) ){

                     m_fields.emplace_back(val);

                } else {

                    //schema error

                };

            } else {

                //throw type error

            };  
        };
    };
};
template void RowValue::push_check<int>(int);
template void RowValue::push_check<bool>(bool);
template void RowValue::push_check<float>(float);

FieldValue& RowValue::get(const int &i) {

    //Wrapper to at because we don't want to make m_fields public,
    //so we can enforce the schema
    return( m_fields.at(i) );

};

void RowValue::add(const int &i,const float & fv){

    //should check that field value at i is int
    m_fields.at(i).m_floatingpoint += fv;

};
//Default constructor
CPTable::CPTable(){

    RowSchema m_schema;
    std::vector<RowValue> m_rows;
    std::string table_name;

} 
CPTable::CPTable(const RowSchema &ms): m_schema(ms) {

    std::vector<RowValue> m_rows;
    std::string table_name;
} 
//copy constructor
CPTable::CPTable(const CPTable & other_table):
m_schema(other_table.m_schema),m_rows(other_table.m_rows),table_name(other_table.table_name+"_copy")
{
    
};

//move constructor
CPTable::CPTable(CPTable&& other_table) noexcept :
m_schema(std::move(other_table.m_schema)),m_rows(std::move(other_table.m_rows))
{
    
};

//copy assignment
CPTable &CPTable::operator=(const CPTable& other_table){

    if (this != &other_table) {

        m_schema = other_table.m_schema;
        m_rows = other_table.m_rows;
        
    }
    return *this; 
};

//move assignment
CPTable &CPTable::operator=(CPTable&& other_table) noexcept {
    if (this != &other_table) {
        m_schema = std::move(other_table.m_schema);
        m_rows = std::move(other_table.m_rows);
    }
    return *this; 
};

CPTable::~CPTable(){

};//Destructor

RowSchema CPTable::scheme(){
    RowSchema new_schema;
    for (auto fs : m_schema.m_fields ){
        std::unordered_map<std::string,int>::const_iterator prot = protected_names.find(fs.get_name());  
        if (prot == protected_names.end()){
            new_schema.append(fs);
        };
    };
    return(new_schema);
};

int CPTable::schema_callback(void* data, int argc, char** argv, char** azColName){ 
    int i; 
    FieldSchema schema_field;
    RowSchema *p = static_cast<RowSchema*>(data);
    for (i = 0; i < argc; i++) {

        if (i<2){

            schema_field = FieldSchema(FieldType::FloatingPoint, azColName[i]);
        
        } else {

            schema_field = FieldSchema(FieldType::Integer, azColName[i]);
        
        }
        
        p->append(schema_field);

    } 
    return 0; 
} 


int CPTable::data_callback(void* data, int argc, char** argv, char** azColName){ 
    
    int i; 
    
    CPTable *cpt = static_cast<CPTable*>(data);

    RowValue row_data( (*cpt).m_schema );

    for (i = 0; i < argc; i++) {
        //Check incase we're out of range
        FieldType ft = (*cpt).m_schema.get(i).get_type();

        if ( ft == FieldType::Boolean){

            row_data.push_check<bool>((bool)atoi(argv[i]));

        } else if ( ft == FieldType::Integer) {

            row_data.push_check<int>(atoi(argv[i]));

        } else if ( ft == FieldType::FloatingPoint){

             row_data.push_check<float>((float)atof(argv[i]));

        } else {

            //fail

        };
    };
    cpt->m_rows.emplace_back(row_data);
    return 0; 
} 

float CPTable::column_sum(const std::string &var){

    float col_sum = 0.0;
    int var_index = m_schema.get_index(var);
    for (auto & row: m_rows) {
        col_sum += row.get(var_index).m_floatingpoint;
    }
    return (col_sum);
};

}//END NAMESPACE