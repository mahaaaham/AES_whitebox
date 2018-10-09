#ifndef STATE_H
#define STATE_H


#include "define.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>


/* Allocates memory for the state.
 * Returns a pointer to it or NULL if the allocation fails */
byte_t **state_alloc ();

/* Frees the state */
void state_free (byte_t **state);

/* Reads up to 4 * NB bytes from the 'input_file' and stores them in 'state'.
 * Memory for 'state' is supposed to have already been allocated.
 * 'input_file' is supposed to be open.
 * Returns the number of bytes read.
 * If 'format' is hex, returns -1 if a non hexa digit is detected. */
int state_read (byte_t **state, FILE *input_file, format_t format);

/* Pads a 'size' long state following the PKCS7 */
void state_add_padding (byte_t **state, int size);

/* Checks the padding of a 'size' long state:
 * remaining bytes must have the value '4 * NB - size' */
bool state_check_padding (byte_t **state, int size);

/* Writes 'size' bytes of 'state' to the specified stream */
void state_write (FILE *stream, byte_t **state, format_t format, size_t size);

/* Generates an initialisation vector */
void state_init (byte_t **init_vector);

/* Converts state given as an array into a double pointer.
 * This is useful in tests to pass double array as input values */
byte_t **state_convert (byte_t const *array);


#endif /* STATE_H */
