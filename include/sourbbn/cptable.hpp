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
    std::string get_name() const;
    FieldType get_type() const;
    bool operator==(const FieldSchema &other) const;
    bool operator==(const std::string &s) const;
};


class RowSchema {

  public:
    RowSchema();
    std::vector<FieldSchema> m_fields;
    void append(const FieldSchema &fs);
    FieldSchema& get(const int &i);
    int get_index(const std::string fname);
    std::vector<std::string> field_names();
    
};

union FieldValue {
  
  //FieldType   field_type;
  bool                    m_boolean;
  int                     m_integer;
  float                   m_floatingpoint;

  FieldValue();
  FieldValue(bool b);
  FieldValue(int i);
  FieldValue(float f);
  bool operator==(const bool &b) const;
  bool operator==(const int &i) const;
  bool operator==(const float &f) const;
  
};

class RowValue {
  
  std::vector<FieldValue>  m_fields;

  public:
    std::shared_ptr<RowSchema>  m_schema;
    RowValue();
    RowValue(RowSchema &ms);
    /*The use of a shared pointer schema 
    effectivley tags the FieldValue unions.

    However, this isn't a robust tag, like in C++17/boost variant.
    And so we have to enforce the schema upon construction of a 
    FieldValue 
    */
    template<typename T> void push_check(T val);
    FieldValue& get(const int &i);
    void add(const int &i,const float & fv);
};

// p m var1 var2...
// float float int int ...
struct CPTable{

    RowSchema m_schema;
    std::vector<RowValue> m_rows;

    //cannot be used for elimination or joins
    const std::unordered_map<std::string,int> protected_names = {{"p",1},{"m",1},{"dist",1}};

    CPTable();
    CPTable(const RowSchema &ms);
    CPTable(const CPTable & other_table);//copy constructor
    CPTable(CPTable&& other_table) noexcept;//move constructor
    CPTable& operator=(const CPTable& other_table);//copy assignment
    CPTable& operator=(CPTable&& other_table) noexcept;//move assignment
    ~CPTable();//Destructor

    static int schema_callback(void* data, int argc, char** argv, char** azColName);
    static int data_callback(void* data, int argc, char** argv, char** azColName);

    float column_sum(const std::string &var);
};

}
#endif //SOURBBN_LIB_CPTABLE_HPP