#include "state.h"

#include <stdio.h>
#include <stdlib.h>

#include <ctype.h>
/* #include <time.h> */


/* Allocates memory for the state.
 * Returns a pointer to it or NULL if the allocation fails */
byte_t **
state_alloc ()
{ 
  byte_t **state = (byte_t **) malloc (4 * sizeof (byte_t *)); 
  if (!state)
    return NULL;
  
  for (int i = 0; i < 4; ++i)
    {
      state[i] = malloc (NB * sizeof (byte_t)); 
      if (!state[i])
	{
	  for (int j = 0; j < i; ++j)
	    free (state[j]);
	  return NULL;
	}
    }
  
  return state; 
}


/* Frees the state */
void
state_free (byte_t **state)
{
  for (int i = 0; i < 4; ++i)
    free (state[i]);
  free (state);
}


/* Reads up to 4 * NB bytes from the 'input_file' and stores them in 'state'.
 * Memory for 'state' is supposed to have already been allocated.
 * 'input_file' is supposed to be open.
 * Returns the number of bytes read.
 * If 'format' is hex, returns -1 if a non hexa digit is detected. */
int
state_read (byte_t **state, FILE *input_file, format_t format)
{
  int row = 0;
  int col = 0;

  if (format == ascii)
    {
      int chr;
      
      while ((col < NB) && ((chr = fgetc (input_file)) != EOF))
	{
	  state[row][col] = chr;

	  if (row < 3)
	    ++row;
	  else
	    ++col, row = 0;
	}
    }
  else /* format = hex */
    {
      int ch1, ch2;
      
      do
	{
	  do
	    ch1 = fgetc (input_file);
	  while (isspace (ch1));

	  if (ch1 == EOF)
	    break;

	  if ((ch1 < '0') || (('9' < ch1) && (ch1 < 'A'))
	      || (('F' < ch1) && (ch1 < 'a')) || ('f' < ch1))
	    return -1;
      
	  do
	    ch2 = fgetc (input_file);
	  while (isspace (ch2));

	  if ((ch2 < '0') || (('9' < ch2) && (ch2 < 'A'))
	      || (('F' < ch2) && (ch2 < 'a')) || ('f' < ch2))
	    return -1;

	  state[row][col] = (((ch1 % 32) + 9) % 25) * 16 + ((ch2 % 32) + 9) % 25;

	  if (row < 3)
	    ++row;
	  else
	    ++col, row = 0;
	}
      while (col < NB);
    }
  
  return 4 * col + row;
}


/* Pads a 'size' long state following the PKCS#7 */
void
state_add_padding (byte_t **state, int size)
{
  int row = size % NB;
  int col = size / NB;
  byte_t pad = 4 * NB - size;

  do
    {
      state[row][col] = pad;
      if (row < 3)
	++row;
      else
	++col, row = 0;
    }
  while (++size < 4 * NB);
}


/* Checks the padding of a 'size' long state:
 * the '4 * NB - size' padding bytes must have the value '4 * NB - size' */
bool
state_check_padding (byte_t **state, int size)
{
  if ((size < 0) || (15 < size))
    return false;
  
  int row = size % NB;
  int col = size / NB;
  byte_t pad = 4 * NB - size;

  do
    {
      if (state[row][col] != pad)
	return false;
    }
  while (++size < 4 * NB);

  return true;
}


/* Writes 'size' bytes of 'state' to the specified stream */
void
state_write (FILE *stream, byte_t **state, format_t format, size_t size)
{
  int row = 0;
  int col = 0;

  if (format == ascii)
    do
      {
	fputc (state[row][col], stream);
	if (row < 3)
	  ++row;
	else
	  ++col, row = 0;
      }
    while (--size);
  else /* format = hex */
    {
      do
	{
	  fprintf (stream, "%02x", state[row][col]);
	  if (row < 3)
	    ++row;
	  else
	    {
	      ++col, row = 0;
	      fprintf (stream, "\n");
	    }
	}
      while (--size);
      
      fprintf (stream, "\n");
    }
} 


/* Generates an initialisation vector */
void
state_init (byte_t **init_vector)
{
  for (int j = 0; j < NB; ++j)
    for (int i = 0; i < 4; ++i)
      init_vector[i][j] = 0;
}


/* Converts state given as an array into a double pointer.
 * This is useful in tests to pass double array as input values */
byte_t **
state_convert (byte_t const *array)
{
  byte_t **state = state_alloc ();
  if (!state)
    return NULL;
    
  for (int j = 0; j < NB; ++j)
    for (int i = 0; i < 4; ++i)
      state[i][j] = array[4 * j + i];

  return state;
}
