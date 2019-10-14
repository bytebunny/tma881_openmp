#include "omp.h"
#include <math.h>
#include <pthread.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifndef _H_NEWTON // once-only header
#define _H_NEWTON
typedef struct
{
  int** pnts;
  size_t len;
} cells;

int
naive_str2l(const char*);

void cell_distances(cells);
void cell_distance(cells, int []);

#endif