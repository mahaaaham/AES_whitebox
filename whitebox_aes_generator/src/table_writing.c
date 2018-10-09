#include "table_writing.h"

#include "inner_function.h"
#include "key.h"
#include "sbox.h"

#include <stdio.h>
#include <stdlib.h>

#define MAX_BYTE 256

static const word_t m_nibble_in_word[8] =
  { 0x0000000f, 0x000000f0, 0x00000f00, 0x0000f000,
  0x000f0000, 0x00f00000, 0x0f000000, 0xf0000000 };

/* convert a word_t WORD into 8 byte_t NIBBLES */
static void
word_to_nibbles (word_t word, nibble_t *nibbles)
{
  for (int i = 0; i < 8; ++i)
    nibbles[i] = (byte_t)((word & m_nibble_in_word[i]) >> (4 * i));
  return;
}

/* convert a byte_t BYTE into 2 byte_t NIBBLES */
static void
byte_to_nibbles (byte_t byte, nibble_t *nibbles)
{
  for (int i = 0; i < 2; ++i)
    nibbles[i] = (byte_t)((byte & m_nibble_in_word[i]) >> (4 * i));
  return;
}

/* convert 2 nibble_t NIBBLES into a byte_t BYTE */
static void
nibbles_to_byte (nibble_t *nibbles, byte_t *byte)
{
  *byte = nibbles[0] | (nibbles[1] << 4);
  return;
}

/* convert 8 nibble_t NIBBLES into a byte_t BYTE */
static void
nibbles_to_word (nibble_t *nibbles, word_t *word)
{
  word_t result = 0;
  for (int i = 0; i < 8; ++i)
    result |= nibbles[i] << (4 * i);
  *word = result;
  return;
}

/* Evaluate the plug in the value value. Recall
   that a byte_plug_t element is the concatenation of 
   2 nibble_t arrays. */
static byte_t
evaluate_byte_plug (byte_plug_t plug, byte_t value)
{
  nibble_t nibble_value[2];

  byte_to_nibbles (value, nibble_value);
  nibble_value[0] = plug[0][nibble_value[0]];
  nibble_value[1] = plug[1][nibble_value[1]];
  nibbles_to_byte (nibble_value, &value);

  return value;
}

/* Evaluate the plug in the value value. Recall
   that a word_plug_t element is the concatenation of 
   8 nibble_t arrays. */
static word_t
evaluate_word_plug (word_plug_t plug, word_t value)
{
  nibble_t nibble_value[8];

  word_to_nibbles (value, nibble_value);
  for (int i = 0; i < 8; ++i)
    nibble_value[i] = plug[i][nibble_value[i]];
  nibbles_to_word (nibble_value, &value);

  return value;
}

/* Purely psychological.. To keep a conversion with evaluate_word_plug
   and evaluate_byte_plug */
static byte_t
evaluate_nibble_plug (nibble_plug_t plug, nibble_t value)
{
  return plug[value];
}

/* exported functions: */

void
create_nibble_array (char *name, int nb_array, byte_t *shift_key,
		     byte_plug_t in[], nibble_plug_t out[],
		     nibble_t (*function) (byte_t, byte_t, byte_t *),
		     FILE * fd)
{
  int nb_row_by_array = 32;
  int nb_elt_by_row = MAX_BYTE / nb_row_by_array;
  byte_t element;
  nibble_t image;

  fprintf (fd, "byte_t %s[%u][256] = {"
	   "                         /* %s */\n", name, nb_array, name);
  for (int indice = 0; indice < nb_array; ++indice)
    {
      fprintf (fd, "/* %s[%d] */\n", name, indice);
      fprintf (fd, "{ ");
      for (int i = 0; i < nb_row_by_array; ++i)
	{
	  for (int j = 0; j < nb_elt_by_row - 1; ++j)
	    {
	      element = i * nb_elt_by_row + j;

	      element = evaluate_byte_plug (in[indice], element);
	      image = function (indice, element, shift_key);
	      image = evaluate_nibble_plug (out[indice], image);

	      fprintf (fd, "0x%02hhx, ", image);
	    }

	  element = i * nb_elt_by_row + (nb_elt_by_row - 1);

	  element = evaluate_byte_plug (in[indice], element);
	  image = function (indice, element, shift_key);
	  image = evaluate_nibble_plug (out[indice], image);

	  if (i < nb_row_by_array - 1)
	    fprintf (fd, "0x%02hhx,\n", image);
	  else
	    fprintf (fd, "0x%02hhx }", image);
	}
      if (indice == nb_array - 1)
	fprintf (fd, "\n};\n");
      else
	fprintf (fd, ",\n");
    }
  fprintf (fd, "\n");
  return;
}

void
create_byte_array (char *name, int nb_array, byte_t *shift_key,
		   byte_plug_t in[], byte_plug_t out[],
		   byte_t (*function) (byte_t, byte_t, byte_t *), FILE * fd)
{
  int nb_row_by_array = 32;
  int nb_elt_by_row = MAX_BYTE / nb_row_by_array;
  byte_t element, image;

  fprintf (fd, "byte_t %s[%u][256] = {"
	   "                         /* %s */\n", name, nb_array, name);
  for (int indice = 0; indice < nb_array; ++indice)
    {
      fprintf (fd, "/* %s[%d] */\n", name, indice);
      fprintf (fd, "{ ");
      for (int i = 0; i < nb_row_by_array; ++i)
	{
	  for (int j = 0; j < nb_elt_by_row - 1; ++j)
	    {
	      element = i * nb_elt_by_row + j;

	      image = function (indice, element, shift_key);

	      element = evaluate_byte_plug (in[indice], element);
	      image = function (indice, element, shift_key);
	      image = evaluate_byte_plug (out[indice], image);

	      fprintf (fd, "0x%02hhx, ", image);
	    }
	  element = i * nb_elt_by_row + (nb_elt_by_row - 1);

	  element = evaluate_byte_plug (in[indice], element);
	  image = function (indice, element, shift_key);
	  image = evaluate_byte_plug (out[indice], image);

	  if (i < nb_row_by_array - 1)
	    fprintf (fd, "0x%02hhx,\n", image);
	  else
	    fprintf (fd, "0x%02hhx }", image);
	}
      if (indice == nb_array - 1)
	fprintf (fd, "\n};\n");
      else
	fprintf (fd, ",\n");
    }
  fprintf (fd, "\n");
  return;
}

void
create_word_array (char *name, int nb_array, byte_t *shift_key,
		   byte_plug_t in[], word_plug_t out[],
		   word_t (*function) (byte_t, byte_t, byte_t *), FILE * fd)
{
  int nb_row_by_array = 64;
  int nb_elt_by_row = MAX_BYTE / nb_row_by_array;
  byte_t element;
  word_t image;

  fprintf (fd, "word_t %s[%u][256] = {"
	   "                          /* %s */\n", name, nb_array, name);
  for (int indice = 0; indice < nb_array; ++indice)
    {
      fprintf (fd, "/* %s[%d] */\n", name, indice);
      fprintf (fd, "{ ");
      for (int i = 0; i < nb_row_by_array; ++i)
	{
	  for (int j = 0; j < nb_elt_by_row - 1; ++j)
	    {
	      element = i * nb_elt_by_row + j;

	      element = evaluate_byte_plug (in[indice], element);
	      image = function (indice, element, shift_key);
	      image = evaluate_word_plug (out[indice], image);

	      fprintf (fd, "0x%08x, ", image);
	    }

	  element = i * nb_elt_by_row + (nb_elt_by_row - 1);

	  element = evaluate_byte_plug (in[indice], element);
	  image = function (indice, element, shift_key);
	  image = evaluate_word_plug (out[indice], image);

	  if (i < nb_row_by_array - 1)
	    fprintf (fd, "0x%08x,\n", image);
	  else
	    fprintf (fd, "0x%08x }", image);
	}
      if (indice == nb_array - 1)
	fprintf (fd, "\n};\n");
      else
	fprintf (fd, ",\n");
    }
  fprintf (fd, "\n");
  return;
}
