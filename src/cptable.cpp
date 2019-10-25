//
// Created by jsanjak on 10/1/2019.
//

#include <memory>
#include <string>
#include <type_traits>
#include <algorithm> 
#include <stdexcept>      
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


std::string FieldSchema::getName() const {

    return m_name;

};

FieldType FieldSchema::getType() const {

    return m_type;

};

bool FieldSchema::operator==(const FieldSchema &other) const {
    //If name and type are equal, then the field schemas are equivalent
    return ( m_name == other.getName() && ( typeid(m_type)==typeid(other.getType()) ) );

};

bool FieldSchema::operator==(const std::string &s) const {
    //If name matches, then this works
    //Rely on RowSchema to enforce unique column names
    return ( m_name == s );

};


void RowSchema::append(const FieldSchema &fs){
    //Enforces unique columns by name, even if you try to add
    //a column with same name but different type
    decltype(m_fields)::iterator it;
    it = find(m_fields.begin(), m_fields.end(), fs.getName());
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
    }
    
};

//
FieldValue::FieldValue(){};

FieldValue::FieldValue(bool b){

    m_boolean = b;

};

FieldValue::FieldValue(int i){

    m_integer = i;

};

FieldValue::FieldValue(double d){

    m_floatingpoint = d;

};

//Row value member functions
RowValue::RowValue() : m_schema(std::make_shared<RowSchema>()) {

    std::vector<FieldValue>  m_fields;

};

RowValue::RowValue(RowSchema &ms): m_schema(std::make_shared<RowSchema>(ms)){

    std::vector<FieldValue>  m_fields;

};

/*FieldValue& RowValue::get(const int &i){
    sourbbn::FieldSchema fs(*(m_schema).at(i))

};*/

//Default constructor
CPTable::CPTable(){

    RowSchema m_schema;
    std::vector<RowValue> m_rows;

} 

CPTable::CPTable(const RowSchema &ms){

    //?Does this need a copy/assignment constructor??
    RowSchema m_schema = ms;
    std::vector<RowValue> m_rows;

} 
/*
CPTable::CPTable(const RowSchema &ms, const std::vector<RowValue> &mr){

    //?Does this need a copy/assignment constructor??
    RowSchema m_schema = ms;
    std::vector<RowValue> m_rows = mr;

} */

int CPTable::SchemaCallback(void* data, int argc, char** argv, char** azColName){ 
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

/*
int CPTable::DataCallback(void* data, int argc, char** argv, char** azColName){ 
    int i; 
    std::vector<RowValue> *p = static_cast<std::vector<RowValue>*>(data);

    RowValue row_data;

    for (i = 0; i < argc; i++) {

        if (i<2){

            schema_field = FieldSchema(FieldType::FloatingPoint, azColName[i]);
        
        } else {

            schema_field = FieldSchema(FieldType::Integer, azColName[i]);
        
        }
        
        p->append(schema_field);

    } 
    return 0; 
} */



}//END NAMESPACE