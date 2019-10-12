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
  char lines[24];
  fp = fopen("cell", "r");
  fseek(fp, 0L, SEEK_END);
  long res = ftell(fp);
  fseek(fp, 0L, SEEK_SET);
  size_t total_lines = res / 24;
  size_t max_load_lines = 2;
  // size_t slice_num = total_lines / max_load_lines; // now i have 5 slices
  // printf("lines %ld\n",total_lines);

  double header_pnt[3];
  size_t current_pnt = 0;
  size_t done_end_blk;

  //[x] [] [] [] []
  for (size_t ix = 0; ix < total_lines; ix++) {
    // printf(">>task load starter start, thds: %d\n", omp_get_thread_num());

    fseek(fp, ix * 24 * sizeof(char), SEEK_SET);
    if (fread(lines, sizeof(char), sizeof(lines), fp) == sizeof(lines)) {
      // puts(lines);
      for (size_t jx = 0; jx < 3; jx++) {
        char nums[7];
        nums[0] = lines[8 * jx + 0];
        nums[1] = lines[8 * jx + 1];
        nums[2] = lines[8 * jx + 2];
        nums[3] = lines[8 * jx + 4];
        nums[4] = lines[8 * jx + 5];
        nums[5] = lines[8 * jx + 6];
        nums[6] = '\0';
        int number = naive_str2l(nums);
        header_pnt[jx] = (double)number / 1000.0;
        // printf("%4.4f\n", header_pnt[jx]);
      }
      current_pnt = ix + 1;
      done_end_blk = 0;
    }

    // printf("current start point: %ld\n", current_pnt);

    // printf("<<task load starter end, thds: %d\n\n", omp_get_thread_num());
    // compute how many blocks need
    size_t blks_need;
    size_t left_lines = total_lines - current_pnt;
    if (left_lines % max_load_lines) {
      blks_need = left_lines / max_load_lines + 1;
    } else {
      blks_need = left_lines / max_load_lines;
    }
    // printf("need blocks: %ld\n", blks_need);
// #pragma omp parallel
// #pragma omp single
// #pragma omp taskloop grainsize(2)
    for (size_t iblk = 0; iblk < blks_need; iblk++) {
      //#pragma omp criticle
      printf(">>task load ender start, thds: %d\n", omp_get_thread_num());
      size_t ixb;
#pragma omp taskloop private(ixb) reduction(+ : counting[:3465])
      for (size_t ixc = 0; ixc < max_load_lines; ixc++) {
        ixb = current_pnt + iblk * max_load_lines + ixc;
        double end_pnt[3] = { 0.0 };
        fseek(fp, ixb * 24 * sizeof(char), SEEK_SET);
        if (fread(lines, sizeof(char), sizeof(lines), fp) == sizeof(lines)) {
          for (size_t jx = 0; jx < 3; jx++) {
            char nums[7];
            nums[0] = lines[8 * jx + 0];
            nums[1] = lines[8 * jx + 1];
            nums[2] = lines[8 * jx + 2];
            nums[3] = lines[8 * jx + 4];
            nums[4] = lines[8 * jx + 5];
            nums[5] = lines[8 * jx + 6];
            nums[6] = '\0';
            int number = naive_str2l(nums);
            end_pnt[jx] = (double)number / 1000.0;

            // printf("%s\n", nums);
            // printf("%4.4f\n", end_pnt[jx]);
          }
          double start_pnt[3];
          start_pnt[0] = header_pnt[0];
          start_pnt[1] = header_pnt[1];
          start_pnt[2] = header_pnt[2];

          double total_len_2 =
            (start_pnt[0] - end_pnt[0]) * (start_pnt[0] - end_pnt[0]) +
            (start_pnt[1] - end_pnt[1]) * (start_pnt[1] - end_pnt[1]) +
            (start_pnt[2] - end_pnt[2]) * (start_pnt[2] - end_pnt[2]);

          size_t total_len_rnd = (size_t)(sqrt(total_len_2) * 100.0 + 0.5);
          counting[total_len_rnd] += 1;
        }
      }
      done_end_blk++;
      // #pragma omp criticle
      //       printf("done_end_blk: %ld\n", done_end_blk);
      // #pragma omp criticle
      //       printf("<<task load ender end, thds: %d\n\n",
      //       omp_get_thread_num());
    }
  } // end of ix loop
  // load target
  fclose(fp);

  for (size_t ixb = 0; ixb < 3465; ixb++) {
    if (counting[ixb] > 0) {
      fprintf(stdout, "%4.2f %ld\n", (double)(ixb) / 100.0, counting[ixb]);
    }
  }
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
