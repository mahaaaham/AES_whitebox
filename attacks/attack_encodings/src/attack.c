#include <stdio.h>
#include <stdlib.h>

#include <stdbool.h>
#include <string.h>

#include "COL_ROW.h"
#include "signature.h"
#include "tables.h"

#define FILENAME (strrchr ("/" __FILE__, '/') + 1)

#define EXIT(message, ...)						\
  do									\
    {									\
      fprintf (stderr, "%s: " message ".\n", FILENAME, __VA_ARGS__);	\
      exit (EXIT_FAILURE);                                              \
    }									\
  while (0)

#define WORD 4
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

static const word_t m_nibble_in_word [8] =
  { 0x0000000f, 0x000000f0, 0x00000f00, 0x0000f000, 
    0x000f0000, 0x00f00000, 0x0f000000, 0xf0000000 };

void
word_to_nibbles (word_t word, nibble_t *nibbles)
{
  for (int i = 0; i < 8; ++i)
    nibbles[i] = (nibble_t)((word & m_nibble_in_word[i]) >> (4 * i));
  return;
}

/* candidate is an (possibly no sorted) array of 8 frequency_signatures 
   and array is an (sorted) array of 6 frequency_signatures. 
   We check if it is true or false that 6 of the signatures of candidate 
   are the 6 signatures of array. */
bool
is_egal (double_t *candidate, double_t *array)
{
  int nb_fails = 0;
  int candidate_counter = 0;
  int array_counter = 0;
  qsort (candidate, 8, sizeof (double_t), comparaison_double);

  while (candidate_counter < 8 && array_counter < 6)
    {
      if (candidate[candidate_counter] != array[array_counter])
	{
	  ++nb_fails;
	  ++candidate_counter;
	}
      else 
	{
	  ++candidate_counter;
	  ++array_counter;
	}
    }
  if (nb_fails <= 2)
    return true;
  else 
    return false;
}

byte_t
analyse_word_array (word_t *array)
{
  nibble_t nibbles[8][16] = {0};
  double_t signatures[8];
  nibble_t to_convert[8];

  /* Getting the signatures */
  for (int i = 0; i < 16; ++i)
    {
      word_to_nibbles (array[i], to_convert); 
      for (int j = 0; j < 8; ++j)
	nibbles[j][i] = to_convert[j];
    }
  for (int i = 0; i < 8; ++i)
    signatures[i] = frequency_signature (nibbles[i]);

  /* Comparaison of the signature with R and C */
  nibble_t result;
  int counter = 0;
  while (counter < 16) 
    {
      if (is_egal (signatures, ROW[counter]))
	{
	  result = counter;
	  break;
	}
      if (is_egal (signatures, COL[counter]))
	{
	  result = counter;
	  break;
        }
      ++counter;
    }
  if (counter == 16)
    EXIT ("%s", "analyse_word_array: no correspondance found. " 
	        "Maybe mixing bijections are activated");

  return result;
}

int 
main ()
{
  byte_t key[16];

  word_t (*square_array)[16];
  word_t row_array[16];

  void *adresses[16] = {&A[0], &B[0], &C[0], &D[0],
		        &A[1], &B[1], &C[1], &D[1],
		        &A[2], &B[2], &C[2], &D[2],
		        &A[3], &B[3], &C[3], &D[3]};
  
  for (int l = 0; l < 16; ++l)
    {
      square_array = adresses[l]; 
      for (int j = 0; j < 16; ++j)
	row_array[j] = square_array[j][0];
      key[l] = analyse_word_array (row_array) 
               + 0x10 * analyse_word_array (square_array[0]);
    }

  byte_t tmp;
  INV_SHIFT_ROW (key, tmp);

  for (int l = 0; l < 16; ++l)
    printf ("%02x ", key[l]);
  printf ("\n");

  return EXIT_SUCCESS;
}
