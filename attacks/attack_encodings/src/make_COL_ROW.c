#define __STDC_FORMAT_MACROS /* to write uint64, with inttypes.h */ 


#include "sbox.h"
#include "signature.h"

#include <stdio.h>
#include <stdlib.h>

#include <inttypes.h>

#define PATH_TO_COL_ROW "../include/COL_ROW.h"

static byte_t 
rijndael_mult(byte_t val_1, byte_t val_2) 
{
  uint8_t result = 0;
  while ((val_1 && val_2) != 0) 
    {
      if (val_2 & 1) 
	result ^= val_1; 

      if (val_1 & 0x80) /* GF modulo: if val_1 >= 128 */
	val_1 = (val_1 << 1) ^ 0x11b; /* XOR with the primitive polynomial 
				 x^8 + x^4 + x^3 + x + 1 (0b1_0001_1011) */ 
      else
	val_1 <<= 1;

      val_2 >>= 1; 
    } 
  return result;
}


void
make_C_R (FILE *fd)
{
  byte_t (*square_sbox)[16]  = (byte_t (*)[16])&sbox[0];

  byte_t array[16];
  ddouble_t signature;
  double_t list_of_signature[6];

  fprintf (fd, "#ifndef C_AND_R_H\n");
  fprintf (fd, "#define C_AND_R_H\n\n");
  fprintf (fd, "#include <stdint.h>\n\n");
  fprintf (fd, "typedef uint64_t double_t;\n");
  fprintf (fd, "/*Lookup tables used by the encoding attack of "
	       "AES white-box algorithm.*/\n\n"); 

  for (int which = 0; which < 2; ++which)
    {
      if (which == 0)
	fprintf (fd, "double_t ROW[16][6] =\n{\n");
      else
	fprintf (fd, "double_t COL[16][6] =\n{\n");
      for (int i = 0; i < 16; ++i)
	{
	  for (int mult = 0; mult < 3; ++mult) /* to make C */
	    {
	      for (int j = 0; j < 16; ++j) 
		{
		  switch (mult)
		    {
		    case 0:
		      if (which == 0)
			array[j] = rijndael_mult (0x01, square_sbox[i][j]);
		      else
			array[j] = rijndael_mult (0x01, square_sbox[j][i]);
		      break;
		    case 1:
		      if (which == 0)
			array[j] = rijndael_mult (0x02, square_sbox[i][j]);
		      else
			array[j] = rijndael_mult (0x02, square_sbox[j][i]);
		      break;
		    case 2:
		      if (which == 0)
			array[j] = rijndael_mult (0x03, square_sbox[i][j]);
		      else
			array[j] = rijndael_mult (0x03, square_sbox[j][i]);
		      break;
		    }
		}
	      signature = double_frequency_signature (array);
	      list_of_signature[2 * mult] = signature.left;
	      list_of_signature[2 * mult + 1] = signature.right;
	    }
	  qsort (list_of_signature, 6, sizeof (double_t), comparaison_double);
	  fprintf (fd, "  { ");
	  for (int i = 0; i < 5; ++i)
	    {
	      fprintf (fd, "%15" PRIu64 ", ", list_of_signature[i]);
	      if (i == 2)
		fprintf (fd, "\n    ");
	    }
	  fprintf (fd, "%15" PRIu64 " },\n", list_of_signature[5]);

	}
      fprintf (fd, "};\n\n");
    }

  fprintf (fd, "#endif /* C_AND_R_H */");

}

int
main ()
{
  FILE *fd = fopen (PATH_TO_COL_ROW,"w");
  if (!fd)
    exit (EXIT_FAILURE);

  make_C_R (fd);
  fclose (fd);
  return 0;
}
