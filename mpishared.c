#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

void printarray(int rank, char *arrayname, int *array, int istart, int istop);

int main(void)
{
  int i, flag;

  int nodesize, noderank;
  int size, rank, irank;
  int tablesize, localtablesize;
  int *table, *localtable;
  int *model;

  MPI_Comm allcomm, nodecomm;

  char verstring[MPI_MAX_LIBRARY_VERSION_STRING];
  char nodename[MPI_MAX_PROCESSOR_NAME];

  MPI_Aint winsize;
  int windisp;
  int *winptr;

  int version, subversion, verstringlen, nodestringlen;

  allcomm = MPI_COMM_WORLD;

  MPI_Win wintable;

  tablesize = 8;

  MPI_Init(NULL, NULL);

  MPI_Comm_size(allcomm, &size);
  MPI_Comm_rank(allcomm, &rank);

  MPI_Get_processor_name(nodename, &nodestringlen);

  MPI_Get_version(&version, &subversion);
  MPI_Get_library_version(verstring, &verstringlen);

  if (rank == 0)
    {
      printf("MPI version %d, subversion %d\n", version, subversion);
      printf("Library\n-------\n%s\n------\n", verstring);
    }

  // Create node-local communicator

  MPI_Comm_split_type(allcomm, MPI_COMM_TYPE_SHARED, rank,
              MPI_INFO_NULL, &nodecomm);

  MPI_Comm_size(nodecomm, &nodesize);
  MPI_Comm_rank(nodecomm, &noderank);

  // Only rank 0 on a node actually allocates memory

  localtablesize = 0;

  if (noderank == 0) localtablesize = tablesize;

  // Do this if you want all processes to allocate a subsection

  // localtablesize = tablesize/nodesize;

  // debug info

  printf("Rank %d of %d, rank %d of %d in node <%s>, localtablesize %d\n",
     rank, size, noderank, nodesize, nodename, localtablesize);


  MPI_Win_allocate_shared(localtablesize*sizeof(int), sizeof(int),
              MPI_INFO_NULL, nodecomm, &localtable, &wintable);

  MPI_Win_get_attr(wintable, MPI_WIN_MODEL, &model, &flag);

  if (1 != flag)
    {
      printf("Attribute MPI_WIN_MODEL not defined\n");
    }
  else
    {
      if (MPI_WIN_UNIFIED == *model)
        {
          if (rank == 0) printf("Memory model is MPI_WIN_UNIFIED\n");
        }
      else
        {
          if (rank == 0) printf("Memory model is *not* MPI_WIN_UNIFIED\n");
          
          MPI_Finalize();
          return 1;
        }
    }

  // Initialise table on rank 0 with appropriate synchronisation

  MPI_Win_fence(0, wintable);

  if (noderank == 0)
    {
      for (i=0; i < tablesize; i++)
        {
          localtable[i] = rank*tablesize + i;
        }
    }

  MPI_Win_fence(0, wintable);

  // Check we did it right

  printarray(rank, "localtable", localtable, 0, localtablesize);

  if (noderank == 0)
    {
      printarray(rank, "localtable", localtable, 0, tablesize);
    }
  else
    {
      printarray(rank, "localtable", localtable, -tablesize, 0);
    }

  table = NULL;

  // Set table manually
  
  table = localtable;

  if (noderank != 0) table = localtable - tablesize;

  /*
   *  Try broadcasting table from rank 0
   */
  
  //  MPI_Bcast(&table, sizeof(table), MPI_BYTE, 0, nodecomm);

  // need to get local pointer valid for table on rank 0

  /*
   *  Use MPI call to enquire what the noderank 0 pointer is
   */

  // if (noderank != 0)
  //   MPI_Win_shared_query(wintable, 0, &winsize, &windisp, &table);

  printf("On rank %d, table pointer = %p\n", rank, table);
  
  printarray(rank, "table", table, 0, tablesize);

  MPI_Finalize();
}

void printarray(int rank, char *arrayname, int *array, int istart, int istop)
{
  int i;
  printf("On rank %d, %s[%d:%d) = [", rank, arrayname, istart, istop);
  for (i=istart; i < istop; i++)
    {
      if (i != 0) printf(",");
      printf(" %d", array[i]);
    }
  printf(" ]\n");
}
