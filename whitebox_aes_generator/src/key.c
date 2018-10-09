#include "key.h"
#include "sbox.h"

#include <stdlib.h>

#include <ctype.h>


#define WORD 4			/* A word is 4 bytes */
#define NK 4			/* Number of words in the key */
#define NR 10			/* Number of rounds */
#define NB 4			/* Number of columns in the state */

#define ROT_WORD(x)				\
  {						\
    byte_t y = x[0];				\
    x[0] = x[1];				\
    x[1] = x[2];				\
    x[2] = x[3];				\
    x[3] = y;					\
  }

#define SUB_WORD(x)				\
  {						\
    x[0] = sbox[x[0]];				\
    x[1] = sbox[x[1]];				\
    x[2] = sbox[x[2]];				\
    x[3] = sbox[x[3]];				\
  }

#define SHIFT_ROW(key, tmp)			\
{                                               \
    /* Row 1, shift of one to the left */	\
    tmp = (key)[1 + WORD * 3];                  \
    (key)[1 + WORD * 3] = (key)[1 + WORD * 0];  \
    (key)[1 + WORD * 0] = (key)[1 + WORD * 1];  \
    (key)[1 + WORD * 1] = (key)[1 + WORD * 2];  \
    (key)[1 + WORD * 2] = tmp;                  \
                                                \
    /* Row 2, shift of two to the left */	\
    tmp = (key)[2 + WORD * 3];                  \
    (key)[2 + WORD * 3] =  (key)[2 + WORD * 1]; \
    (key)[2 + WORD * 1] = tmp;                  \
    tmp = (key)[2 + WORD * 2];                  \
    (key)[2 + WORD * 2] = (key)[2 + WORD * 0];  \
    (key)[2 + WORD * 0] = tmp;                  \
                                                \
    /* Row 3, shift of three to the left */	\
    tmp = (key)[3 + WORD * 3];                  \
    (key)[3 + WORD * 3] =  (key)[3 + WORD * 2]; \
    (key)[3 + WORD * 2] = (key)[3 + WORD * 1];  \
    (key)[3 + WORD * 1] = (key)[3 + WORD * 0];  \
    (key)[3 + WORD * 0] = tmp;                  \
                                                \
  }

/* rcon[0] isn't used */
static const byte_t rcon[11] =
  { 0x8d, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x1b, 0x36 };


byte_t *
key_alloc ()
{
  byte_t *key = malloc (4 * NB * sizeof (byte_t));
  
  if (!key)
    return NULL;

  return key;
}


int
key_read (byte_t *key, FILE *key_file)
{
  int ch1, ch2;
  int cnt = 0;			/* Number of bytes read so far */

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
key_expansion (byte_t *key)
{
  byte_t *key_schedule = malloc (NB * (NR + 1) * WORD * sizeof (byte_t));
  if (!key_schedule)
    return NULL;

  byte_t temp[4];
  int i = 0;
  while (i < NK)
    {
      key_schedule[i * WORD] = key[i * WORD];
      key_schedule[i * WORD + 1] = key[i * WORD + 1];
      key_schedule[i * WORD + 2] = key[i * WORD + 2];
      key_schedule[i * WORD + 3] = key[i * WORD + 3];
      ++i;
    }

  while (i < NB * (NR + 1))
    {
      temp[0] = key_schedule[i * WORD - 4];
      temp[1] = key_schedule[i * WORD - 3];
      temp[2] = key_schedule[i * WORD - 2];
      temp[3] = key_schedule[i * WORD - 1];
      if ((i % NK) == 0)
	{
	  ROT_WORD (temp);
	  SUB_WORD (temp);
	  temp[0] ^= rcon[i / NK];
	}
      key_schedule[i * WORD] = key_schedule[(i - NK) * WORD] ^ temp[0];
      key_schedule[i * WORD + 1] = key_schedule[(i - NK) * WORD
						+ 1] ^ temp[1];
      key_schedule[i * WORD + 2] = key_schedule[(i - NK) * WORD
						+ 2] ^ temp[2];
      key_schedule[i * WORD + 3] = key_schedule[(i - NK) * WORD
						+ 3] ^ temp[3];
      ++i;
    }

  return key_schedule;
}


void
key_schedule_shift (byte_t *key_schedule)
{
  byte_t temp;
  for (int i = 0; i < (NR + 1); ++i)
    SHIFT_ROW (key_schedule + i * NB * WORD, temp);
  return;
}


void
key_schedule_write (FILE * stream, byte_t *key)
{
  for (int key_nb = 0; key_nb < (NR + 1); ++key_nb)
    {
      fprintf (stream, "\nkey number %d:\n", key_nb);
      for (int row = 0; row < NB; ++row)	/* row number */
	{
	  for (int column = 0; column < 4; ++column)	/* row number */
	    fprintf (stream, "%02x ", key[key_nb * (NB * WORD)
					  + row + WORD * column]);
	  fprintf (stream, "\n");
	}
    }
  return;
}
