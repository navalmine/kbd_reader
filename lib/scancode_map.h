#ifndef SCANCODE_MAP_H
#define SCANCODE_MAP_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  unsigned int shift : 1;
  unsigned int ctrl : 1;
  unsigned int alt : 1;
  unsigned int caps : 1;
} scancode_state_t;

/*
 * Initializes the scancode parser state.
 */
void scancode_state_init(scancode_state_t *state);

/*
 * Processes a set-1 PC/AT scancode, updating modifier state and optionally
 * emitting output into `out`. Returns the number of bytes written to `out`.
 * `counted_out` is incremented for printable output (not backspace/tokens).
 */
size_t scancode_process(scancode_state_t *state,
                        uint8_t scancode,
                        char *out,
                        size_t out_size,
                        unsigned long *counted_out);

#ifdef __cplusplus
}
#endif

#endif
