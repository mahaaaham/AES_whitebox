#include "cipher.h"
#include "state.h"

#include <stdlib.h>
#include <stdio.h>


#define VERBOSE(string)					\
  do							\
    {							\
      if (verbose)					\
	{						\
	  fprintf (stdout, string);			\
	  fprintf (stdout, "\n\n");			\
	  state_write (stdout, state, hex, 4 * NB);	\
	}						\
    }							\
  while (0)


/* Implementation of the add_round_key subroutine from the aes algorithm */
void
add_round_key (byte_t **state, byte_t const *key_schedule, size_t round)
{
  for (int j = 0; j < NB; ++j)
    for (int i = 0; i < 4; ++i)
      state[i][j] ^= key_schedule[round * WORD * NK + 4 * j + i];
  
  VERBOSE ("add round key");
}


/* Implementation of the sub_bytes subroutine from the aes algorithm */
void
sub_bytes (byte_t **state)
{
  for (int i = 0; i < 4; ++i)
    for (int j = 0; j < NB; ++j)
      state[i][j] = sbox[state[i][j]];
  
  VERBOSE ("sub bytes");
}


/* Implementation of the inv_sub_bytes subroutine from the aes algorithm */
void
inv_sub_bytes (byte_t **state)
{
  for (int i = 0; i < 4; ++i)
    for (int j = 0; j < NB; ++j)
      state[i][j] = inv_sbox[state[i][j]];
  
  VERBOSE ("inv sub bytes");
}


/* Implementation of the shift_row subroutine from the aes algorithm */
void
shift_row (byte_t **state)
{
  byte_t tmp;

  /* Row 1: shift left once */
  tmp = state[1][0];
  state[1][0] = state[1][1];
  state[1][1] = state[1][2];
  state[1][2] = state[1][3];
  state[1][3] = tmp;

  /* Row 2: shift left twice */
  tmp = state[2][0];
  state[2][0] = state[2][2];
  state[2][2] = tmp;
  tmp = state[2][1];
  state[2][1] = state[2][3];
  state[2][3] = tmp;

  /* Row 3: shift left 3 times */
  tmp = state[3][0];
  state[3][0] = state[3][3];
  state[3][3] = state[3][2];
  state[3][2] = state[3][1];
  state[3][1] = tmp;

  VERBOSE ("shift rows");
}


/* Implementation of the inv_shift_row subroutine from the aes algorithm */
void
inv_shift_row (byte_t **state)
{
  byte_t tmp;

  /* Row 1: shift right once */
  tmp = state[1][0];
  state[1][0] = state[1][3];
  state[1][3] = state[1][2];
  state[1][2] = state[1][1];
  state[1][1] = tmp;

  /* Row 2: shift right twice */
  tmp = state[2][0];
  state[2][0] = state[2][2];
  state[2][2] = tmp;
  tmp = state[2][1];
  state[2][1] = state[2][3];
  state[2][3] = tmp;

  /* Row 3: shift right 3 times */
  tmp = state[3][0];
  state[3][0] = state[3][1];
  state[3][1] = state[3][2];
  state[3][2] = state[3][3];
  state[3][3] = tmp;

  VERBOSE ("inv shift rows");
}


/* Thanks to wikipedia "Finite Field Arithmetic" article */
byte_t
rijndael_mult (byte_t a, byte_t b)
{
  byte_t p = 0;
  
  while (a && b) 
    {
      if (b & 1) 
	p ^= a;
      
      /* GF modulo:
       * if a >= 128, then it will overflow when shifted left, so reduce */
      if (a & 0x80) 
	a = (a << 1) ^ 0x11b; /* XOR with the primitive polynomial 
			       * x^8 + x^4 + x^3 + x + 1 (0b1_0001_1011) */
      else
	a <<= 1; /* equivalent to a * 2 */
      
      b >>= 1; /* equivalent to b // 2 */
    }
  
  return p;
}


void
mix_columns (byte_t **state)
{
  for (int c = 0; c < 4; ++c)
    {
      byte_t s0 = rijndael_mult (0x02, state[0][c])
	^ rijndael_mult (0x03, state[1][c])
	^ state[2][c]
	^ state[3][c];
      byte_t s1 = state[0][c]
	^ rijndael_mult (0x02, state[1][c])
	^ rijndael_mult (0x03, state[2][c])
	^ state[3][c];
      byte_t s2 = state[0][c]
	^ state[1][c]
	^ rijndael_mult (0x02, state[2][c])
	^ rijndael_mult (0x03, state[3][c]);
      byte_t s3 = rijndael_mult (0x03, state[0][c]) 
	^ state[1][c]
	^ state[2][c] 
	^ rijndael_mult (0x02, state[3][c]);

      state[0][c] = s0;
      state[1][c] = s1;
      state[2][c] = s2;
      state[3][c] = s3;
    }
  
  VERBOSE ("mix columns");
}


void 
inv_mix_columns (byte_t **state)
{
  for (int c = 0; c < 4; ++c)
    {
      byte_t s0 = rijndael_mult (0x0e, state[0][c])
	^ rijndael_mult (0x0b, state[1][c]) 
	^ rijndael_mult (0x0d, state[2][c]) 
	^ rijndael_mult (0x09, state[3][c]);
      byte_t s1 = rijndael_mult (0x09, state[0][c])
	^ rijndael_mult (0x0e, state[1][c]) 
	^ rijndael_mult (0x0b, state[2][c]) 
	^ rijndael_mult (0x0d, state[3][c]);
      byte_t s2 = rijndael_mult (0x0d, state[0][c])
	^ rijndael_mult (0x09, state[1][c])
	^ rijndael_mult (0x0e, state[2][c])
	^ rijndael_mult (0x0b, state[3][c]);
      byte_t s3 = rijndael_mult (0x0b, state[0][c])
	^ rijndael_mult (0x0d, state[1][c])
	^ rijndael_mult (0x09, state[2][c])
	^ rijndael_mult (0x0e, state[3][c]);

      state[0][c] = s0;
      state[1][c] = s1;
      state[2][c] = s2;
      state[3][c] = s3;
    }

  VERBOSE ("inv mix columns");                         
}


void
cipher (byte_t **state, byte_t **input, byte_t const *key_schedule)
{
  /* Initialisation of the state */
  for (int i = 0; i < 4; ++i)
    for (int j = 0; j < NB; ++j)
      state[i][j] = input[i][j];
  
  VERBOSE ("\n"
	   "-----------------------\n"
	   "------ New block ------\n"
	   "-----------------------\n\n"
	   "input");

  /* Start enciphering */
  add_round_key (state, key_schedule, 0);

  for (int round = 1; round < NR; ++round)
    {
      if (verbose)
	printf ("------ Round %d ------\n\n", round);

      VERBOSE ("input");
      sub_bytes (state);
      shift_row (state);
      mix_columns (state);
      add_round_key (state, key_schedule, round);
    }

  if (verbose)
    printf ("------ Round 10 ------\n\n");

  VERBOSE ("input");
  sub_bytes (state);
  shift_row (state);
  add_round_key (state, key_schedule, NR);
}


void
inv_cipher (byte_t **state, byte_t **input, byte_t const *key_schedule)
{
  /* Initialisation of the state */
  for (int i = 0; i < 4; ++i)
    for (int j = 0; j < NB; ++j)
      state[i][j] = input[i][j];

  VERBOSE ("\n"
	   "-----------------------\n"
	   "------ New block ------\n"
	   "-----------------------\n\n"
	   "input");
  
  /* Start deciphering */
  add_round_key (state, key_schedule, NR);

  for (int round = NR - 1; round > 0; --round)
    {
      if (verbose)
	printf ("------ Round %d ------\n\n", round);
      
      VERBOSE ("input");
      inv_shift_row (state);
      inv_sub_bytes (state);
      add_round_key (state, key_schedule, round);
      inv_mix_columns (state);
    }

  if (verbose)
    printf ("------ Round 0 ------\n\n");

  VERBOSE ("input");
  inv_shift_row (state);
  inv_sub_bytes (state);
  add_round_key (state, key_schedule, 0);
}
