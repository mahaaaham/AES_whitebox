#ifndef CIPHER_H
#define CIPHER_H


#include "define.h"

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>


extern bool verbose;

extern const byte_t sbox[256];

extern const byte_t inv_sbox[256];


/* Implementation of the add_round_key subroutine from the aes algorithm */
void add_round_key (byte_t **state, byte_t const *key_schedule, size_t round);

/* Implementation of the sub_bytes subroutine from the aes algorithm */
void sub_bytes (byte_t **state);

/* Implementation of the inv_sub_bytes subroutine from the aes algorithm */
void inv_sub_bytes (byte_t **state);

/* Implementation of the shift_row subroutine from the aes algorithm */
void shift_row (byte_t **state);

/* Implementation of the inv_shift_row subroutine from the aes algorithm */
void inv_shift_row (byte_t **state);

/* Rijndael field multiplication */
byte_t rijndael_mult(byte_t a, byte_t b);

/* Implementation of the mix_columns subroutine from the aes algorithm */
void mix_columns (byte_t **state);

/* Implementation of the inv_mix_columns subroutine from the aes algorithm */
void inv_mix_columns (byte_t **state);

/* Aes cipher.
 * Returns in the state the input enciphered with the key_schedule. */
void cipher (byte_t **state, byte_t **input, byte_t const *key_schedule);

/* Aes inverse cipher.
 * Returns in the state the input deciphered with the key_schedule. */
void inv_cipher (byte_t **state, byte_t **input, byte_t const *key_schedule);


#endif /* CIPHER_H */
