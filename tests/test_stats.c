#include "stats.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static int write_seed(const char *path) {
  FILE *fp = fopen(path, "w");
  if (!fp) {
    return 1;
  }
  fputs("total=3\n", fp);
  fputs("2025-01-07=2\n", fp);
  fputs("2025-01-06=1\n", fp);
  fclose(fp);
  return 0;
}

int main(void) {
  char tmpl[] = "/tmp/kbdstatsXXXXXX";
  int fd = mkstemp(tmpl);
  if (fd < 0) {
    return 1;
  }
  close(fd);

  if (write_seed(tmpl) != 0) {
    unlink(tmpl);
    return 1;
  }

  stats_t stats;
  if (stats_init(&stats, tmpl, "2025-01-07") != 0) {
    unlink(tmpl);
    return 1;
  }

  if (stats.total != 3 || stats.day_count != 2) {
    stats_free(&stats);
    unlink(tmpl);
    return 1;
  }

  stats_record(&stats, 4);
  if (stats.total != 7 || stats.day_count != 6) {
    stats_free(&stats);
    unlink(tmpl);
    return 1;
  }

  if (stats_save(&stats) != 0) {
    stats_free(&stats);
    unlink(tmpl);
    return 1;
  }

  stats_free(&stats);

  stats_t stats2;
  if (stats_init(&stats2, tmpl, "2025-01-07") != 0) {
    unlink(tmpl);
    return 1;
  }

  if (stats2.total != 7 || stats2.day_count != 6) {
    stats_free(&stats2);
    unlink(tmpl);
    return 1;
  }

  stats_free(&stats2);
  unlink(tmpl);
  return 0;
}
