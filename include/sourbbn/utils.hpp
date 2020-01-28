//Header only 
#ifndef SOURBBN_LIB_UTILS_HPP
#define SOURBBN_LIB_UTILS_HPP

#include <string>
#include <vector>

#include "sourbbn/cptable.hpp"

namespace sourbbn {

int standard_sqlite_callback(void* data, int argc, char** argv, char** azColName);

void print_cptable(CPTable &cp_table, bool print);
void print_row(RowValue &row, bool print);

CPTable expand(CPTable & t1, CPTable & t2);
CPTable elim(CPTable &cp_table,std::string var);
CPTable elim(CPTable &cp_table,std::vector<std::string> vars);

CPTable join(CPTable &t1, CPTable &t2);
CPTable join(const std::vector<CPTable> & tables);
CPTable d_join( CPTable dg_table, const std::vector<CPTable> & tables, std::size_t & ij);

std::string max_index(CPTable & b_tbl, std::vector<std::string> & variable_order_pi);

std::vector<std::string> scheme_diff(RowSchema & s1, RowSchema & s2 );
std::vector<std::string> scheme_overlap(RowSchema & s1, RowSchema & s2 );

}

#endif //SOURBBN_LIB_UTILS_HPP