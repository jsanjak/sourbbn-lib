//
// Created by jsanjak on 10/1/2019.
//

#include <string>
#include <type_traits>
#include "sourbbn/cptable.hpp"

namespace sourbbn {

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


void RowSchema::append(const FieldSchema &fs){

    m_fields.emplace_back(fs);

};

FieldSchema& RowSchema::get(const int &i){

    return m_fields.at(i);  

};

}