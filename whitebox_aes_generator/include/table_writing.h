#ifndef TABLE_WRITING_H
#define TABLE_WRITING_H

#include <stdio.h>
#include <stdint.h>

typedef uint32_t word_t;
typedef uint8_t byte_t;
typedef uint8_t nibble_t;	/* purely psychological */

typedef nibble_t function_t[16];

struct bijection_t
{
  function_t in;
  function_t out;
};
typedef struct bijection_t bijection_t;

typedef function_t word_plug_t[8];
typedef function_t byte_plug_t[2];
typedef function_t nibble_plug_t;


/* write an array  "word_t NAME[NB_ARRAY][256]"  in the FILE *FD.  
   for 0 <= i <  NB_ARRAY  and  0 <= j < 256, 
   NAME[i][j] is the word 
   e_out [function (i, e_in[j], shift_key)]
   (so, first the input encoding e_in, second the function and finally 
   the output encoding e_out). */
void create_word_array (char *name, int nb_array, byte_t *shift_key,
			byte_plug_t e_in[], word_plug_t e_out[],
			word_t (*function) (byte_t, byte_t, byte_t *),
			FILE * fd);

/* as create_word_array except that it is an array of byte_t instead of word_t
   elements */
void create_byte_array (char *name, int nb_array, byte_t *shift_key,
			byte_plug_t e_in[], byte_plug_t e_out[],
			byte_t (*function) (byte_t, byte_t, byte_t *),
			FILE * fd);

/* as create_word_array except that it is an array of nibble_t instead of word_t
   elements */
void create_nibble_array (char *name, int nb_array, byte_t *shift_key,
			  byte_plug_t e_in[], nibble_plug_t e_out[],
			  nibble_t (*function) (byte_t, byte_t, byte_t *),
			  FILE * fd);

#endif /* TABLE_WRITING_H */
