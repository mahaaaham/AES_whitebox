#ifndef KEY_H
#define KEY_H


#include "define.h"

#include <stdint.h>
#include <stdio.h>


extern const byte_t sbox[256];

extern const byte_t inv_sbox[256];


/* Allocates memory for the key.
 * Returns a pointer to it or NULL if the allocation fails */
byte_t *key_alloc ();

void key_free (byte_t *key);

/* Reads up to 4 * NB bytes from 'key_file' and stores them in 'key'.
 * Memory for 'key' is supposed to have already been allocated.
 * 'key_file' is supposed to be open.
 * Returns the number of bytes read or -1 if a non hexa digit is detected. */
int key_read (byte_t *key, FILE *key_file);

/* Implementation of the key expansion routine from the aes algorithm.
 * Generates the key schedule (both memory allocation and filling).
 * Returns a pointer to it or NULL if the allocation fails. */
byte_t *key_expansion (byte_t const *key);

/* Implementation of the rot_word subroutine from the aes algorithm */
void rot_word (byte_t *word);

/* Implementation of the sub_word subroutine from the aes algorithm */
void sub_word (byte_t *word);  


#endif /* KEY_H */
