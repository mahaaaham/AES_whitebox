#ifndef MODE_H
#define MODE_H


#include "define.h"

#include <stdio.h>


/* Returns the byte to byte xor of blocks A and B */
void block_xor (byte_t **block_A_xor_B, byte_t **block_A, byte_t **block_B);

/* Encrypt input_file.
 * output_file is the result of the encryption.
 * Returns the number of bytes encrypted (without padding) */
int encipher (FILE *output_file, FILE *input_file, FILE *key_file,
	      format_t format, block_t block);

/* Decrypt input_file.
 * output_file is the result of the decryption.
 * Returns the number of bytes decrypted (without padding) */
int decipher (FILE *output_file, FILE *input_file, FILE *key_file,
	      format_t format, block_t block);


#endif /* MODE_H */
