# MPI shared memory

## Simple code to show how to use the MPI shared memory

Illustrates use of the MPI shared memory model to create a shared
lookup table: one copy per node but can be directly accessed by all
processes.

Note that, for simplicity, the code may make certain assumptions about
assignment of MPI processes to nodes that are not generally true
e.g. that they are assigned linearly.