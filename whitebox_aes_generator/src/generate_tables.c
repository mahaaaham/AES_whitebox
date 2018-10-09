#include "inner_function.h"
#include "key.h"
#include "matrix.h"
#include "sbox.h"
#include "table_writing.h"
#include "test.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include <fcntl.h>	/* to use random_nibble () */
#include <getopt.h>
#include <string.h>
#include <unistd.h>

#define TABLES_H_PATH "../whitebox_aes/include/tables.h"

#define WORD 4

#define A "l_a"
#define B "l_b"
#define C "l_c"
#define D "l_d"
#define W "l_w"
#define X "l_x"
#define Y "l_y"
#define Z "l_z"
#define TBOX "l_tbox"
#define LXOR1 "l_lxor1"
#define RXOR1 "l_rxor1"
#define MXOR1 "l_mxor1"
#define LXOR2 "l_lxor2"
#define RXOR2 "l_rxor2"
#define MXOR2 "l_mxor2"

#define NB_MIX 36
#define NB_TBOX 16
#define NB_XOR 288

#define BINARY (strrchr (argv[0], '/') + 1)

#define EXIT(message, ...)						\
  do									\
    {									\
      fprintf (stderr, "%s: " message ".\n", BINARY, __VA_ARGS__);	\
      exit (EXIT_FAILURE);						\
    }									\
  while (0)



/* create a the bijection:
   u ---> u xor VALUE */
void xor_bijection (bijection_t *bijection, nibble_t value)
{
  for (int i = 0; i < 16; i++)
    {
      bijection->in[i] = i ^ value;
      bijection->out[i] = i ^ value;
    }
  return;
}

nibble_t
random_nibble ()
{
  nibble_t buffer;
  int fd = open ("/dev/urandom", O_RDONLY);
  read (fd, &buffer, 1);
  close (fd);
  buffer = buffer >> 4;
  return buffer;
}


void
init_header (FILE * fd)
{
  fprintf (fd, "#ifndef TABLES_H\n");
  fprintf (fd, "#define TABLES_H\n\n");
  fprintf (fd, "#include <stdint.h>\n\n");
  fprintf (fd, "/*Lookup tables used by the whitebox AES algorithm.*/\n");
  fprintf (fd, "#define A %s\n", A);
  fprintf (fd, "#define B %s\n", B);
  fprintf (fd, "#define C %s\n", C);
  fprintf (fd, "#define D %s\n", D);
  fprintf (fd, "#define W %s\n", W);
  fprintf (fd, "#define X %s\n", X);
  fprintf (fd, "#define Y %s\n", Y);
  fprintf (fd, "#define Z %s\n", Z);
  fprintf (fd, "#define TBOX %s\n", TBOX);
  fprintf (fd, "#define LXOR1 %s\n", LXOR1);
  fprintf (fd, "#define RXOR1 %s\n", RXOR1);
  fprintf (fd, "#define MXOR1 %s\n\n", MXOR1);
  fprintf (fd, "#define LXOR2 %s\n", LXOR2);
  fprintf (fd, "#define RXOR2 %s\n", RXOR2);
  fprintf (fd, "#define MXOR2 %s\n\n", MXOR2);

  fprintf (fd, "typedef uint8_t nibble_t;\n");
  fprintf (fd, "typedef uint8_t byte_t; \n");
  fprintf (fd, "typedef uint32_t word_t; \n\n");

  fprintf (fd, "\n\n");
}

void
close_header (FILE * fd)
{
  fprintf (fd, "\n#endif /* TABLES_H */");
  return;
}

/* create a bijection and give on side to IN and the another to OUT */
void
branch_nibble (nibble_plug_t out, nibble_plug_t in, bool activated)
{
  bijection_t bij;
  nibble_t vector_bij = 0;
  if (activated)
    vector_bij = random_nibble ();

  xor_bijection (&bij, vector_bij);

  for (int variable = 0; variable < 16; ++variable)
    {
      in[variable] = bij.in[variable];
      out[variable] = bij.out[variable];
    }
  return;
}

/* a slight complexification of branch_nibble, as showed by the diagram: */
//          out_1      out_2
//            |          |
//            ------------
//                 |
//          (out_1 | out_2)
void
branch_nibbles_byte (nibble_plug_t out_1, nibble_plug_t out_2, byte_plug_t in,
		     bool activated)
{
  bijection_t bij[2];
  nibble_t vector_bij[2] = { 0, 0 };
  if (activated)
    {
      vector_bij[0] = random_nibble ();
      vector_bij[1] = random_nibble ();
    }

  xor_bijection (&bij[0], vector_bij[0]);
  xor_bijection (&bij[1], vector_bij[1]);

  for (int variable = 0; variable < 16; ++variable)
    {
      out_1[variable] = bij[0].out[variable];
      in[0][variable] = bij[0].in[variable];

      out_2[variable] = bij[1].out[variable];
      in[1][variable] = bij[1].in[variable];
    }
  return;
}


/* create one bijection between bytes (concatenation of 
   2 bijection_t) and give on side to IN and the another to OUT */
void
branch_byte (byte_plug_t out, byte_plug_t in, bool activated)
{
  bijection_t bij[2];
  nibble_t vector_bij[2] = { 0 };
  if (activated)
    {
      vector_bij[0] = random_nibble ();
      vector_bij[1] = random_nibble ();
    }
  xor_bijection (&bij[0], vector_bij[0]);
  xor_bijection (&bij[1], vector_bij[1]);

  for (int variable = 0; variable < 16; ++variable)
    {
      in[0][variable] = bij[0].in[variable];
      in[1][variable] = bij[1].in[variable];

      out[0][variable] = bij[0].out[variable];
      out[1][variable] = bij[1].out[variable];
    }
  return;
}


/* create one bijection between words (concatenation of 
   8 bijection_t) and give on side to IN and the another to OUT */
void
branch_word (word_plug_t out, word_plug_t in, bool activated)
{
  bijection_t bij[8];
  nibble_t vector_bij[8] = { 0 };
  if (activated)
    for (int i = 0; i < 8; ++i)
      vector_bij[i] = random_nibble ();

  for (int i = 0; i < 8; ++i)
    xor_bijection (&bij[i], vector_bij[i]);

  for (int variable = 0; variable < 16; ++variable)
    for (int plug = 0; plug < 8; ++plug)
      {
	in[plug][variable] = bij[plug].in[variable];
	out[plug][variable] = bij[plug].out[variable];
      }
  return;
}


int
main (int argc, char *argv[])
{
  struct option long_options[] = {
    {"encodings", no_argument, NULL, 'e'},
    {"help", no_argument, NULL, 'h'},
    {"mixing-bijections", no_argument, NULL, 'm'},
    {"zero-protection", no_argument, NULL, 'z'},
    {NULL, 0, NULL, 0}
  };

  /* determines if encodings and mixing bijections are applied or not */
  bool encodings = true;
  bool mixing = true;

  char opt;
  
  while ((opt = getopt_long (argc, argv, "ehmz", long_options, NULL)) != -1)
    switch (opt)
      {
      case 'e':		/* encodings only unless -e -m */
	if (!encodings)
	  encodings = true;
	else
	  mixing = false;
	break;

      case 'h':
	printf ("Usage: %s [-e] [-h] [-m] [-z] KEY\n"
		"Generate tables from KEY for a whitebox aes.\n\n"
		"Default implementation uses both encodings and "
		"mixing bijections (as -em).\n\n"
		"  -e, --encodings\t\tuse encodings only "
		"(no mixing bijections) to protect tables\n"
		"  -h, --help\t\t\tdisplay this help\n"
		"  -m, --mixing-bijections\tuse mixing bijections only "
		"(no encodings) to protect tables\n"
		"  -z, --zero-protection\t\tdo not protect tables "
		"(no encodings, no mixing bijections)\n", BINARY);
	exit (EXIT_SUCCESS);

      case 'm':		/* mixing bijections only unless -e -m */
	if (!mixing)
	  mixing = true;
	else
	  encodings = false;
	break;

      case 'z':
	encodings = false;
	mixing = false;
	break;

      default:
	exit (EXIT_FAILURE);
      }

  /* Check argument count */
  if (argc == optind)
    EXIT ("%s", "key file argument expected");
  else if (argc - optind > 1)
    EXIT ("%s", "too many arguments");

  FILE *key_file = fopen (argv[optind], "r");
  if (!key_file)
    EXIT ("cannot open key file '%s'", argv[optind]);

  byte_t *key = key_alloc ();
  if (!key)
    EXIT ("%s", "cannot allocate memory for the key");

  int key_len = key_read (key, key_file);
  if (key_len != 4 * NB)
    EXIT ("%s", "wrong key length");

  fclose (key_file);

  byte_t *shift_key = key_expansion (key);
  if (!shift_key)
    EXIT ("%s", "cannot expand key");
  free (key);
  key_schedule_shift (shift_key);

  /* Determine relative path of tables.h
   * Note it is relative to this executable's directory,
   * not to the current working directory */
  size_t len = strlen (argv[0]) - strlen (strrchr (argv[0], '/')) + 2;
  char *tables_path = malloc (len + strlen (TABLES_H_PATH));
  if (!tables_path)
    EXIT ("%s", "cannot allocate memory for the path string of tables.h");
  snprintf (tables_path, len, "%s", argv[0]);
  strcat (tables_path, TABLES_H_PATH);

  FILE *fd = fopen (tables_path, "w");
  if (!fd)
    EXIT ("%s", "cannot open lookup tables file");

  free (tables_path);

  /* ----------------------------------------------------- */
  /* creation of mixing bijections (defined in function.c) */
  /* ----------------------------------------------------- */
  /* inverse of the permutation used by in shift_row */
  int inv_shift[16] = { 0, 13, 10, 7,
    4, 1, 14, 11,
    8, 5, 2, 15,
    12, 9, 6, 3
  };

  for (int i = 0; i < 4 * 9; ++i)
    {
      mixing_bijection_1[i] = inv_matrix_init (32);
      if (mixing_bijection_1[i] == NULL)
	EXIT ("%s", "problem with initialisation "
	      "of matrix_t * element of size 32");

      inv_matrix_random (mixing_bijection_1[i], mixing);

      mixing_bijection_2_concat[i] = matrix_init (32);	/* for rounds 2 to 10 */
      if (mixing_bijection_2_concat[i] == NULL)
	EXIT ("%s", "problem with initialisation "
	      "of matrix_t * element of size 32");
    }

  for (int i = 0; i < 9; ++i)
    for (int j = 0; j < 16; ++j)
      {
	mixing_bijection_2[i][j] = inv_matrix_init (8);
	if (mixing_bijection_2[i][j] == NULL)
	  EXIT ("%s", "problem with initialisation "
		"of matrix_t * element of size 8");
	inv_matrix_random (mixing_bijection_2[i][j], mixing);
      }

  for (int i = 0; i < 9; ++i)
    for (int j = 0; j < 4; ++j)
      matrix_concatenate (mixing_bijection_2[i][inv_shift[4 * j]]->out,
			  mixing_bijection_2[i][inv_shift[4 * j + 1]]->out,
			  mixing_bijection_2[i][inv_shift[4 * j + 2]]->out,
			  mixing_bijection_2[i][inv_shift[4 * j + 3]]->out,
			  mixing_bijection_2_concat[4 * i + j]);


  /* ----------------------------------------- */
  /* variables definition to create encodings  */
  /* ----------------------------------------- */

  /* nibble_plug */
  nibble_plug_t OLXOR1[NB_XOR];
  nibble_plug_t ORXOR1[NB_XOR];
  nibble_plug_t OMXOR1[NB_XOR];

  nibble_plug_t OLXOR2[NB_XOR];
  nibble_plug_t ORXOR2[NB_XOR];
  nibble_plug_t OMXOR2[NB_XOR];

  /* byte_plug */
  byte_plug_t IA[NB_MIX];
  byte_plug_t IB[NB_MIX];
  byte_plug_t IC[NB_MIX];
  byte_plug_t ID[NB_MIX];

  byte_plug_t IX[NB_MIX];
  byte_plug_t IY[NB_MIX];
  byte_plug_t IZ[NB_MIX];
  byte_plug_t IW[NB_MIX];

  byte_plug_t ILXOR1[NB_XOR];
  byte_plug_t IRXOR1[NB_XOR];
  byte_plug_t IMXOR1[NB_XOR];
  byte_plug_t ILXOR2[NB_XOR];
  byte_plug_t IRXOR2[NB_XOR];
  byte_plug_t IMXOR2[NB_XOR];

  byte_plug_t ITBOX[NB_TBOX];
  byte_plug_t OTBOX[NB_TBOX];

  /* word_plug */
  word_plug_t OA[NB_MIX];
  word_plug_t OB[NB_MIX];
  word_plug_t OC[NB_MIX];
  word_plug_t OD[NB_MIX];

  word_plug_t OX[NB_MIX];
  word_plug_t OY[NB_MIX];
  word_plug_t OZ[NB_MIX];
  word_plug_t OW[NB_MIX];


  /* ----------------------------- */
  /* creation of encodings  */
  /* ----------------------------- */

  /*  External encodings */
  for (int variable = 0; variable < 16; ++variable)
    {
      for (int i = 0; i < 4; ++i)
	for (int plug = 0; plug < 2; ++plug)
	  {
	    IA[i][plug][variable] = variable;
	    IB[i][plug][variable] = variable;
	    IC[i][plug][variable] = variable;
	    ID[i][plug][variable] = variable;
	  }

      for (int i = 0; i < NB_TBOX; ++i)
	for (int plug = 0; plug < 2; ++plug)
	  OTBOX[i][plug][variable] = variable;	/* external encoding */
    }

  /* Internal encodings */
  for (int i = 0; i < NB_XOR; ++i)
    {
      branch_nibble (OLXOR1[i], IMXOR1[i][0], encodings);
      branch_nibble (ORXOR1[i], IMXOR1[i][1], encodings);
      branch_nibble (OLXOR2[i], IMXOR2[i][0], encodings);
      branch_nibble (ORXOR2[i], IMXOR2[i][1], encodings);
    }

  for (int i = 0; i < NB_MIX; ++i)
    for (int plug = 0; plug < 8; ++plug)
      {
	branch_nibbles_byte (OW[i][plug], OX[i][plug],
			     ILXOR2[i * 8 + plug], encodings);
	branch_nibbles_byte (OY[i][plug], OZ[i][plug],
			     IRXOR2[i * 8 + plug], encodings);
	branch_nibbles_byte (OA[i][plug], OB[i][plug],
			     ILXOR1[i * 8 + plug], encodings);
	branch_nibbles_byte (OC[i][plug], OD[i][plug],
			     IRXOR1[i * 8 + plug], encodings);
      }

  int shift[4][4];
  for (int col = 0; col < 4; ++col)
    {
      shift[0][col] = col;
      shift[1][col] = (col + 3) % 4;
      shift[2][col] = (col + 2) % 4;
      shift[3][col] = (col + 1) % 4;
    }

  for (int round = 0; round < 8; ++round)	/* I and O for all XOR */
    /* the last round, OMXOR1 is relied to ITBOX */
    for (int col = 0; col < 4; ++col)
      {
	branch_byte (((byte_plug_t *)OMXOR2)[round * 16 + col * 4],
		     IA[round * 4 + shift[0][col] + 4], encodings);
	branch_byte (((byte_plug_t *)OMXOR2)[round * 16 + col * 4 + 1],
		     IB[round * 4 + shift[1][col] + 4], encodings);
	branch_byte (((byte_plug_t *)OMXOR2)[round * 16 + col * 4 + 2],
		     IC[round * 4 + shift[2][col] + 4], encodings);
	branch_byte (((byte_plug_t *)OMXOR2)[round * 16 + col * 4 + 3],
		     ID[round * 4 + shift[3][col] + 4], encodings);
      }

  for (int col = 0; col < 4; ++col)
    {
      branch_byte (((byte_plug_t *)OMXOR2)[8 * 16 + 4 * col],
		   ITBOX[4 * shift[0][col]], encodings);
      branch_byte (((byte_plug_t *)OMXOR2)[8 * 16 + 4 * col + 1],
		   ITBOX[4 * shift[1][col] + 1], encodings);
      branch_byte (((byte_plug_t *)OMXOR2)[8 * 16 + 4 * col + 2],
		   ITBOX[4 * shift[2][col] + 2], encodings);
      branch_byte (((byte_plug_t *)OMXOR2)[8 * 16 + 4 * col + 3],
		   ITBOX[4 * shift[3][col] + 3], encodings);
    }

  for (int round = 0; round < 9; ++round)
    for (int i = 0; i < 4; ++i)
      {
	branch_byte (((byte_plug_t *)OMXOR1)[16 * round + 4 * i],
		     IW[round * 4 + i], encodings);
	branch_byte (((byte_plug_t *)OMXOR1)[16 * round + 4 * i + 1],
		     IX[round * 4 + i], encodings);
	branch_byte (((byte_plug_t *)OMXOR1)[16 * round + 4 * i + 2],
		     IY[round * 4 + i], encodings);
	branch_byte (((byte_plug_t *)OMXOR1)[16 * round + 4 * i + 3],
		     IZ[round * 4 + i], encodings);
      }


  /* ----------------------------- */
  /* creation of the lookup tables */
  /* ----------------------------- */

  init_header (fd);

  /* nibble_array */
  create_nibble_array (LXOR1, NB_XOR, NULL, ILXOR1, OLXOR1, table_xor, fd);
  create_nibble_array (RXOR1, NB_XOR, NULL, IRXOR1, ORXOR1, table_xor, fd);
  create_nibble_array (MXOR1, NB_XOR, NULL, IMXOR1, OMXOR1, table_xor, fd);

  create_nibble_array (LXOR2, NB_XOR, NULL, ILXOR2, OLXOR2, table_xor, fd);
  create_nibble_array (RXOR2, NB_XOR, NULL, IRXOR2, ORXOR2, table_xor, fd);
  create_nibble_array (MXOR2, NB_XOR, NULL, IMXOR2, OMXOR2, table_xor, fd);

  /* byte_array */
  create_byte_array (TBOX, NB_TBOX, shift_key, ITBOX, OTBOX, table_tbox, fd);

  /* word_array */
  create_word_array (A, NB_MIX, shift_key, IA, OA, table_a, fd);
  create_word_array (B, NB_MIX, shift_key, IB, OB, table_b, fd);
  create_word_array (C, NB_MIX, shift_key, IC, OC, table_c, fd);
  create_word_array (D, NB_MIX, shift_key, ID, OD, table_d, fd);

  create_word_array (W, NB_MIX, shift_key, IW, OW, table_w, fd);
  create_word_array (X, NB_MIX, shift_key, IX, OX, table_x, fd);
  create_word_array (Y, NB_MIX, shift_key, IY, OY, table_y, fd);
  create_word_array (Z, NB_MIX, shift_key, IZ, OZ, table_z, fd);

  close_header (fd);

  /* -------------------------- */
  /* free the mixing bijections */
  /* -------------------------- */

  for (int i = 0; i < NB_MIX; ++i)
    {
      inv_matrix_free (mixing_bijection_1[i]);
      matrix_free (mixing_bijection_2_concat[i]);
    }
  for (int i = 0; i < 9; ++i)
    for (int j = 0; j < 16; ++j)
      inv_matrix_free (mixing_bijection_2[i][j]);

  /* ---------------------------- */
  /* free key and file descriptor */
  /* ---------------------------- */

  free (shift_key);
  fclose (fd);

  return EXIT_SUCCESS;
}
