# EECS583-FinalProject
EECS 583 Final Project

# How this project works?
We generate a C file that implements regular matrix multiplication of a certain size. This C program reads in 3 matrices (X, Y, and Z such that XY = Z). We run a pass on this program to generate the tiled matrix multiplication version of it, utilizing differernt algorithms to select tile sizes. Finally, we run the untiled version and the tiled version using cachegrind to simulate specific cache characteristics (32/64 KB sizes, fully associative, 64 B cache lines) to get the number of L1 Data cache misses. 

We tested this code on class server b, where valgrind and the necessary tools are installed.

In this repo, we have 4 passes to convert regular matrix multiplication into tiled matrix multiplication:
  - Square root of cache size
  - Lam et al's tile selection algorithm
  - Factor-based tile selection (choose tile sizes based on the factors of the matrix dimenstions)
  - LRU-based tile selection

To run (\<N\> is the matrix dimension for an NxN matrix):
  - Square root: `./test_sqrt <N>`
  - Lam et al.: `./test_lam <N>`
  - Factor-based: `./test_factor <N>`
  - LRU-based: `./test_lru <N>`
  - All algos: `./run.sh <N>`
