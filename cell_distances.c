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
  char filnam[] = "cells";
  FILE* fp;
  fp = fopen(filnam, "r");
  fseek(fp, 0L, SEEK_END);
  long res = ftell(fp);
  size_t total_lines = res / 24;
  size_t max_load_lines = 80000000;
  size_t blks_need;
  size_t total_lines_cnt = total_lines;
  // compute how many blocks are needed
  if (total_lines_cnt % max_load_lines) {
    blks_need = total_lines_cnt / max_load_lines + 1;
  } else {
    blks_need = total_lines_cnt / max_load_lines;
  }
  // compute how many lines need to be loaded inside a block
  size_t blk_list[blks_need], blk_start[blks_need];
  for (size_t ix = 0; ix < blks_need; ix++) {
    if (total_lines_cnt >= max_load_lines) {
      blk_list[ix] = max_load_lines;
      total_lines_cnt -= max_load_lines;
    } else {
      blk_list[ix] = total_lines_cnt;
    }
    blk_start[ix] = ix * max_load_lines;
  }

  for (size_t iblk = 0; iblk < blks_need; iblk++) {
    size_t curr_max_load = blk_list[iblk];
    short* block_pnts_list =
      (short*)aligned_alloc(64, sizeof(short) * curr_max_load * 3);
    short** block_pnts = (short**)aligned_alloc(64, sizeof(short*) * curr_max_load);
    for (size_t ix = 0; ix < curr_max_load; ix++) {
      block_pnts[ix] = block_pnts_list + ix * 3;
    }

    fseek(fp, blk_start[iblk] * 24L, SEEK_SET);
    for (size_t ixc = 0; ixc < curr_max_load; ixc++) {
      char par_line[25];
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
        block_pnts[ixc][jx] = number;
      }
    }
    cells points;
    points.len = curr_max_load;
    points.pnts = block_pnts;
    cell_distances(points);

    if (iblk > 0) {
      size_t previous_blks_nums = iblk;
      for (size_t ix = 0; ix < previous_blks_nums; ix++) {
        size_t previous_blk_start = blk_start[ix];
        size_t previous_blk_lines = blk_list[ix];
        short previous_header[3];
        fseek(fp, previous_blk_start * 24L, SEEK_SET);
        for (size_t ixc = 0; ixc < previous_blk_lines; ixc++) {
          char par_line[25];
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
            short number = naive_str2l(nums);
            previous_header[jx] = number;
          }
          cell_distance(points, previous_header);
        }
      }
    }
    free(block_pnts_list);
    free(block_pnts);

  } // block loop
  fclose(fp);
  for (size_t ixb = 0; ixb < 3465; ixb++) {
    printf("%05.2f %ld\n", (double)(ixb) / 100.0, counting[ixb]);
  }
}

short
naive_str2l(const char* p)
{
  short x = 0;
  short neg = 0;
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
  short** cells_loc = points.pnts;
  
  size_t ix, jx;
  short header[3], total_len_rnd;
  int total_len_2;
  #pragma omp parallel for \
    default(none) private(ix, jx, header, total_len_2, total_len_rnd) \
    shared(rows, cells_loc) reduction(+:counting[:3465])
  for ( ix = 0; ix < rows - 1; ix++) {
    header[0] = cells_loc[ix][0];
    header[1] = cells_loc[ix][1];
    header[2] = cells_loc[ix][2];
    for ( jx = ix + 1; jx < rows; jx++) {
      total_len_2 =
        (header[0] - cells_loc[jx][0]) * (header[0] - cells_loc[jx][0]) +
        (header[1] - cells_loc[jx][1]) * (header[1] - cells_loc[jx][1]) +
        (header[2] - cells_loc[jx][2]) * (header[2] - cells_loc[jx][2]);

      total_len_rnd = (short)(sqrtf(total_len_2) / 10 + 0.5);
      counting[total_len_rnd] += 1;
    }
  }
}
void
cell_distance(cells points, short header[3])
{
  extern size_t counting[];
  size_t rows = points.len;
  short** cells_loc = points.pnts;

  size_t jx;
  int total_len_2;
  short total_len_rnd;
  #pragma omp parallel for \
    default(none) private(jx, total_len_2, total_len_rnd) \
    shared(rows, cells_loc, header) reduction(+:counting[:3465])
  for ( jx = 0; jx < rows; jx++) {
    total_len_2 =
      (header[0] - cells_loc[jx][0]) * (header[0] - cells_loc[jx][0]) +
      (header[1] - cells_loc[jx][1]) * (header[1] - cells_loc[jx][1]) +
      (header[2] - cells_loc[jx][2]) * (header[2] - cells_loc[jx][2]);

    total_len_rnd = (short)(sqrtf(total_len_2) / 10 + 0.5);
    counting[total_len_rnd] += 1;
  }
}
