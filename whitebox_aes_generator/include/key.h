#ifndef KEY_H
#define KEY_H

#include <stdint.h>
#include <stdio.h>

typedef uint8_t byte_t;


/* Allocates 4 * NB bytes of memory for the key */
byte_t *key_alloc ();

/* Free the key allocated by key_alloc () */ 
void key_free (byte_t *key);

/* Reads up to 4 * NB bytes from the stream 'key_file' and stores them in 
 * 'key'. Memory for 'key' is supposed to have already been allocated.
 * 'key_file' is supposed to be open.
 * Returns the number of bytes read or -1 if a non hexa digit is detected. */
int key_read (byte_t *key, FILE * key_file);

/* Expand the key KEY into the expanded key needed by the AES algorithm. */
byte_t *key_expansion (byte_t *key);

/* Apply SHIFT_ROW to the 11 keys of the key_schedule */
void key_schedule_shift (byte_t *key);

/* Write in the stream STREAM the expanded key (so, 
   of the size (NR + 1) * NB words = 44 words). */
void key_schedule_write (FILE * stream, byte_t *key);

#endif /* KEY_H */
