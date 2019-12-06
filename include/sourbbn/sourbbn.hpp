//
// Created by jsanjak on 9/11/2019.
//

#ifndef SOURBBN_LIB_SOURBBN_HPP
#define SOURBBN_LIB_SOURBBN_HPP

#include <string>
#include <vector>
#include <memory>

namespace sourbbn {

//Here for compatibility with the mobile example
std::string from_sourbbn(const std::string &s1);
////////////////////////////////////////////////

class Sourbbn {
    private:
        class sourbbn_impl;
        std::unique_ptr<sourbbn_impl> sourbbn_pimpl;
    public:
        
        Sourbbn(const std::string &db_path);

        Sourbbn(const std::string &db_path, const bool & is_fake);

        void set_query(
            const std::vector<std::string> & evidence_vars,
            const std::vector<int> & evidence_values,
            const std::string & query_var);
        void calc_means();
        std::vector<float> read_means();
        void calc_standard_devs();
        std::vector<std::string> read_query_names();
        std::vector<float> read_standard_devs();

        ~Sourbbn();
        //TODO copy/move assign/construct 

};



}
#endif //SOURBBN_LIB_SOURBBN_HPP