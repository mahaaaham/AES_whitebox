#ifndef CONVERSION_H
#define CONVERSION_H

#include <stdint.h>

typedef uint64_t double_t;
typedef uint32_t word_t;
typedef uint8_t byte_t;
typedef uint8_t nibble_t;

static const word_t m_nibble_in_word [8] =
  { 0x0000000f, 0x000000f0, 0x00000f00, 0x0000f000, 
    0x000f0000, 0x00f00000, 0x0f000000, 0xf0000000 };

static const word_t m_byte_in_word [4] =
  { 0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000 };

void word_to_nibbles (word_t word, nibble_t *nibbles);
void word_to_bytes (word_t word, byte_t *bytes);
void nibbles_to_bytes (nibble_t const *nibbles, byte_t *bytes);
word_t nibbles_to_word (nibble_t const *nibbles);

#endif /* CONVERSION_H */
