#include "scancode_map.h"

#include <string.h>

static const char normal_map[128] = {
    [0x01] = 0,
    [0x02] = '1',
    [0x03] = '2',
    [0x04] = '3',
    [0x05] = '4',
    [0x06] = '5',
    [0x07] = '6',
    [0x08] = '7',
    [0x09] = '8',
    [0x0A] = '9',
    [0x0B] = '0',
    [0x0C] = '-',
    [0x0D] = '=',
    [0x0E] = '\b',
    [0x0F] = '\t',
    [0x10] = 'q',
    [0x11] = 'w',
    [0x12] = 'e',
    [0x13] = 'r',
    [0x14] = 't',
    [0x15] = 'y',
    [0x16] = 'u',
    [0x17] = 'i',
    [0x18] = 'o',
    [0x19] = 'p',
    [0x1A] = '[',
    [0x1B] = ']',
    [0x1C] = '\n',
    [0x1E] = 'a',
    [0x1F] = 's',
    [0x20] = 'd',
    [0x21] = 'f',
    [0x22] = 'g',
    [0x23] = 'h',
    [0x24] = 'j',
    [0x25] = 'k',
    [0x26] = 'l',
    [0x27] = ';',
    [0x28] = '\'',
    [0x29] = '`',
    [0x2B] = '\\',
    [0x2C] = 'z',
    [0x2D] = 'x',
    [0x2E] = 'c',
    [0x2F] = 'v',
    [0x30] = 'b',
    [0x31] = 'n',
    [0x32] = 'm',
    [0x33] = ',',
    [0x34] = '.',
    [0x35] = '/',
    [0x39] = ' ',
};

static const char shifted_map[128] = {
    [0x02] = '!',
    [0x03] = '@',
    [0x04] = '#',
    [0x05] = '$',
    [0x06] = '%',
    [0x07] = '^',
    [0x08] = '&',
    [0x09] = '*',
    [0x0A] = '(',
    [0x0B] = ')',
    [0x0C] = '_',
    [0x0D] = '+',
    [0x1A] = '{',
    [0x1B] = '}',
    [0x27] = ':',
    [0x28] = '\"',
    [0x29] = '~',
    [0x2B] = '|',
    [0x33] = '<',
    [0x34] = '>',
    [0x35] = '?',
};

void scancode_state_init(scancode_state_t *state) {
  if (!state) {
    return;
  }
  memset(state, 0, sizeof(*state));
}

static int write_token(const char *token, char *out, size_t out_size, unsigned long *counted_out) {
  if (!out || out_size == 0) {
    return 0;
  }
  size_t len = strlen(token);
  if (len >= out_size) {
    len = out_size - 1;
  }
  memcpy(out, token, len);
  out[len] = '\0';
  if (counted_out) {
    *counted_out = 0;
  }
  return (int)len;
}

static int is_letter(char ch) {
  return ch >= 'a' && ch <= 'z';
}

size_t scancode_process(scancode_state_t *state,
                        uint8_t scancode,
                        char *out,
                        size_t out_size,
                        unsigned long *counted_out) {
  if (!state || !out || out_size == 0) {
    if (counted_out) {
      *counted_out = 0;
    }
    return 0;
  }

  if (counted_out) {
    *counted_out = 0;
  }

  int release = (scancode & 0x80) != 0;
  uint8_t code = scancode & 0x7F;

  switch (code) {
    case 0x2A:
    case 0x36:
      state->shift = release ? 0 : 1;
      if (!release) {
        return (size_t)write_token("<SHIFT>", out, out_size, counted_out);
      }
      return 0;
    case 0x1D:
      state->ctrl = release ? 0 : 1;
      if (!release) {
        return (size_t)write_token("<CTRL>", out, out_size, counted_out);
      }
      return 0;
    case 0x38:
      state->alt = release ? 0 : 1;
      if (!release) {
        return (size_t)write_token("<ALT>", out, out_size, counted_out);
      }
      return 0;
    case 0x3A:
      if (!release) {
        state->caps = !state->caps;
        return (size_t)write_token(state->caps ? "<CAPS_ON>" : "<CAPS_OFF>",
                                   out, out_size, counted_out);
      }
      return 0;
    default:
      break;
  }

  if (release) {
    return 0;
  }

  if (code == 0x01) {
    return (size_t)write_token("<ESC>", out, out_size, counted_out);
  }

  char ch = 0;
  if (state->shift && shifted_map[code]) {
    ch = shifted_map[code];
  } else {
    ch = normal_map[code];
  }

  if (ch == 0) {
    return 0;
  }

  if (is_letter(ch) && (state->shift ^ state->caps)) {
    ch = (char)(ch - 'a' + 'A');
  }

  out[0] = ch;
  out[1] = '\0';
  if (counted_out) {
    *counted_out = (ch == '\b') ? 0 : 1;
  }
  return 1;
}
