#include "conversion.h" 

#include "sbox.h"
#include "tables.h"

#include <fcntl.h>  /* to use random_nibble () */
#include <getopt.h>
#include <string.h>
#include <unistd.h>

#include <stdio.h>
#include <stdlib.h>

word_t
random_word ()
{
  word_t buffer;
  int fd = open("/dev/urandom", O_RDONLY);
  read (fd, &buffer, 4);
  close (fd);
  return buffer;
}

word_t
compose_boxes (word_t image, int col)
{
  byte_t *state = malloc (4 * sizeof (byte_t));
  word_to_bytes (image, state);

  int round = 0;
  word_t column[4];
  byte_t nibble[4][8];
  byte_t xor_result_left[8], xor_result_right[8], xor_result_medium[8];

  column[0] = A[4 * round + col][state[0]];
  column[1] = B[4 * round + col][state[1]];
  column[2] = C[4 * round + col][state[2]];
  column[3] = D[4 * round + col][state[3]];

  for (int i = 0; i < 4; ++i)
    word_to_nibbles (column[i], nibble[i]);

  for (int e = 0; e < 8; e++)
    {
      xor_result_left[e] = LXOR1[32 * round + 8 * col + e]
                                [nibble[0][e] ^ (nibble[1][e] << 4 )];
      xor_result_right[e] = RXOR1[32 * round + 8 * col + e]
                                 [nibble[2][e] ^ (nibble[3][e] << 4)];
      xor_result_medium[e] = MXOR1[32 * round + 8 * col + e]
                                  [xor_result_left[e] ^ (xor_result_right[e] 
							 << 4)];
    }

  /* Not optimized at at all.. */
  nibbles_to_bytes (xor_result_medium, state); 

  /* pass to W X Y Z */
  column[0] = W[4 * round + col][state[0]];
  column[1] = X[4 * round + col][state[1]];
  column[2] = Y[4 * round + col][state[2]];
  column[3] = Z[4 * round + col][state[3]];

  for (int i = 0; i < 4; ++i)
    word_to_nibbles (column[i], nibble[i]);

  for (int e = 0; e < 8; e++)
    {
      xor_result_left[e] = LXOR2[32 * round + 8 * col + e]
                                [nibble[0][e] ^ (nibble[1][e] << 4 )];
      xor_result_right[e] = RXOR2[32 * round + 8 * col + e]
                                 [nibble[2][e] ^ (nibble[3][e] << 4)];
      xor_result_medium[e] = MXOR2[32 * round + 8 * col + e]
                                  [xor_result_left[e] ^ (xor_result_right[e] 
						         << 4)];
    }

  free (state);

  word_t result = nibbles_to_word (xor_result_medium);
  return result;
}

int
main ()
{
  word_t val1;
  word_t val2;
  word_t image1;
  word_t image2;
  
  int egality_count;
  int difference;
  unsigned int try = 0;

  for (int col = 0; col < 4; ++col)
    {
      try = 0;
      while (1)
	{
	  egality_count = 0; /* number of byte where there will be equality 
				between the transformation of 
				val1 and of val2 */
	  val1 = random_word ();
	  val2 = random_word ();

	  if (((m_byte_in_word[0] & val1) != (m_byte_in_word[0] & val2)) &&
	      ((m_byte_in_word[1] & val1) != (m_byte_in_word[1] & val2)) &&
	      ((m_byte_in_word[2] & val1) != (m_byte_in_word[2] & val2)) &&
	      ((m_byte_in_word[3] & val1) != (m_byte_in_word[3] & val2)))
	    {

	      image1 = compose_boxes (val1, col);
	      image2 = compose_boxes (val2, col);

	      for (int i = 0; i < 4; ++i)
		{
		  if ((m_byte_in_word[i] & image1) == (m_byte_in_word[i] 
						       & image2)) 
		    ++egality_count;
		  else 
		    difference = i;
		}
	      if (egality_count == 3)
		{
		  printf ("col = %d\n"
			  "collision with: val1 = %x, val2 = %x\n" 
			  "image1 = %x and image2 = %x" 
			  "at try %d\n", col, val1, val2, image1, image2, try);
		  printf ("difference in the %d element.\n", difference);
		  break;
		}
	      ++try;
	    }
	}
    }

  return EXIT_SUCCESS;
}
