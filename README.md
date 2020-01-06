# Sourbbn-lib: C++ Library for solving Second Order Uncertainy in Bayesian Belief Networks

Bayesian Belief Networks  (BBNs) are a type of probabilistic graphical model where the joint distribution over all variables in the model is represented by directed acyclic graph. Nodes in the graph represent the probability distribution of a given variable conditional on it's parents in the graph. These conditional distributions are stored as conditional probability tables. The values in conditional probability tables are often inferred from data and are not known exactly. This library provides a set of C/C++ classes and functions implementing the MeanVar and BuckElim+ algorithms, extracted from [Van Allen et al. 2008](https://doi.org/10.1016/j.artint.2007.09.004),  for querying BBNs with uncertain data.

## Build 

Sourbbn-lib is built with CMake and is compatible with gcc v7.3. To build the library locally do the following

```
cd sourbbn-lib/
cmake . 
make
```

## Documentation

## Test/Examples
After building the library, you can explore the tests. To execute the tests 
run the following commands from the root directory

```
./unit_tests -s
./scratch_test
```
The two test source files contain

 - Unit tests: **test/unit_test.cpp**

 - Examples: **test/scratch_test.cpp**

