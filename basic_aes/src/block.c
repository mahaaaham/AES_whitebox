#include "block.h"
#include "cipher.h"
#include "key.h"
#include "state.h"

#include <stdlib.h>

#include <string.h>


#define FILENAME (strrchr ("/" __FILE__, '/') + 1)

#define ASSERT(statement)					\
  do								\
    {								\
      if (!(statement))						\
	{							\
	  fprintf (stderr, "%s: %d: statement '" #statement	\
		   "' failed.\n", FILENAME, __LINE__);		\
	  text_size = -1;					\
	  goto Fail;						\
	}							\
    }								\
  while (0)

#define EXIT(message, ...)						\
  do									\
    {									\
      fprintf (stderr, "%s: " message ".\n", FILENAME, __VA_ARGS__);	\
      text_size = -1;							\
      goto Fail;							\
    }									\
  while (0)


/* Returns the byte to byte xor of blocks A and B */
void
block_xor (byte_t **block_A_xor_B, byte_t **block_A, byte_t **block_B)
{
  for (int i = 0; i < 4; ++i)
    for (int j = 0; j < NB; ++j)
      block_A_xor_B[i][j] = block_A[i][j] ^ block_B[i][j];
}


/* Encrypt input_file.
 * output_file is the result of the encryption.
 * Returns the number of bytes encrypted (without padding) */
int
encipher (FILE *output_file, FILE *input_file, FILE *key_file,
	  format_t format, block_t block)
{
  int text_size = 0;
  
  byte_t *key = NULL;
  byte_t *key_schedule = NULL;
  byte_t **plaintext = NULL;
  byte_t **ciphertext = NULL;
  byte_t **state = NULL;
  
  /* Grab the key */
  key = key_alloc ();
  ASSERT (key);
  int key_len = key_read (key, key_file);
  if (key_len != 4 * NB)
    EXIT ("%s", "wrong key length");

  /* Expand the key */
  key_schedule = key_expansion (key);
  ASSERT (key_schedule);
  
  plaintext = state_alloc ();
  ASSERT (plaintext);

  ciphertext = state_alloc ();
  ASSERT (ciphertext);

  int block_size;
  
  switch (block)
    {
    case ECB:
      /* Feed the input file to the cipher block by block */
      do
	{
	  /* Read the next block of input
	   * and check its characters if format is hexadecimal */
	  block_size = state_read (plaintext, input_file, format);
	  if (block_size == -1)
	    EXIT ("%s", "non hexadecimal character found");
	  
	  /* Pad the plaintext if necessary */
	  if (block_size < 4 * NB)
	    state_add_padding (plaintext, block_size);

	  /* Run the plaintext through the cipher */
	  cipher (ciphertext, plaintext, key_schedule);

	  /* Write the resulting ciphertext to the output file */
	  state_write (output_file, ciphertext, format, 4 * NB);
	  text_size += block_size;
	}
      while (block_size == 4 * NB);
      
      break;
      
    case CBC:
      /* Generate an initialisation vector */
      state_init (ciphertext);
      state_write (output_file, ciphertext, format, 4 * NB);

      state = state_alloc ();
      ASSERT (state);
      
      /* Feed the input file to the cipher block by block */
      do
	{
	  /* Read the next block of plaintext,
	   * check it for non hexadecimal character if format is hex,
	   * pad it if necessary
	   * and xor it with the previous enciphered block */
	  block_size = state_read (plaintext, input_file, format);
	  if (block_size == -1)
	    EXIT ("%s", "non hexadecimal character found");
	  
	  if (block_size < 4 * NB)
	    state_add_padding (plaintext, block_size);
	  
	  block_xor (state, ciphertext, plaintext);

	  /* Run the state through the cipher */
	  cipher (ciphertext, state, key_schedule);

	  /* Write the resulting ciphertext to the output file */
	  state_write (output_file, ciphertext, format, 4 * NB);
	  text_size += block_size;
	}
      while (block_size == 4 * NB);
      
      break;
    }

 Fail:
  /* Clean up */
  if (key)
    key_free (key);
  if (key_schedule)
    key_free (key_schedule);
  if (plaintext)
    state_free (plaintext);
  if (ciphertext)
    state_free (ciphertext);
  if (state)
    state_free (state);

  return text_size;
}


/* Decrypt input_file.
 * output_file is the result of the decryption.
 * Returns the number of bytes decrypted (without padding) */
int
decipher (FILE *output_file, FILE *input_file, FILE *key_file,
	  format_t format, block_t block)
{
  int text_size = 0;
  
  byte_t *key = NULL;
  byte_t *key_schedule = NULL;
  byte_t **ciphertext = NULL;
  byte_t **plaintext = NULL;
  byte_t **prev_ciphertext = NULL;
  byte_t **state = NULL;  
  
  /* Grab the key */
  key = key_alloc ();
  ASSERT (key);
  size_t key_len = key_read (key, key_file);
  if (key_len != 4 * NB)
    EXIT ("%s", "wrong key_length");

  /* Expand the key */
  key_schedule = key_expansion (key);
  ASSERT (key_schedule);
  
  ciphertext = state_alloc ();
  ASSERT (ciphertext);

  plaintext = state_alloc ();
  ASSERT (plaintext);

  int block_size = 4 * NB;
  int next_block_size;
  
  switch (block)
    {
    case ECB:
      /* Read the first block of ciphertext, check its size
       * and check it for non hexadecimal characters if format is hex */
      next_block_size = state_read (ciphertext, input_file, format);
      if (next_block_size == -1)
	EXIT ("%s", "non hexadecimal character found");
      if (next_block_size != 4 * NB)
	EXIT ("%s", "encrypted file contains less than one block of data");
      
      /* Feed the input file to the inverse cipher block by block */
      do
	{
	  /* Run the ciphertext through the inverse cipher */
	  inv_cipher (plaintext, ciphertext, key_schedule);

	  /* Read the next block, check its size
	   * and check it for non hexadecimal characters if format is hex */
	  next_block_size = state_read (ciphertext, input_file, format);
	  if (next_block_size == -1)
	    EXIT ("%s", "non hexadecimal character found");
	  if (next_block_size % (4 * NB) != 0)
	    EXIT ("%s", "ciphertext length is not a multiple of block size");

	  /* If this was the last block, check the padding and cut it */
	  if (!next_block_size)
	    {
	      block_size -= plaintext[3][NB - 1];
	      if (!state_check_padding (plaintext, block_size))
		EXIT ("%s", "corrupted padding");
	    }
	  
	  /* Write the resulting plaintext to the output file */
	  if (block_size)
	    {
	      state_write (output_file, plaintext, format, block_size);
	      text_size += block_size;
	    }
	}
      while (next_block_size);
      
      break;
      
    case CBC:;
      prev_ciphertext = state_alloc ();
      ASSERT (prev_ciphertext);

      state = state_alloc ();
      ASSERT (state);

      /* Pointer used for swapping pointers ciphertext and prev_ciphertext */
      byte_t **ptr = prev_ciphertext;
      
      /* Read the first block of encrypted text (which is the IV),
       * check its size and for non hexadecimal character if needed */
      next_block_size = state_read (prev_ciphertext, input_file, format);
      if (next_block_size == -1)
	EXIT ("%s", "non hexadecimal character found");
      if (next_block_size != 4 * NB)
	EXIT ("%s", "encrypted file contains less than one block of data");


      /* Read the second block of encrypted text, check its size
       * and check it for non hexadecimal character if needed */
      next_block_size = state_read (ciphertext, input_file, format);
      if (next_block_size == -1)
	EXIT ("%s", "non hexadecimal character found");
      if (next_block_size != 4 * NB)
	EXIT ("%s", "encrypted file contains less than one block of data");
      
      /* Decipher a block at a time */
      do
	{
	  /* Run the ciphertext through the inverse cipher */
	  inv_cipher (state, ciphertext, key_schedule);
	  
	  /* xor the decrypted block (state)
	   * with the previous encrypted block (prev_ciphertext) */
	  block_xor (plaintext, state, prev_ciphertext);

	  /* ciphertext becomes prev_ciphertext */
	  prev_ciphertext = ciphertext;
	  ciphertext = ptr;
	  ptr = prev_ciphertext;

	  /* Read the next block of ciphertext, check its size
	   * and check it for non hexadecimal character if needed */
	  next_block_size = state_read (ciphertext, input_file, format);
	  if (next_block_size == -1)
	    EXIT ("%s", "non hexadecimal character found");
	  if (next_block_size % (4 * NB) != 0)
	    EXIT ("%s", "ciphertext length is not a multiple of block size");

	  /* If this was the last block, check the padding and cut it */
	  if (!next_block_size)
	    {
	      block_size -= plaintext[3][NB - 1];
	      if ((block_size < 0) || (16 < block_size)
		  || (!state_check_padding (plaintext, block_size)))
		EXIT ("%s", "corrupted padding");
	    }
	  
	  /* Write the resulting plaintext to the output file */
	  if (block_size)
	    {
	      state_write (output_file, plaintext, format, block_size);
	      text_size += block_size;
	    }
	}
      while (next_block_size);
      
      break;
    }

 Fail:
  /* Clean up */
  if (key)
    key_free (key);
  if (key_schedule)
    key_free (key_schedule);
  if (ciphertext)
    state_free (ciphertext);
  if (plaintext)
    state_free (plaintext);
  if (prev_ciphertext)
    state_free (prev_ciphertext);
  if (state)
    state_free (state);

  return text_size;
}
