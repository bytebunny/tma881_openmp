#include <stdio.h> // printing to screan
#include <string.h>
#include <stdlib.h> // memory allocation
#include <math.h> // sqrtf().
#include <omp.h>

int n_threads;
const size_t MAX_DISTS = 3465; // max number of unique distances on interval [-10; 10] with 2 decimals precision.

int computeDistance(const int* p1, const int* p2)
{
  float distance=sqrtf( (*p2 - *p1)*(*p2 - *p1) + 
                        (*(p2+1)-*(p1+1)) * (*(p2+1)-*(p1+1)) + 
                        (*(p2+2)-*(p1+2)) * (*(p2+2)-*(p1+2)) );
  // Divide distance by 10 to keep 5-digit precision.
  return (int) (distance/10 + 0.5); // round(distance*0.1) is too expensive.
}

int main(int argc, char *argv[])
{
  char *ptr; // pointer to parse input data.

  if ( argc == 2 ) {
    ptr = strchr( argv[1], 't'); // starts with 1 because 0 is the program name.
    n_threads = strtol(++ptr, NULL, 10);
    omp_set_num_threads( n_threads ); // overrides previous value
  }
  else {
    printf("Missing arguments! Correct syntax is: cell_distances -t#numberOfThreads# \n");
    exit(0);
  }

  /////////////////////////// Read input data from file ////////////////////////
  FILE *inp_fp = fopen("cells", "r");
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
      for( size_t ix = 0; ix < 3; ++ix )
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

  fclose( inp_fp );

  ////////////////////////////// Compute the distances /////////////////////////
  int* freqArray = (int*) calloc( MAX_DISTS, sizeof(int) );
  size_t ix, jx;
  int dist;
  #pragma omp parallel for  \
    default(none) private(ix, jx, dist) shared(coords, n_points) reduction(+:freqArray[:MAX_DISTS])
  for ( ix = 0; ix < 3*n_points; ix += 3 ) {
    for ( jx = ix+3; jx < 3*n_points; jx += 3) {
      if (jx >= ix + 3) {
        {
          dist = computeDistance( &coords[ix], &coords[jx]);
        }
        ++freqArray[dist];
      }
    }
  }

  //print the results
  for ( size_t ix = 0; ix < MAX_DISTS; ++ix ) {
//    if ( freqArray[ix] ) {
      printf("%05.2f %d \n", (float)(ix)/100, freqArray[ix]);
//    }
  }

  free(freqArray);
  free(coords);

  return(0);
}
