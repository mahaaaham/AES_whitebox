#include "tables.h"
#include "sbox.h"

#include <stdio.h>
#include <stdlib.h>

#define NK 4
#define NB 4
#define WORD 4

byte_t
inv_Ty0 (word_t word)
{
  return (byte_t) (word >> 8);
}


byte_t
inv_Ty1 (word_t word)
{
  return (byte_t) (word >> 16);
}


byte_t
inv_Ty2 (word_t word)
{
  return (byte_t) word;
}


byte_t
inv_Ty3 (word_t word)
{
  return (byte_t) word;
}


int
main ()
{
  /* Allocate memory for the sought after key */
  byte_t *key = malloc (4 * NB);
  if (!key)
    return (EXIT_FAILURE);

  key[0] = inv_sbox[inv_Ty0 (A[0][0])];
  key[1] = inv_sbox[inv_Ty1 (B[3][0])];
  key[2] = inv_sbox[inv_Ty2 (C[2][0])];
  key[3] = inv_sbox[inv_Ty3 (D[1][0])];
  key[4] = inv_sbox[inv_Ty0 (A[1][0])];
  key[5] = inv_sbox[inv_Ty1 (B[0][0])];
  key[6] = inv_sbox[inv_Ty2 (C[3][0])];
  key[7] = inv_sbox[inv_Ty3 (D[2][0])];
  key[8] = inv_sbox[inv_Ty0 (A[2][0])];
  key[9] = inv_sbox[inv_Ty1 (B[1][0])];
  key[10] = inv_sbox[inv_Ty2 (C[0][0])];
  key[11] = inv_sbox[inv_Ty3 (D[3][0])];
  key[12] = inv_sbox[inv_Ty0 (A[3][0])];
  key[13] = inv_sbox[inv_Ty1 (B[2][0])];
  key[14] = inv_sbox[inv_Ty2 (C[1][0])];
  key[15] = inv_sbox[inv_Ty3 (D[0][0])];
  
  for (size_t i = 0; i < 4 * NB; ++i)
    printf ("%02x ", key[i]);
  printf ("\n");
  
  free (key);

  return (EXIT_SUCCESS);
}
