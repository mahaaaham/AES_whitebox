#include "inner_function.h"

#include "key.h"
#include "sbox.h"
#include "table_writing.h"

#include <stdio.h>
#include <stdlib.h>

#define INV_SHIFT_ROW(state, tmp)			  \
{							  \
    /* Row 1, shift of one to the right */		  \
    tmp = (state)[1 + WORD * 3];			  \
    (state)[1 + WORD * 3] = (state)[1 + WORD * 2];	  \
    (state)[1 + WORD * 2] = (state)[1 + WORD * 1];	  \
    (state)[1 + WORD * 1] = (state)[1 + WORD * 0];	  \
    (state)[1 + WORD * 0] = tmp;			  \
							  \
    /* Row 2, shift of two to the right */		  \
    tmp = (state)[2 + WORD * 3];			  \
    (state)[2 + WORD * 3] =  (state)[2 + WORD * 1];       \
    (state)[2 + WORD * 1] = tmp;                          \
    tmp = (state)[2 + WORD * 2];                          \
    (state)[2 + WORD * 2] = (state)[2 + WORD * 0];        \
    (state)[2 + WORD * 0] = tmp;                          \
                                                          \
    /* Row 3, shift of three to the right */              \
    tmp = (state)[3 + WORD * 3];                          \
    (state)[3 + WORD * 3] =  (state)[3 + WORD * 0];       \
    (state)[3 + WORD * 0] = (state)[3 + WORD * 1];        \
    (state)[3 + WORD * 1] = (state)[3 + WORD * 2];        \
    (state)[3 + WORD * 2] = tmp;                          \
                                                          \
  }

/* mixing bijections */
inv_matrix_t *mixing_bijection_1[4 * 9];	/* for rounds 1 to 9 */
/* Easier with two dimensions because of shift_row that
   melt the 16 bytes of the state */
inv_matrix_t *mixing_bijection_2[9][4 * 4];
matrix_t *mixing_bijection_2_concat[4 * 9];	/* for rounds 2 to 10 */


/* Thanks to wikipedia "Finite Field Arithmetic" article */
static byte_t
rijndael_mult (byte_t val_1, byte_t val_2)
{
  uint8_t result = 0;
  while ((val_1 && val_2) != 0)
    {
      if (val_2 & 1)
	result ^= val_1;

      if (val_1 & 0x80)		/* GF modulo: if val_1 >= 128 */
	val_1 = (val_1 << 1) ^ 0x11b;	/* XOR with the primitive polynomial 
					   x^8 + x^4 + x^3 + x + 1 
					   (0b1_0001_1011) */
      else
	val_1 <<= 1;

      val_2 >>= 1;
    }
  return result;
}

byte_t
table_tbox (byte_t indice, byte_t value, byte_t *shift_key)
{
  /* input mixing bijection */
  value = matrix_eval_byte (mixing_bijection_2[8][indice]->in, value);

  byte_t key_10[16];
  byte_t tmp;
  for (int i = 0; i < 16; ++i)
    key_10[i] = shift_key[10 * 16 + i];
  INV_SHIFT_ROW (key_10, tmp);

  byte_t return_value = sbox[value ^ shift_key[9 * NK * WORD + indice]]
    ^ key_10[indice];
  return return_value;
}

word_t
table_a (byte_t indice, byte_t value, byte_t *shift_key)
{
  /* input mixing bijection */
  if (indice >= 4)		/* Not in the first round */
    value =
      matrix_eval_byte (mixing_bijection_2[(indice / 4) - 1]
			[(indice % 4) * 4]->in, value);

  byte_t s = sbox[value ^ shift_key[indice * WORD]];

  word_t s0 = 0, s1 = 0, s2 = 0, s3 = 0;

  s0 = rijndael_mult (0x02, s);
  s1 = s;
  s2 = s;
  s3 = rijndael_mult (0x03, s);

  word_t return_value = s0 + (s1 << 8) + (s2 << 16) + (s3 << 24);

  /* output mixing bijection */
  return_value =
    matrix_eval_word (mixing_bijection_1[indice]->out, return_value);

  return return_value;
}

word_t
table_b (byte_t indice, byte_t value, byte_t *shift_key)
{
  /* input mixing bijection */
  if (indice >= 4)		/* Not in the first round */
    value =
      matrix_eval_byte (mixing_bijection_2[(indice / 4) - 1]
			[(indice % 4) * 4 + 1]->in, value);

  byte_t s = sbox[value ^ shift_key[indice * WORD + 1]];
  word_t s0 = 0, s1 = 0, s2 = 0, s3 = 0;

  s0 = rijndael_mult (0x03, s);
  s1 = rijndael_mult (0x02, s);
  s2 = s;
  s3 = s;

  word_t return_value = s0 + (s1 << 8) + (s2 << 16) + (s3 << 24);

  /* output mixing bijection */
  return_value =
    matrix_eval_word (mixing_bijection_1[indice]->out, return_value);

  return return_value;
}

word_t
table_c (byte_t indice, byte_t value, byte_t *shift_key)
{
  /* input mixing bijection */
  if (indice >= 4)		/* Not in the first round */
    value =
      matrix_eval_byte (mixing_bijection_2[(indice / 4) - 1]
			[(indice % 4) * 4 + 2]->in, value);

  byte_t s = sbox[value ^ shift_key[indice * WORD + 2]];
  word_t s0, s1, s2, s3;

  s0 = s;
  s1 = rijndael_mult (0x03, s);
  s2 = rijndael_mult (0x02, s);
  s3 = s;

  word_t return_value = s0 + (s1 << 8) + (s2 << 16) + (s3 << 24);

  /* output mixing bijection */
  return_value =
    matrix_eval_word (mixing_bijection_1[indice]->out, return_value);

  return return_value;
}

word_t
table_d (byte_t indice, byte_t value, byte_t *shift_key)
{
  /* input mixing bijection */
  if (indice >= 4)		/* Not in the first round */
    value =
      matrix_eval_byte (mixing_bijection_2[(indice / 4) - 1]
			[(indice % 4) * 4 + 3]->in, value);

  byte_t s = sbox[value ^ shift_key[indice * WORD + 3]];
  word_t s0 = 0, s1 = 0, s2 = 0, s3 = 0;

  s0 = s;
  s1 = s;
  s2 = rijndael_mult (0x03, s);
  s3 = rijndael_mult (0x02, s);

  word_t return_value = s0 + (s1 << 8) + (s2 << 16) + (s3 << 24);

  /* output mixing bijection */
  return_value =
    matrix_eval_word (mixing_bijection_1[indice]->out, return_value);

  return return_value;
}

word_t
table_w (byte_t indice, byte_t value,
	 __attribute__ ((unused)) byte_t *shift_key)
{
  word_t result = (word_t)value;

  /* input mixing bijection */
  result = matrix_eval_word (mixing_bijection_1[indice]->in, result);

  /* output mixing bijection */
  result = matrix_eval_word (mixing_bijection_2_concat[indice], result);

  return result;
}

word_t
table_x (byte_t indice, byte_t value,
	 __attribute__ ((unused)) byte_t *shift_key)
{
  word_t result = (word_t)value;

  /* input mixing bijection */
  result = result << 8;
  result = matrix_eval_word (mixing_bijection_1[indice]->in, result);

  /* output mixing bijection */
  result = matrix_eval_word (mixing_bijection_2_concat[indice], result);

  return result;
}


word_t
table_y (byte_t indice, byte_t value,
	 __attribute__ ((unused)) byte_t *shift_key)
{
  word_t result = (word_t)value;

  /* input mixing bijection */
  result = result << 8 * 2;
  result = matrix_eval_word (mixing_bijection_1[indice]->in, result);

  /* output mixing bijection */
  result = matrix_eval_word (mixing_bijection_2_concat[indice], result);

  return result;
}

word_t
table_z (byte_t indice, byte_t value,
	 __attribute__ ((unused)) byte_t *shift_key)
{
  word_t result = (word_t)value;

  /* input mixing bijection */
  result = result << 8 * 3;
  result = matrix_eval_word (mixing_bijection_1[indice]->in, result);

  /* output mixing bijection */
  result = matrix_eval_word (mixing_bijection_2_concat[indice], result);

  return result;
}

nibble_t
table_xor (__attribute__ ((unused)) byte_t indice, 
	    byte_t value, __attribute__ ((unused)) byte_t *unused)
{
  return (value >> 4) ^ ((byte_t)(value << 4) >> 4);
}
