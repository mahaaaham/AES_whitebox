#include "test.h"
#include "cipher.h"
#include "key.h"

#include "table_writting.h"
#include "matrix.h"

#include <stdio.h>
#include <stdlib.h>

/* Some internal test used to be sure the fonctions defined 
   really do what we want, see the .h */
void
matrix_concatenate_test (char mode)
{
  matrix_t *mat0 = matrix_init (8);
  matrix_t *mat1 = matrix_init (8);
  matrix_t *mat2 = matrix_init (8);
  matrix_t *mat3 = matrix_init (8);

  matrix_random (mat0, mode);
  matrix_random (mat1, mode);
  matrix_random (mat2, mode);
  matrix_random (mat3, mode);

  matrix_t *result = matrix_init (32);

  matrix_concatenate (mat0, mat1, mat2, mat3, result);

  matrix_write (stdout, result);

  matrix_free (mat0);
  matrix_free (mat1);
  matrix_free (mat2);
  matrix_free (mat3);

  return;
}

void
matrix_word_test (word_t word, size_t nb_test)
{
  size_t size = 32;
  int error = 0;

  inv_matrix_t *couple = inv_matrix_init (size);

  word_t tmp;

  for (size_t i = 0; i < nb_test; ++i)
    {
      inv_matrix_random (couple, 'r');

      tmp = matrix_eval_word (couple->in, word);
      tmp = matrix_eval_word (couple->out, tmp);

      if (tmp != word)
	++error;
    }
  printf ("result matrix_word_test: %d errors on %zu tests.\n", error,
	  nb_test);
  inv_matrix_free (couple);
  return;
}

void
matrix_byte_test (byte_t byte, size_t nb_test)
{
  size_t size = 8;
  int error = 0;

  inv_matrix_t *couple = inv_matrix_init (size);

  byte_t tmp;

  for (size_t i = 0; i < nb_test; ++i)
    {
      inv_matrix_random (couple, 'r');

      tmp = matrix_eval_byte (couple->in, byte);
      tmp = matrix_eval_byte (couple->out, tmp);

      if (tmp != byte)
	++error;

    }
  printf ("result matrix_byte_test: %d errors on %zu tests.\n", error,
	  nb_test);
  inv_matrix_free (couple);
  return;
}

