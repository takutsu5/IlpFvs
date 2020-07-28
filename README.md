# IlpFvs
IlpFvs - computing a minimum feedback vertex set for a directed graph using integer linear programming

# Description
This is a program for computing a minimum feedback vertex set for a directed graph using integer linear programming (ILP), where IBM ILOG CPLEX must be used as an ILP solver. Note that a feedback vertex set is a set of vertices removal of which makes the graph acyclic, and a minimum feedback vertex set is a feedback vertex set with the minimum cardinality.

# Installation
gcc -o convfile convfile.c
gcc -o ilpfvs ilpfvs.c

# Requirement
IBM ILOG CPLEX must be available from the current directory. It seems that most recent versions are OK although I tested Version 12.7.1.0 only.

# Usage
convfile inputfilename<br>
ilpfvs outputfilename

Note that 'convfile' converts an input file into 'neuronodes.txt' and 'neuroedges.txt' files, and then 'ilpfvs' computes a minimum feedback vertex set for a graph specified by these two files. Several temporal files will also be created.

# Input file
An input directed graph should be given as a text file in the form of adjacency matrix.
For example, if an input graph consists of egdes, (v1,v3),(v2,v1),(v2,v2),(v3,v2),(v3,v4),(v4,v1), the input file (e.g., "example.txt") should be:

----------------
0,0,1,0<br>
1,1,0,0<br>
0,1,0,1<br>
1,0,0,0

----------------
Note that there is an edge (vi,vj) if and only if the value of (i,j)-th element is 1.
Note also that isolated vertices (i.e., vertices with no edges) will be ignored.

# Output file
IlpFvs outputs a text file where each row consists of either 0 or 1, where i-th row corresponds to i-th vertex, and  1 means that the corresponding vertex belongs to some specific minimum feedback vertex set. For example, the following file will be output for the graph given above.

----------------
1<br>
1<br>
0<br>
0<br>

----------------
It means that v1 and v2 are in a minimum feedback vertex set (MFVS). Note that {v2,v3} and {v2,v4} are also MFVSs. However, IlpFvs outputs one MFVS only, and selection of the MFVS depends on the ILP solver.

# Example File
example.txt 

# Example
$ ./convfile example.txt <br>
#nodes = 4    #edges = 6 <br>
$ ./ilpfvs output.txt <br>
#nodes = 4    #relevant nodes = 4    #edges = 6 <br>
#FvsNodes = 2 <br>

Then, you can see from "output.txt" that {v1,v2} is a minimum feedback vertex set.

# Parameters
The code includes several parameters. Since memory spaces are not dynamically allocated, parameter values should be appropriately adjusted depending on the size of a network, and so on. If the number of nodes or edges is large, huge CPU time may be needed or CPLEX may fail with an error message. 

If you want to adjust parameters, please understand details of the code (since there are almost no comment lines, I do not recommend it.)
However, the code cannot handle big graphs and thus adjustments of parameters may be meaningless.

# Reference
IlpFvs was developed for analysis of real neural networks discussed in the following manuscript.<br>
M. Kajiwara et al.: Inhibitory neurons are a central controlling regulator in the effective cortical microconnectome,<br>
bioRxiv, doi.org/10.1101/2020.02.18.954016.

IlpFvs uses the ILP formulation given in the following paper, where IlpFvs does not include any preprocessing or functions for computing critical and redundant vertices.<br>
Y. Bao et al.: Analysis of critical and redundant vertices in controlling directed complex networks using feedback vertex sets.<br>
Journal of Computational Biology, 25(10):1071-1090, 2018,

