//
// Created by jsanjak on 9/16/2019.
//

#ifndef SOURBBN_LIB_CPTABLE_HPP
#define SOURBBN_LIB_CPTABLE_HPP

#include <string>
#include <vector>
#include <unordered_map>

namespace sourbbn {

//Credit https://stackoverflow.com/questions/29154673/how-to-implement-a-data-table-with-different-column-data-types-in-c/29155129
enum class FieldType {
  // Scalar types:
  Boolean, Integer, FloatingPoint, String

};

class FieldSchema {
    
  FieldType   m_type;
  std::string m_name;

  public:
    FieldSchema(FieldType mt, std::string mn);
    std::string getName() const;
    FieldType getType() const;
    bool operator==(const FieldSchema &other) const;

};


class RowSchema {

  std::vector<FieldSchema> m_fields;

  public:
    void append(const FieldSchema &fs);
    FieldSchema& get(const int &i);

};


union FieldValue {
  bool                     m_boolean;
  int                      m_integer;
  double                   m_floatingpoint;
  std::string              m_string;

};

class RowValue {
  RowSchema*               m_schama;
  std::vector<FieldValue>  m_fields;
};

struct CPTable{

    std::unordered_map<std::string, RowValue> m_rows;

};

}
#endif //SOURBBN_LIB_CPTABLE_HPP