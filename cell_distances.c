#include <stdio.h> // printing to screan
#include <string.h>
#include <stdlib.h> // memory allocation

int n_threads;

int main(int argc, char *argv[])
{
  char *ptr; // pointer to parse input data.

  if ( argc == 2 ) {
    ptr = strchr( argv[1], 't'); // starts with 1 because 0 is the program name.
    n_threads = strtol(++ptr, NULL, 10);
  }
  else {
    printf("Missing arguments! Correct syntax is: cell_distances -t#numberOfThreads# \n");
    exit(0);
  }
  printf("N threads = %d\n", n_threads);
  return(0);
}
