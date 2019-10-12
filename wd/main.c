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
  char filnam[] = "cell";
  FILE* fp;
  fp = fopen(filnam, "r");
  fseek(fp, 0L, SEEK_END);
  long res = ftell(fp);
  fseek(fp, 0L, SEEK_SET);
  // fclose(fp);
  size_t total_lines = res / 24;
  size_t max_load_lines = 100;
  // printf("lines %ld\n",total_lines);

  //[x] [] [] [] []
  for (size_t ix = 0; ix < total_lines - 1; ix++) {
    // printf(">>task load starter start, thds: %d\n", omp_get_thread_num());
    double header_pnt[3];
    char lines[24];
    fseek(fp, ix * 24 * sizeof(char), SEEK_SET);
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
        header_pnt[jx] = (double)number / 1000.0;
        // printf("%s\n", nums);
      }
      // done_end_blk = 0;
    }

    printf("\033[0;35m");
    printf("start line: %ld\n", ix);
    printf("\033[0m");
    printf("\033[0;36m");
    printf("------------\n");
    printf("\033[0m");

    // printf("<<task load starter end, thds: %d\n\n", omp_get_thread_num());
    // compute how many blocks need
    size_t now_read = ix + 1;
    size_t blks_need;
    size_t left_lines = total_lines - now_read;
    if (left_lines % max_load_lines) {
      blks_need = left_lines / max_load_lines + 1;
    } else {
      blks_need = left_lines / max_load_lines;
    }
    size_t blk_list[blks_need], remain_left_lines = left_lines;
    for (size_t ix = 0; ix < blks_need; ix++) {
      if (remain_left_lines >= max_load_lines) {
        blk_list[ix] = max_load_lines;
        remain_left_lines -= max_load_lines;
      } else {
        blk_list[ix] = remain_left_lines;
      }
    }

    // printf("need blocks: %ld\n", blks_need);
    for (size_t iblk = 0; iblk < blks_need; iblk++) {
      size_t line_offset = now_read + iblk * max_load_lines;
      size_t curr_max_load = blk_list[iblk];
      double* block_pnts_list =
        (double*)aligned_alloc(64, sizeof(double) * curr_max_load * 3);
      double** block_pnts =
        (double**)aligned_alloc(64, sizeof(double*) * total_lines);
      for (size_t ix = 0; ix < total_lines; ix++) {
        block_pnts[ix] = block_pnts_list + ix * 3;
      }

      for (size_t ixc = 0; ixc < curr_max_load; ixc++) {
        // printf("task load ender start, thds: %d\n", omp_get_thread_num());
        char par_line[24];
        size_t read_line_num = line_offset + ixc;
        // FILE* fl = fopen(filnam, "r");
        fseek(fp, read_line_num * 24 * sizeof(char), SEEK_SET);
        fread(par_line, 1, 24, fp);
        // if (fread(par_line, 1, 24, fp) == 24) {
        printf("\033[0;32m");
        printf(
          "reading line: %ld, thd: %d\n", read_line_num, omp_get_thread_num());
        printf("\033[0m");
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
          block_pnts[ixc][jx] = (double)number / 1000.0;
          //#pragma omp critical
          //            printf("%s\n", nums);
          // printf("%4.4f\n", end_pnt[jx]);
        }
        //}
        // fclose(fl);
      }
      cell points;
      points.len = curr_max_load;
      points.pnts = block_pnts;
      cell_distances(points, header_pnt);
      free(block_pnts_list);
      free(block_pnts);
      printf("\033[0;34m");
      printf("------------\n");
      printf("\033[0m");
      // done_end_blk++;
    } // block loop
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

void
cell_distances(cell points, double header[3])
{
  extern size_t counting[];
  size_t rows = points.len;
  double** cells_loc = points.pnts;
#pragma omp parallel for reduction(+ : counting[:3465])
  for (size_t jx = 0; jx < rows; jx++) {
    double end_pnt[3];
    end_pnt[0] = cells_loc[jx][0];
    end_pnt[1] = cells_loc[jx][1];
    end_pnt[2] = cells_loc[jx][2];

    double total_len_2 = (header[0] - end_pnt[0]) * (header[0] - end_pnt[0]) +
                         (header[1] - end_pnt[1]) * (header[1] - end_pnt[1]) +
                         (header[2] - end_pnt[2]) * (header[2] - end_pnt[2]);

    size_t total_len_rnd = (size_t)(sqrt(total_len_2) * 100.0 + 0.5);
    counting[total_len_rnd] += 1;
  }
}