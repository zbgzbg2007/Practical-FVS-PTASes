# INTRODUCTION

The code implements approximation algorithms for minimum feedback vertex set problem (FVS) in planar graphs, accompanying the paper **Designing Practical PTASes for Minimum Feedback Vertex Set in Planar Graphs**.


# Implemented Algorithms

* The 2-approximation algorithm of Becker and Geiger for FVS

  _Optimization of Pearl's method of conditioning and greedy-like approximation algorithms for the vertex feedback set problem_, 1996

* The linear kernel for FVS in planar graphs of Bonamy and Kowalik (we corrected their Reduction Rule 6)
  
  _A 13k-kernel for planar feedback vertex set via region decomposition_, 2016

* A polynomial-time approximation scheme (PTAS) for planar FVS based on the above kernel and Lipton and Tarjan's balanced separators

  _A separator theorem for planar graphs_, 1979

* Our Heuristic Approximation Scheme for planar FVS using a hybrid algorithm and local search heuristic

  _Designing Practical PTASes for Minimum Feedback Vertex Set in Planar Graphs_, 2018
  
  
# How to Use



## Requirements

* g++ 4.8.5 or higher

* Java 1.7 or higher

## How to Run

Our algorithm needs [Iwata and Imanishi's FPT implementation](https://github.com/wata-orz/fvs) as an exact algorithm.
So it should be copied and built before compile this code.
The code assumes by default the FPT algorithm is built in a folder called _fvs-solver_ in current repository, but it can be changed in the config file.



To compile the code and generate a executable file, say _fvs-alg_, run the following in current repository (ignore warnings):
```
g++ -std=c++11 -o fvs-alg ./*.cpp
```


To execute the generated file _fvs-alg_ with a configuration file, run the following in current repository:
```
./fvs-alg path_to_config_file
```




## Parameters in Configuration 

All parameter values in configuration file **_config.txt_** should not contain spaces.

* algorithm (integer): value indicates which algorithm to use (default: 4)
  - 0: an exact algorithm which first apply linear kernel and then apply Iwata. This may not be able to give a solution in a limited time.
and Imanishi's FPT implementation on the kernel
  - 1: the 2-approximation algorithm
  - 2: first apply linear kernel and then apply the 2-approximation algorithm on the kernel
  - 3: hybrid algorithm which alternates the greedy step of the 2-approximation and the kernelization
  - 4: Heuristic Approximation Scheme which starts with hybrid algorithm and then apply local search to improve the solution
  - 5: PTAS using linear kernel and balanced separator
  - 6: PTAS with post-processing, which will always return a minimal solution
  - 7: optimized PTAS with post-processing and recursively applied kernelization



* region\_size (positive integer): the largest size of decomposed graphs in PTAS (default: 60)

* input\_file (string): path of the input file

* output\_file (string): path of the output file

* write\_output (boolean): write solution to output file or not (default: 1)

* test\_result (boolean): test the solution or not, if set 1 then test if the solution is feasible for the original graph (default: 1)


* frequency (positive integer): the number of greedy steps between two consecutive kernelization in the hybrid algorithm (default: 41)


* t\_start (positive integer): the start value for parameter _t_ in local search heuristic (default: 3)

* t\_end (positive integer): the end value for parameter _t_ in local search heuristic (default: 30)

* t\_increase (positive integer): the each-time increase for parameter _t_ (default: 3)

* show\_improve (boolean): if print in console the local search improvement from each local search iteration (default: 0)

* fvs\_path (string): path of the FPT algorithm implementation (default: fvs-solver)

* time\_limit (positive integer): time limit for FPT algorithm to run in terms of seconds (default: 15)


* random\_seed (positive integer): random seed for local search (default: 41)


* terminal (positive integer): the local search will terminate for current value of _t_ if there are this number of consecutive local search iterations that find no improvement

## Input File Format

The implementation accepts two forms of input file:

- .graph file: each line consists of three integers _a_, _b_ and _c_, which means there are _c_ parallel edges between vertices _a_ and _b_. Each edge should only be defined by one line. See the example1.graph file.

- .ir file: each line consists of several integers separated by space as neighbor list where the first one is the vertex id and the remaining are its neighbors. Each vertex should only be defined by one line. See the example2.ir file.




# Authors

* Hung Le
* Baigong Zheng

