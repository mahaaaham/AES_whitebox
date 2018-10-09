#ifndef MATRIX_H
#define MATRIX_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef uint8_t byte_t;
typedef uint32_t word_t;
typedef uint8_t nibble_t;

/* implementation in matrix.c. a 
   matrix_t element is a square matrix
   with values { 0, 1 } */
struct matrix_t;
typedef struct matrix_t matrix_t;

/* used in generate_table.c */
struct inv_matrix_t
{
  matrix_t *in;
  matrix_t *out;
};
typedef struct inv_matrix_t inv_matrix_t;

/* function prototypes */

/* initialise a matrix_t element of size SIZE
   and returns a pointer to it. */
matrix_t *matrix_init (size_t size);
/* free the matrix pointed by mat */
void matrix_free (matrix_t *mat);

/* initialise a inv_matrix_t element of size SIZE
   and returns a pointer to it. */
inv_matrix_t *inv_matrix_init (size_t size);
/* free the inv_matrix_t pointed by inv_mat */
void inv_matrix_free (inv_matrix_t *inv_mat);


/* mat is supposed to be initialised, it is filled
   by aleatory values (in { 0, 1 }) */
void matrix_random (matrix_t *mat, bool mode);

/* inv_mat is supposed to be initialised, it is filled
   by an inversible matrix and its inverse */
void inv_matrix_random (inv_matrix_t *inv_mat, bool mode);

/* write the matrix into the stream */
void matrix_write (FILE * stream, matrix_t *mat);

/* mat0, ... , mat3 are pointers to matrix
   of size 8
   and result a matrix of size 32. This
   fonction transforms result in:
( mat0   0    0    0   )
(  0    mat1  0    0   ) 
(  0     0   mat3  0   )
(  0     0    0   mat4 ) */
void matrix_concatenate (matrix_t *mat0, matrix_t *mat1,
			 matrix_t *mat2, matrix_t *mat3, matrix_t *result);

/* mat is a pointer to a matrix of size 8*8. 
   This function returns the product of the matrix pointed
   by mat and the byte_t BYTE seen as a column vector of size 8. */
byte_t matrix_eval_byte (matrix_t *mat, byte_t byte);

/* mat is a pointer to a matrix of size 32*32. 
   This function returns the product of the matrix pointed
   by mat and the word_t WORD seen as a column vector of size 32. */
word_t matrix_eval_word (matrix_t *mat, word_t word);

#endif /* MATRIX_H */
