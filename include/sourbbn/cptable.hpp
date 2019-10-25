//
// Created by jsanjak on 9/16/2019.
// Inspired by https://stackoverflow.com/questions/29154673/how-to-implement-a-data-table-with-different-column-data-types-in-c/45726354#45726354
//

#ifndef SOURBBN_LIB_CPTABLE_HPP
#define SOURBBN_LIB_CPTABLE_HPP

#include <memory>
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
    FieldSchema();
    FieldSchema(FieldType mt, std::string mn);
    std::string getName() const;
    FieldType getType() const;
    bool operator==(const FieldSchema &other) const;
    bool operator==(const std::string &s) const;
};


class RowSchema {

  public:
    std::vector<FieldSchema> m_fields;
    void append(const FieldSchema &fs);
    FieldSchema& get(const int &i);
    int get_index(const std::string fname);

};

union FieldValue {
  bool                     m_boolean;
  int                      m_integer;
  double                   m_floatingpoint;
  FieldValue();
  FieldValue(bool b);
  FieldValue(int i);
  FieldValue(double d);
  
};

class RowValue {
  public:
    std::shared_ptr<RowSchema>  m_schema;
    std::vector<FieldValue>  m_fields;
    RowValue();
    RowValue(RowSchema &ms);
    //FieldValue& get(const int &i);
    //FieldValue& get(const std::string &s); 

};

struct CPTable{
    RowSchema m_schema;
    std::vector<RowValue> m_rows;
    CPTable();
    CPTable(const RowSchema &ms);
    //CPTable(const RowSchema &ms, const std::vector<RowValue> &mr);

    static int SchemaCallback(void* data, int argc, char** argv, char** azColName);
    static int DataCallback(void* data, int argc, char** argv, char** azColName);

};

}
#endif //SOURBBN_LIB_CPTABLE_HPP