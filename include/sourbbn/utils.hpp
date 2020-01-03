//Header only 
#ifndef SOURBBN_LIB_UTILS_HPP
#define SOURBBN_LIB_UTILS_HPP

#include <string>
#include <vector>

#include "sourbbn/cptable.hpp"

namespace sourbbn {

int standard_sqlite_callback(void* data, int argc, char** argv, char** azColName);

void print_cptable(CPTable &cp_table, bool print);

CPTable elim(CPTable &cp_table,std::string var);
CPTable elim(CPTable &cp_table,std::vector<std::string> vars);

CPTable join(CPTable &t1, CPTable &t2);
CPTable join(const std::vector<CPTable> & tables);

std::string max_index(CPTable & b_tbl, std::vector<std::string> & variable_order_pi);

}

#endif //SOURBBN_LIB_UTILS_HPP