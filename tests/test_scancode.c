#include "scancode_map.h"

#include <stdio.h>
#include <string.h>

static int feed(scancode_state_t *state,
                unsigned char sc,
                const char *expected,
                unsigned long expected_count) {
  char out[32];
  unsigned long counted = 0;
  size_t len = scancode_process(state, sc, out, sizeof(out), &counted);
  if (expected == NULL) {
    if (len != 0 || counted != 0) {
      fprintf(stderr, "scancode 0x%02X expected no output\n", sc);
      return 1;
    }
    return 0;
  }
  if (strcmp(out, expected) != 0) {
    fprintf(stderr, "scancode 0x%02X expected \"%s\" got \"%s\"\n", sc, expected, out);
    return 1;
  }
  if (counted != expected_count) {
    fprintf(stderr, "scancode 0x%02X expected count %lu got %lu\n",
            sc, expected_count, counted);
    return 1;
  }
  return 0;
}

int main(void) {
  int failures = 0;
  scancode_state_t state;
  scancode_state_init(&state);

  failures += feed(&state, 0x1E, "a", 1);
  failures += feed(&state, 0x02, "1", 1);
  failures += feed(&state, 0x39, " ", 1);
  failures += feed(&state, 0x2A, "<SHIFT>", 0);
  failures += feed(&state, 0x1E, "A", 1);
  failures += feed(&state, 0xAA, NULL, 0);
  failures += feed(&state, 0x2A, "<SHIFT>", 0);
  failures += feed(&state, 0x02, "!", 1);
  failures += feed(&state, 0xAA, NULL, 0);
  failures += feed(&state, 0x3A, "<CAPS_ON>", 0);
  failures += feed(&state, 0x1E, "A", 1);
  failures += feed(&state, 0x3A, "<CAPS_OFF>", 0);
  failures += feed(&state, 0x1E, "a", 1);
  failures += feed(&state, 0x1D, "<CTRL>", 0);
  failures += feed(&state, 0x9D, NULL, 0);
  failures += feed(&state, 0x01, "<ESC>", 0);
  failures += feed(&state, 0x0E, "\b", 0);
  failures += feed(&state, 0xFF, NULL, 0);

  return failures == 0 ? 0 : 1;
}
