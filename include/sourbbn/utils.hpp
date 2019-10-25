//Header only 
#ifndef SOURBBN_LIB_UTILS_HPP
#define SOURBBN_LIB_UTILS_HPP

#include <string>
#include <vector>

namespace sourbbn {

inline int standard_sqlite_callback(void* data, int argc, char** argv, char** azColName) 
{ 
    int i; 
    std::vector<std::string> *p = static_cast<std::vector<std::string>*>(data);
    for (i = 0; i < argc; i++) { 
        p->push_back(argv[i]);
    } 
    return 0; 
} 

}

#endif //SOURBBN_LIB_UTILS_HPP