#include "helper.h"

int
main(int argc, char* argv[])
{
  size_t counting[3465] = { 0 };
  char* ptr = strchr(argv[1], 't');
  int n_threads = 1;
  if (ptr) {
    n_threads = strtol(++ptr, NULL, 10);
  }
  omp_set_num_threads(n_threads);
  FILE* fp;
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
  // size_t done_end_blk;

  //[x] [] [] [] []
  for (size_t ix = 0; ix < total_lines - 1; ix++) {
    // printf(">>task load starter start, thds: %d\n", omp_get_thread_num());
    char lines[24];
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
        // printf("%s\n", nums);
      }
      current_pnt = ix + 1;
      // done_end_blk = 0;
    }

    printf("\033[0;35m");
    printf("start line: %ld\n", current_pnt - 1);
    printf("\033[0m");
    printf("\033[0;36m");
    printf("------------\n");
    printf("\033[0m");
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
      size_t ixb = current_pnt + iblk * max_load_lines;
#pragma omp parallel default(none) firstprivate(ixb)                           \
  shared(max_load_lines, header_pnt, fp, counting)
#pragma omp for reduction(+ : counting[:3465])
      for (size_t ixc = 0; ixc < max_load_lines; ixc++) {
        // printf("task load ender start, thds: %d\n", omp_get_thread_num());
        char par_line[24];
        fseek(fp, (ixb + ixc) * 24 * sizeof(char), SEEK_SET);
        if (fread(par_line, 1, 24, fp) == 24) {
          printf("\033[0;32m");
          printf("reading line: %ld\n", ixb + ixc);
          printf("\033[0m");
          double end_pnt[3];
          for (size_t jx = 0; jx < 3; jx++) {
            char nums[7];
            nums[0] = par_line[8 * jx + 0];
            nums[1] = par_line[8 * jx + 1];
            nums[2] = par_line[8 * jx + 2];
            nums[3] = par_line[8 * jx + 4];
            nums[4] = par_line[8 * jx + 5];
            nums[5] = par_line[8 * jx + 6];
            nums[6] = '\0';
            int number = naive_str2l(nums);
            end_pnt[jx] = (double)number / 1000.0;
            //#pragma omp criticle
            //            printf("%s\n", nums);
            // printf("%4.4f\n", end_pnt[jx]);
          }

          double total_len_2 =
            (header_pnt[0] - end_pnt[0]) * (header_pnt[0] - end_pnt[0]) +
            (header_pnt[1] - end_pnt[1]) * (header_pnt[1] - end_pnt[1]) +
            (header_pnt[2] - end_pnt[2]) * (header_pnt[2] - end_pnt[2]);

          size_t total_len_rnd = (size_t)(sqrt(total_len_2) * 100.0 + 0.5);
          counting[total_len_rnd] += 1;
          // printf("counting: %ld\n", counting[total_len_rnd]);
        }
        //#pragma omp criticle
        //        printf("done_end_blk: %ld\n", done_end_blk);
        //#pragma omp criticle
        //        printf("<<task load ender end, thds: %d\n",
        //        omp_get_thread_num());
      }
      printf("\033[0;36m");
      printf("------------\n");
      printf("\033[0m");
      // done_end_blk++;
    }
    printf("\n");
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
