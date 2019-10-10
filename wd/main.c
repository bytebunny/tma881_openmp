#include "helper.h"

size_t counting[3465] = { 0 };

int
main(int argc, char* argv[])
{
  char* ptr = strchr(argv[1], 't');
  int n_threads = 1;
  if (ptr) {
    n_threads = strtol(++ptr, NULL, 10);
  }
  omp_set_num_threads(n_threads);
  FILE* fp;
  char lines[25];
  fp = fopen("cell_e5", "r");
  fseek(fp, 0L, SEEK_END);
  long res = ftell(fp);
  fseek(fp, 0L, SEEK_SET);
  size_t total_lines = res / 24;
  // printf("lines %ld\n",total_lines);
  double* cells = (double*)aligned_alloc(64, sizeof(double) * total_lines * 3);
  double** cellrows =
    (double**)aligned_alloc(64, sizeof(double*) * total_lines);
  for (size_t ix = 0; ix < total_lines; ix++) {
    cellrows[ix] = cells + ix * 3;
  }

  for (size_t ix = 0; ix < total_lines; ix++) {
    if (fgets(lines, sizeof(lines), fp) != NULL) {
      // puts(lines);
      for (size_t jx = 0; jx < 3; jx++) {
        char nums[6];
        nums[0] = lines[8 * jx + 0];
        nums[1] = lines[8 * jx + 1];
        nums[2] = lines[8 * jx + 2];
        nums[3] = lines[8 * jx + 4];
        nums[4] = lines[8 * jx + 5];
        nums[5] = lines[8 * jx + 6];
        int number = naive_str2l(nums);
        cellrows[ix][jx] = (double)number / 1000.0;
        // printf("%4.4f\n", cells[ix * 3 + jx]);
      }
    }
  }
  fclose(fp);
  cell points;
  points.len = total_lines * 3;
  points.pnts = cellrows;
  cell_distances(points);
  free(cells);
  free(cellrows);
}

int
naive_str2l(const char* p)
{
  int x = 0;
  int neg = 0;
  if (*p == '-') {
    neg = 1;
  }
  ++p;
  while (*p >= '0' && *p <= '9') {
    x = (x * 10) + (*p - '0');
    ++p;
  }
  if (neg) {
    x = -x;
  }
  return x;
}

void
cell_distances(cell points)
{
  extern size_t counting[];
  size_t rows = points.len / 3;
  double** cells_loc = points.pnts;
#pragma omp parallel for reduction(+ : counting[:3465])
  for (size_t ix = 0; ix < rows; ix++) {
    // printf("thread %d\n", omp_get_thread_num());
    double start_pnt[3];
    start_pnt[0] = cells_loc[ix][0];
    start_pnt[1] = cells_loc[ix][1];
    start_pnt[2] = cells_loc[ix][2];
    for (size_t jx = ix + 1; jx < rows; jx++) {
      double end_pnt[3];
      end_pnt[0] = cells_loc[jx][0];
      end_pnt[1] = cells_loc[jx][1];
      end_pnt[2] = cells_loc[jx][2];

      double total_len_2 =
        (start_pnt[0] - end_pnt[0]) * (start_pnt[0] - end_pnt[0]) +
        (start_pnt[1] - end_pnt[1]) * (start_pnt[1] - end_pnt[1]) +
        (start_pnt[2] - end_pnt[2]) * (start_pnt[2] - end_pnt[2]);

      size_t total_len_rnd = (size_t)(sqrt(total_len_2) * 100.0 + 0.5);
      counting[total_len_rnd] += 1;
      // total_len = (double)(total_len_rnd / 100);

      // printf(
      //  "start: %4.3f %4.3f %4.3f\n", start_pnt[0], start_pnt[1],
      //  start_pnt[2]);
      // printf("end: %4.3f %4.3f %4.3f\n", end_pnt[0], end_pnt[1], end_pnt[2]);
      // printf("distance: %4.3f\n", total_len);
    }
  }
  for (size_t ix = 0; ix < 3465; ix++) {
    if (counting[ix] > 0) {
      fprintf(stdout, "%4.2f %ld\n", (double)(ix) / 100.0, counting[ix]);
    }
  }
}
