#ifndef STATS_H
#define STATS_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  char day[11];
  char *path;
  unsigned long total;
  unsigned long day_count;
} stats_t;

int stats_init(stats_t *stats, const char *path, const char *day);
void stats_record(stats_t *stats, unsigned long count);
int stats_save(stats_t *stats);
void stats_free(stats_t *stats);

#ifdef __cplusplus
}
#endif

#endif
