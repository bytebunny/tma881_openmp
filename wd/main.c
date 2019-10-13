#include "helper.h"

size_t counting[3465] = { 0 };
int
main(int argc, char* argv[])
{
  int n_threads = 1;
  char* ptr = strchr(argv[1], 't');
  if (ptr) {
    n_threads = strtol(++ptr, NULL, 10);
  }
  omp_set_num_threads(n_threads);
  char filnam[] = "cell_e4";
  FILE* fp;
  fp = fopen(filnam, "r");
  fseek(fp, 0L, SEEK_END);
  long res = ftell(fp);
  //  fseek(fp, 0L, SEEK_SET);
  // fclose(fp);
  size_t total_lines = res / 24;
  size_t max_load_lines = 10;
  // printf("lines %ld\n",total_lines);

  // printf("\033[0;35m");
  // printf("start line: %ld\n", ix);
  // printf("\033[0m");
  // printf("\033[0;36m");
  // printf("------------\n");
  // printf("\033[0m");

  size_t blks_need;
  size_t total_lines_cnt = total_lines;
  // compute how many blocks need
  if (total_lines_cnt % max_load_lines) {
    blks_need = total_lines_cnt / max_load_lines + 1;
  } else {
    blks_need = total_lines_cnt / max_load_lines;
  }
  // compute how many lines need to load inside a block
  size_t blk_list[blks_need], blk_start[blks_need];
  for (size_t ix = 0; ix < blks_need; ix++) {
    if (total_lines_cnt >= max_load_lines) {
      blk_list[ix] = max_load_lines;
      total_lines_cnt -= max_load_lines;
    } else {
      blk_list[ix] = total_lines_cnt;
    }
    blk_start[ix] = ix * max_load_lines;
    // printf("blk start: %ld\n",blk_start[ix]);
  }

  for (size_t iblk = 0; iblk < blks_need; iblk++) {
    // size_t line_offset = now_read + iblk * max_load_lines;
    size_t curr_max_load = blk_list[iblk];
    double* block_pnts_list =
      (double*)aligned_alloc(64, sizeof(double) * curr_max_load * 3);
    double** block_pnts =
      (double**)aligned_alloc(64, sizeof(double*) * curr_max_load);
    for (size_t ix = 0; ix < curr_max_load; ix++) {
      block_pnts[ix] = block_pnts_list + ix * 3;
    }

    fseek(fp, blk_start[iblk] * 24L, SEEK_SET);
    // printf("file at %ld\n", ftell(fp));
    // printf("load lines: %ld\n", curr_max_load);
    for (size_t ixc = 0; ixc < curr_max_load; ixc++) {
      char par_line[24];
      fgets(par_line, 25, fp);
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
        // printf("%s\n", nums);
      }
    }
    cells points;
    points.len = curr_max_load;
    points.pnts = block_pnts;
    cell_distances(points);

    if (iblk > 0) {
      size_t previous_blks_nums = iblk;
      for (size_t ix = 0; ix < previous_blks_nums; ix++) {
        // printf("previous blk %ld\n",ix);
        size_t previous_blk_start = blk_start[ix];
        size_t previous_blk_lines = blk_list[ix];
        double* previous_header_list =
          (double*)aligned_alloc(64, sizeof(double) * previous_blk_lines * 3);
        double** previous_header =
          (double**)aligned_alloc(64, sizeof(double*) * previous_blk_lines);
        for (size_t ix = 0; ix < previous_blk_lines; ix++) {
          previous_header[ix] = previous_header_list + ix * 3;
        }
        fseek(fp, previous_blk_start * 24L, SEEK_SET);
        // printf("pre file at %ld\n", ftell(fp));
        // printf("pre load lines: %ld\n", previous_blk_lines);
        for (size_t ixc = 0; ixc < previous_blk_lines; ixc++) {
          char par_line[24];
          fgets(par_line, 25, fp);
          // #pragma omp critical
          //           {
          //             printf("\033[0;34m");
          //             printf("ordered thd num: %d\n", omp_get_thread_num());
          //             printf("\033[0m");
          //           }
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
            previous_header[ixc][jx] = (double)number / 1000.0;
            // printf("%s\n", nums);
          }
        }
        cells headers;
        headers.len = previous_blk_lines;
        headers.pnts = previous_header;
        cell_distance(points, headers);
        free(previous_header_list);
        free(previous_header);
      }
    }
    free(block_pnts_list);
    free(block_pnts);

    // printf("\033[0;34m");
    // printf("------------\n");
    // printf("\033[0m");
  } // block loop
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
cell_distances(cells points)
{
  extern size_t counting[];
  size_t rows = points.len;
  double** cells_loc = points.pnts;
#pragma omp taskloop reduction(+ : counting[:3465])
  for (size_t ix = 0; ix < rows - 1; ix++) {
    double header[3];
    header[0] = cells_loc[ix][0];
    header[1] = cells_loc[ix][1];
    header[2] = cells_loc[ix][2];
    for (size_t jx = ix + 1; jx < rows; jx++) {
      double total_len_2 =
        (header[0] - cells_loc[jx][0]) * (header[0] - cells_loc[jx][0]) +
        (header[1] - cells_loc[jx][1]) * (header[1] - cells_loc[jx][1]) +
        (header[2] - cells_loc[jx][2]) * (header[2] - cells_loc[jx][2]);

      size_t total_len_rnd = (size_t)(sqrt(total_len_2) * 100.0 + 0.5);
      counting[total_len_rnd] += 1;
    }
  }
}
void
cell_distance(cells points, cells headers)
{
  extern size_t counting[];
  size_t rows = points.len;
  double** cells_loc = points.pnts;
  size_t header_rows = headers.len;
  double** header_loc = headers.pnts;

#pragma omp taskloop collapse(2) reduction(+ : counting[:3465])
  for (size_t ix = 0; ix < header_rows; ix++) {
    for (size_t jx = 0; jx < rows; jx++) {
      double total_len_2 = (header_loc[ix][0] - cells_loc[jx][0]) *
                             (header_loc[ix][0] - cells_loc[jx][0]) +
                           (header_loc[ix][1] - cells_loc[jx][1]) *
                             (header_loc[ix][1] - cells_loc[jx][1]) +
                           (header_loc[ix][2] - cells_loc[jx][2]) *
                             (header_loc[ix][2] - cells_loc[jx][2]);

      size_t total_len_rnd = (size_t)(sqrt(total_len_2) * 100.0 + 0.5);
      counting[total_len_rnd] += 1;
    }
  }
}