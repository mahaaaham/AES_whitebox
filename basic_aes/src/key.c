#include "key.h"

#include <stdlib.h>

#include <ctype.h>


/* rcon[0] isn't used */
static const byte_t rcon[11] = 
  { 0x8d, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x1b, 0x36 };


void
rot_word (byte_t *word)
{
  byte_t tmp = word[0];
  
  word[0] = word[1];
  word[1] = word[2];
  word[2] = word[3];
  word[3] = tmp;
}

  
void
sub_word (byte_t *word)
{
  word[0] = sbox[word[0]];
  word[1] = sbox[word[1]];
  word[2] = sbox[word[2]];
  word[3] = sbox[word[3]];
}


/* Allocates 4 * NB bytes of memory for the key */
byte_t *
key_alloc ()
{
  byte_t *key = malloc (4 * NB * sizeof (byte_t));
  if (!key)
    return NULL;

  return key;
}


/* Frees the key */
void
key_free (byte_t *key)
{
  free (key);
}


/* Reads up to 4 * NB bytes from 'key_file' and stores them in 'key'.
 * Memory for 'key' is supposed to have already been allocated.
 * 'key_file' is supposed to be open.
 * Returns the number of bytes read or -1 if a non hexa digit is detected. */
int
key_read (byte_t *key, FILE *key_file)
{
  int ch1, ch2;
  int cnt = 0; /* Number of bytes read so far */

  do
    {
      do
	ch1 = fgetc (key_file);
      while isspace (ch1);

      if (ch1 == EOF)
	break;

      if ((ch1 < '0') || (('9' < ch1) && (ch1 < 'A'))
	  || (('F' < ch1) && (ch1 < 'a')) || ('f' < ch1))
	return -1;
      
      do
	ch2 = fgetc (key_file);
      while isspace (ch2);

      if ((ch2 < '0') || (('9' < ch2) && (ch2 < 'A'))
	  || (('F' < ch2) && (ch2 < 'a')) || ('f' < ch2))
	return -1;

      key[cnt] = (((ch1 % 32) + 9) % 25) * 16 + ((ch2 % 32) + 9) % 25;
    }
  while (++cnt < 4 * NB);
    
  return cnt;
}


byte_t *
key_expansion (byte_t const *key)
{
  byte_t *key_schedule = malloc (NB * (NR + 1) * WORD * sizeof (byte_t));
  if (!key_schedule)
    return NULL;

  byte_t tmp[4];
  int i = 0;
  while (i < NK)
    {
      key_schedule [i * WORD] = key [i * WORD];
      key_schedule [i * WORD + 1] = key [i * WORD + 1];
      key_schedule [i * WORD + 2] = key [i * WORD + 2];
      key_schedule [i * WORD + 3] = key [i * WORD + 3];
      ++i;
    }

  while (i < NB * (NR + 1))
    {
      tmp[0] = key_schedule [i * WORD - 4];
      tmp[1] = key_schedule [i * WORD - 3];
      tmp[2] = key_schedule [i * WORD - 2];
      tmp[3] = key_schedule [i * WORD - 1];
      if (i % NK == 0)
	{
	  rot_word (tmp);
	  sub_word (tmp); 
	  tmp[0] ^= rcon[i / NK];
	}
      key_schedule [i * WORD ] = key_schedule [(i - NK) * WORD] ^ tmp[0];
      key_schedule [i * WORD + 1] = key_schedule [(i - NK) * WORD
						  + 1] ^ tmp[1];
      key_schedule [i * WORD + 2] = key_schedule [(i - NK) * WORD 
						  + 2] ^ tmp[2];
      key_schedule [i * WORD + 3] = key_schedule [(i - NK) * WORD 
						  + 3] ^ tmp[3];
      ++i;
    }

  return key_schedule;
}
