#include <string>
#include "sourbbn/sourbbn.hpp"

#define CATCH_CONFIG_MAIN
#include "catch2/catch.hpp"

TEST_CASE("simple") {

    std::string to_repeat = "Hello";

    std::string repeated = sourbbn::from_sourbbn(to_repeat);

    REQUIRE( repeated == "Hello" );
    

}