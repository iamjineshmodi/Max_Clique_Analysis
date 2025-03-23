# Maximal Clique Enumeration Algorithm Implementation

## Overview
This repository contains implementations of three different maximal clique enumeration algorithms from the following papers:
1. The worst-case time complexity for generating all maximal cliques and computational experiments - Procedure CLIQUE (p.31)
2. Listing All Maximal Cliques in Sparse Graphs in Near-Optimal Time - BronkerboschDegeneracy (Fig. 4)
3. Arboricity and Subgraph Listing Algorithms - Procedure CLIQUE (pp.220-222)

All implementations are in C++ for optimal performance and consistent comparison of runtime.

## Execution Instructions

### Prerequisites
- C++ compiler with C++11 support (GCC 4.8+ or Clang 3.3+)

### Building the Project
1. Compile the source code with optimizations:
```bash
g++ -O3 -march=native -flto -funroll-loops -ffast-math -o daa1 daa1.cpp
```

2. Run the program by providing the dataset name without the .txt suffix:
```bash
./daa1 email-Enron
```

## Dataset Preparation

1. Download the datasets from the following links and save them as .txt files:
   - https://snap.stanford.edu/data/email-Enron.html
   - https://snap.stanford.edu/data/as-Skitter.html
   - https://snap.stanford.edu/data/wiki-Vote.html

2. Remove the comment lines at the beginning of each file

3. Add a header line with the number of nodes and edges:
   ```
   num_of_nodes num_of_edges
   ```

### Input File Format
The program accepts graph files in the following edge list format:
```
num_of_nodes num_of_edges
u1 v1
u2 v2
...
um vm
```
Where:
- Each line `ui vi` represents an undirected edge between vertices `ui` and `vi`

## Implementation Details

### Algorithm 1: Tomita et al.
Implementation of the CLIQUE procedure from "The worst-case time complexity for generating all maximal cliques and computational experiments" by Tomita et al.

Key features:
- Pivot selection to minimize branching
- O(3^(n/3)) worst-case time complexity
- Recursive backtracking approach

### Algorithm 2: BronkerboschDegeneracy
Implementation based on "Listing All Maximal Cliques in Sparse Graphs in Near-Optimal Time".

Key features:
- Degeneracy ordering of vertices
- Pruning based on vertex ordering
- Optimal for sparse graphs

### Algorithm 3: Arboricity-based
Implementation of CLIQUE procedure from "Arboricity and Subgraph Listing Algorithms".

Key features:
- Exploits graph arboricity for improved performance
- Efficient for graphs with low arboricity
- Theoretical complexity related to graph arboricity

## Individual Contributions

- **Algorithm Implementation**:
  - Tomita et al. algorithm: Om Patil, Kartik Maheshwari
  - BronkerboschDegeneracy algorithm: Kartik Maheshwari, Karan Sethia
  - Arboricity-based algorithm: Meghaditya Giri, Jinesh Modi
  
- **Documentation**:
  - Implementation documentation: Jinesh Modi
  - Results analysis and reporting: Karan Sethia
  - Project webpage creation: Om Patil

## References

1. Tomita, E., Tanaka, A., & Takahashi, H. (2006). The worst-case time complexity for generating all maximal cliques and computational experiments. Theoretical Computer Science, 363(1), 28-42.
2. Eppstein, D., Löffler, M., & Strash, D. (2010). Listing All Maximal Cliques in Sparse Graphs in Near-Optimal Time. In Algorithms and Computation (pp. 403-414). Springer.
3. Chiba, N., & Nishizeki, T. (1985). Arboricity and Subgraph Listing Algorithms. SIAM Journal on Computing, 14(1), 210-223.
