//
// Created by jsanjak on 9/11/2019.
//

#ifndef SOURBBN_LIB_SOURBBN_HPP
#define SOURBBN_LIB_SOURBBN_HPP

#include <string>
#include <vector>
#include <memory>

#include "sourbbn/cptable.hpp"

namespace sourbbn {

std::string from_sourbbn(const std::string &s1);

/**
 * Sourbbn provides the interface to construct and query
 * Second-Order Uncertainty Bayesian Belief Networks (sou-bbn)
 * 
 * The tables that define the sou-bbn must be contained in an sqlite3 database.
 * Evidence and queries are set at the same time. The means and standard deviations
 * of the query, must be calculated sequentially.
 * 
 */
class Sourbbn {
    private:
        class sourbbn_impl;
        std::unique_ptr<sourbbn_impl> sourbbn_pimpl;
    public:
        
        Sourbbn(const std::string &db_path);

        /*
        Use the constructor with is_fake = true as the placeholder.
        When is_fake is set to true, the class will generate fake data.
        Specifically, it will return:
            - Query variable names of: anaplasmosis, rickettsiosis, lyme_disease and ehrlichiosis
            - Means of: 0.7, 0.24, 0.05 and 0.01
            - Std. devs of: 0.1, 0.05, 0.01 and 0.005

        Currently, when is_fake is false (default), it also give garbage data.
        However, that will change as the real algorithm is implemented.
        */
        Sourbbn(const std::string &db_path, const bool & is_fake);

        void set_query(
            const std::vector<std::string> & evidence_vars,
            const std::vector<int> & evidence_values,
            const std::string & query_var);

        void calc_means();
        void calc_standard_devs();
        std::vector<std::string> read_cptable_names();
        std::vector<std::string> read_query_names();
        std::vector<float> read_means();
        std::vector<float> read_standard_devs();
        
        ~Sourbbn();
        
        //CPTable& get_table(std::string & tbl_name);
        //CPTable& get_table(std::string && tbl_name);

        //TODO copy/move assign/construct 

};



}
#endif //SOURBBN_LIB_SOURBBN_HPP