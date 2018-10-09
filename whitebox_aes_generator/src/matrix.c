#include "matrix.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <fcntl.h>	/* to define the random_bit () function */
#include <unistd.h>

#include <math.h>

#include <gsl/gsl_matrix_double.h>
#include <gsl/gsl_linalg.h>

struct matrix_t
{
  size_t n;
  nibble_t *m;
};

/* Use the gsl determinant who is faster than the naive implementation */
nibble_t
matrix_determinant (matrix_t mat)
{
  gsl_matrix *gsl_mat = gsl_matrix_alloc (mat.n, mat.n);

  for (size_t row = 0; row < mat.n; ++row)
    for (size_t column = 0; column < mat.n; ++column)
      gsl_matrix_set (gsl_mat, row, column,
		      (double)mat.m[row * mat.n + column]);

  int sign = 0;
  double det = 0.0;
  int row_sq = gsl_mat->size1;
  gsl_permutation *p = gsl_permutation_calloc (row_sq);
  gsl_matrix *tmp_ptr = gsl_matrix_calloc (row_sq, row_sq);
  int *signum = &sign;

  gsl_matrix_memcpy (tmp_ptr, gsl_mat);
  gsl_linalg_LU_decomp (tmp_ptr, p, signum);

  det = gsl_linalg_LU_det (tmp_ptr, *signum);

  gsl_permutation_free (p);
  gsl_matrix_free (tmp_ptr);
  gsl_matrix_free (gsl_mat);

  byte_t result;
  result = (((long int)round (det) % 2) == 0 ? 0 : 1);
  return result;
}


void
matrix_concatenate (matrix_t *mat0, matrix_t *mat1,
		    matrix_t *mat2, matrix_t *mat3, matrix_t *result)
{
  if (mat0->n != 8 || mat1->n != 8
      || mat2->n != 8 || mat3->n != 8 || result->n != 32)
    return;

  for (size_t i = 0; i < result->n; ++i)
    for (size_t j = 0; j < result->n; ++j)
      result->m[i * result->n + j] = 0;

  for (size_t i = 0; i < 4; ++i)
    for (size_t j = 0; j < 8; ++j)
      for (size_t k = 0; k < 8; ++k)
	{
	  result->m[j * result->n + k] = mat0->m[j * mat0->n + k];
	  result->m[(1 * 8 + j) * result->n
		    + (1 * 8 + k)] = mat1->m[j * mat1->n + k];
	  result->m[(2 * 8 + j) * result->n
		    + (2 * 8 + k)] = mat2->m[j * mat2->n + k];
	  result->m[(3 * 8 + j) * result->n
		    + (3 * 8 + k)] = mat3->m[j * mat3->n + k];
	}
  return;
}



/* matrix_transpose of a square matrix_t, do it in place */
void
matrix_transpose (matrix_t mat, matrix_t mat_result)
{
  nibble_t tmp;

  for (size_t i = 1; i < mat.n; ++i)
    for (size_t j = 0; j < i; ++j)
      {
	tmp = mat.m[i * mat.n + j];
	mat_result.m[i * mat.n + j] = mat.m[j * mat.n + i];
	mat_result.m[j * mat.n + i] = tmp;
      }
  return;
}

void
matrix_inverse (matrix_t mat, matrix_t mat_inv)
{
  size_t i1, j1;
  size_t size = mat.n;

  matrix_t comat;
  comat.n = size - 1;
  comat.m = malloc (comat.n * comat.n * sizeof (nibble_t));
  if (!comat.m)
    return;

  for (size_t j = 0; j < size; ++j)
    {
      for (size_t i = 0; i < size; ++i)
	{
	  /* Form the adjoint a_ij */
	  i1 = 0;

	  for (size_t ii = 0; ii < size; ++ii)
	    {
	      if (ii == i)
		continue;

	      j1 = 0;
	      for (size_t jj = 0; jj < size; ++jj)
		{
		  if (jj == j)
		    continue;
		  comat.m[i1 * comat.n + j1] = mat.m[ii * mat.n + jj];
		  j1++;
		}
	      i1++;
	    }

	  /* Fill in the elements of the cofactor */
	  mat_inv.m[i * mat_inv.n + j] = matrix_determinant (comat);
	}
    }
  free (comat.m);
  matrix_transpose (mat_inv, mat_inv);
  return;
}



void
matrix_prod (matrix_t mat1, matrix_t mat2, matrix_t result)
{
  if ((mat1.n != mat2.n) || (mat1.n != result.n))
    return;

  size_t size = mat1.n;

  for (size_t row = 0; row < size; ++row)
    for (size_t column = 0; column < size; ++column)
      result.m[row * result.n + column] = 0;

  for (size_t row = 0; row < size; ++row)
    for (size_t column = 0; column < size; ++column)
      for (size_t k = 0; k < size; ++k)
	result.m[row * result.n + column] ^= ((mat1.m[row * mat1.n + k]
					       & mat2.m[k * mat2.n +
							column]));
  return;
}

void
matrix_set_to_id (matrix_t *mat)
{
  for (size_t row = 0; row < mat->n; ++row)
    for (size_t column = 0; column < mat->n; ++column)
      mat->m[row * mat->n + column] = 0;

  for (size_t i = 0; i < mat->n; ++i)
    mat->m[i * mat->n + i] = 1;

  return;
}

nibble_t
random_bit ()
{
  nibble_t buffer;
  int fd = open ("/dev/urandom", O_RDONLY);
  read (fd, &buffer, 1);
  close (fd);
  buffer = buffer >> 7;
  return buffer;
}

/* exported functions: */
void
matrix_free (matrix_t *mat)
{
  free (mat->m);
  free (mat);
  return;
}

matrix_t *
matrix_init (size_t size)
{
  if (size == 0)
    return NULL;
  matrix_t *mat = malloc (sizeof (matrix_t));
  if (!mat)
    return NULL;

  mat->n = size;
  mat->m = malloc (mat->n * mat->n * sizeof (nibble_t));

  if (!mat->m)
    {
      free (mat);
      return NULL;
    }
  return mat;
}

void
matrix_random (matrix_t *mat, bool activated)
{
  if (!activated)
    {
      matrix_set_to_id (mat);
      return;
    }

  for (size_t row = 0; row < mat->n; ++row)
    for (size_t column = 0; column < mat->n; ++column)
      mat->m[row * mat->n + column] = random_bit ();
}

void
inv_matrix_random (inv_matrix_t *inv_mat, bool activated)
{
  bool inversible = false;
  int count = 0;
  byte_t det;

  if (!activated)
    {
      matrix_set_to_id (inv_mat->in);
      matrix_set_to_id (inv_mat->out);
      return;
    }
  matrix_t prod = { inv_mat->in->n, NULL };
  prod.m = malloc (prod.n * prod.n * sizeof (nibble_t));
  if (!prod.m)
    return;

  while (!inversible)
    {
      matrix_random (inv_mat->in, 'r');
      ++count;
      det = round (matrix_determinant (*inv_mat->in));
      if (det != 0)
	{
	  inversible = true;
	  matrix_inverse (*inv_mat->in, *inv_mat->out);
	}
    }
  free (prod.m);
  return;
}

byte_t
matrix_eval_byte (matrix_t *mat, byte_t byte)
{
  if (mat->n != 8)
    return 0;

  byte_t byte_tab[8] = { 0 };
  byte_t result_tab[8] = { 0 };
  byte_t result = 0;

  for (size_t i = 0; i < mat->n; ++i)
    byte_tab[i] = (byte & ((byte_t)1 << i)) >> i;

  for (size_t i = 0; i < mat->n; ++i)
    for (size_t col = 0; col < mat->n; ++col)
      result_tab[i] ^= (mat->m[i * mat->n + col] * byte_tab[col]);

  for (size_t i = 0; i < mat->n; ++i)
    result |= (result_tab[i] << i);

  return result;
}

word_t
matrix_eval_word (matrix_t *mat, word_t word)
{
  if (mat->n != 32)
    return 0;

  byte_t word_tab[32] = { 0 };
  byte_t result_tab[32] = { 0 };

  word_t result = 0;

  for (size_t i = 0; i < mat->n; ++i)
    word_tab[i] = (word & ((word_t)1 << i)) >> i;

  for (size_t i = 0; i < mat->n; ++i)
    for (size_t col = 0; col < mat->n; ++col)
      result_tab[i] ^= (mat->m[i * mat->n + col] & word_tab[col]);

  for (size_t i = 0; i < mat->n; ++i)
    result |= ((word_t)result_tab[i] << i);

  return result;
}

void
inv_matrix_free (inv_matrix_t *inv_mat)
{
  matrix_free (inv_mat->in);
  matrix_free (inv_mat->out);
  free (inv_mat);
}

inv_matrix_t *
inv_matrix_init (size_t size)
{
  inv_matrix_t *inv_mat = malloc (sizeof (inv_matrix_t));
  if (!inv_mat)
    return NULL;

  inv_mat->in = matrix_init (size);
  if (!inv_mat->in)
    {
      free (inv_mat);
      return NULL;
    }

  inv_mat->out = matrix_init (size);
  if (!inv_mat->out)
    {
      free (inv_mat->in);
      free (inv_mat);
      return NULL;
    }

  return inv_mat;
}

void
matrix_write (FILE * stream, matrix_t *mat)
{
  for (size_t i = 0; i < mat->n; ++i)
    {
      for (size_t j = 0; j < mat->n; ++j)
	fprintf (stream, " %x ", mat->m[i * mat->n + j]);
      fprintf (stream, "\n");
    }
  return;
}
