#include "stats.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int parse_line(const char *line, char *key, size_t key_size, unsigned long *value) {
  const char *eq = strchr(line, '=');
  if (!eq) {
    return 0;
  }

  size_t key_len = (size_t)(eq - line);
  if (key_len == 0 || key_len >= key_size) {
    return 0;
  }

  memcpy(key, line, key_len);
  key[key_len] = '\0';

  const char *val_str = eq + 1;
  char *endptr = NULL;
  unsigned long val = strtoul(val_str, &endptr, 10);
  if (endptr == val_str) {
    return 0;
  }

  *value = val;
  return 1;
}

int stats_init(stats_t *stats, const char *path, const char *day) {
  if (!stats || !path || !day || strlen(day) != 10) {
    return -1;
  }

  memset(stats, 0, sizeof(*stats));
  memcpy(stats->day, day, 10);
  stats->day[10] = '\0';
  stats->path = strdup(path);
  if (!stats->path) {
    return -1;
  }

  FILE *fp = fopen(path, "r");
  if (!fp) {
    if (errno == ENOENT) {
      return 0;
    }
    return -1;
  }

  char line[256];
  while (fgets(line, sizeof(line), fp)) {
    char key[32];
    unsigned long value = 0;
    if (!parse_line(line, key, sizeof(key), &value)) {
      continue;
    }

    if (strcmp(key, "total") == 0) {
      stats->total = value;
    } else if (strcmp(key, stats->day) == 0) {
      stats->day_count = value;
    }
  }

  fclose(fp);
  return 0;
}

void stats_record(stats_t *stats, unsigned long count) {
  if (!stats) {
    return;
  }
  stats->total += count;
  stats->day_count += count;
}

int stats_save(stats_t *stats) {
  if (!stats || !stats->path) {
    return -1;
  }

  FILE *fp = fopen(stats->path, "r");
  char **other_lines = NULL;
  size_t other_count = 0;

  if (fp) {
    char line[256];
    while (fgets(line, sizeof(line), fp)) {
      char key[32];
      unsigned long value = 0;
      if (parse_line(line, key, sizeof(key), &value)) {
        if (strcmp(key, "total") == 0 || strcmp(key, stats->day) == 0) {
          continue;
        }
      }

      char *copy = strdup(line);
      if (!copy) {
        fclose(fp);
        return -1;
      }
      char **tmp = realloc(other_lines, sizeof(char *) * (other_count + 1));
      if (!tmp) {
        free(copy);
        fclose(fp);
        return -1;
      }
      other_lines = tmp;
      other_lines[other_count++] = copy;
    }
    fclose(fp);
  }

  fp = fopen(stats->path, "w");
  if (!fp) {
    for (size_t i = 0; i < other_count; ++i) {
      free(other_lines[i]);
    }
    free(other_lines);
    return -1;
  }

  fprintf(fp, "total=%lu\n", stats->total);
  fprintf(fp, "%s=%lu\n", stats->day, stats->day_count);
  for (size_t i = 0; i < other_count; ++i) {
    fputs(other_lines[i], fp);
    free(other_lines[i]);
  }
  free(other_lines);
  fclose(fp);
  return 0;
}

void stats_free(stats_t *stats) {
  if (!stats) {
    return;
  }
  free(stats->path);
  stats->path = NULL;
}
