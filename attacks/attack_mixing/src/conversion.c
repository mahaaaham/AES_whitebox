#include "test.h" 

#include <stdio.h>
#include <stdlib.h>

static const word_t m_nibble_in_word [8] =
  { 0x0000000f, 0x000000f0, 0x00000f00, 0x0000f000, 
    0x000f0000, 0x00f00000, 0x0f000000, 0xf0000000 };

static const word_t m_byte_in_word [4] =
  { 0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000 };


void
word_to_nibbles (word_t word, nibble_t *nibbles)
{
  for (int i = 0; i < 8; ++i)
    nibbles[i] = (nibble_t)((word & m_nibble_in_word[i]) >> (4 * i));
  return;
}

void
word_to_bytes (word_t word, byte_t *bytes)
{
  for (int i = 0; i < 4; ++i)
    bytes[i] = (byte_t)((word & m_byte_in_word[i]) >> (8 * i));
  return;
}

void
nibbles_to_bytes (nibble_t const *nibbles, byte_t *bytes)
{
  for (int i = 0; i < 4; ++i)
    bytes[i] = nibbles[2 * i] | (nibbles[2 * i + 1] << 4);
}

word_t
nibbles_to_word (nibble_t const *nibbles)
{
  word_t word = nibbles[7];
  for (int i = 1; i < 8; ++i)
    word = (word << 4) + nibbles[7 - i];
  return word;
}
