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
  size_t slice_num = total_lines / max_load_lines; // now i have 5 slices
  // printf("lines %ld\n",total_lines);

  double header_pnt[3];
  size_t startrdy = 0;
  size_t offset;

//  for (size_t ib = 0; ib < slice_num; ib++) {
//[x] [] [] [] []
#pragma omp parallel
#pragma omp single
  {
    for (size_t ix = 0; ix < total_lines; ix++) {
#pragma omp task shared(offset) depend(out : startrdy)
      { // read the start point
#pragma omp critical
        printf(">>task load starter start, thds: %d\n", omp_get_thread_num());
        size_t curr_line_num = ix;
        fseek(fp, curr_line_num * 24 * sizeof(char), SEEK_SET);
        if (fread(lines, sizeof(char), sizeof(lines), fp) == sizeof(lines)) {
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
            header_pnt[jx] = (double)number / 1000.0;
            printf("%4.4f\n", header_pnt[jx]);
          }
        }
        startrdy = ix;
        offset = 0;
#pragma omp critical
        {
          printf("current start point: %ld\n", startrdy);
        }

#pragma omp critical
        printf("<<task load starter end, thds: %d\n\n", omp_get_thread_num());
      } // task start point read done
#pragma omp task shared(header_pnt, startrdy, offset) depend(in                \
                                                             : startrdy)       \
  depend(inout                                                                 \
         : offset)
      {
#pragma omp critical
        printf(">>task load ender start, thds: %d\n", omp_get_thread_num());
        size_t local_lines = 0;
        double end_pnt[3] = { 0.0 };
        for (size_t ixb = startrdy + 1 + offset * max_load_lines, jxb = 0;
             ixb < total_lines && jxb < max_load_lines;
             ixb++, jxb++) {
#pragma omp critical
          printf("offset: %ld\n", offset);
          fseek(fp, ixb * 24 * sizeof(char), SEEK_SET);
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
              end_pnt[jx] = (double)number / 1000.0;

              printf("%s\n", nums);
              printf("%4.4f\n", end_pnt[jx]);
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

            offset++;
            jxb = 0;
            local_lines++;
          }
#pragma omp critical
          printf("<<task load ender end, thds: %d\n\n", omp_get_thread_num());
        }
      }
    } // end of ix loop
  }   // single
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
