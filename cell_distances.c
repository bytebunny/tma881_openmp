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

  /////////////////////////// Read input data from file ////////////////////////
  FILE *inp_fp = fopen("cells.txt", "r");
  if( inp_fp == NULL ) {
    perror("Error opening file");
    return(-1);
  }

  char line[25]; // each line has length 23 + \n + \0 character.
  int n_points = 0;
  while( fgets(line, sizeof(line), inp_fp) != NULL ){ // count points
    n_points++;
  }
  fseek( inp_fp, 0, SEEK_SET ); // return to beginning of file.

  int* coords = (int*) malloc( sizeof(int) * 3 * n_points ); // 3: x,y,z.
  char number[7]; // sign + 5 digits + \0.
  int offset = 0;
  while( fgets(line, sizeof(line), inp_fp) != NULL )
    {// read 3 numbers from line:
      for( size_t ix = 0; ix < 3; ++ix ) // empty increment
        {
          number[0] = line[8 * ix];
          number[1] = line[8 * ix + 1];
          number[2] = line[8 * ix + 2];
          number[3] = line[8 * ix + 4]; // skip '.'
          number[4] = line[8 * ix + 5];
          number[5] = line[8 * ix + 6];
          coords[3 * offset + ix] = (int) strtol(number, NULL, 10);
        }
      offset++;
    }
  
  for (size_t ix = 0; ix < 3*n_points; ix += 3){
    printf("%d %d %d\n", coords[ix], coords[ix + 1], coords[ix + 2]);
  }

  fclose( inp_fp );

  free(coords);

  return(0);
}
