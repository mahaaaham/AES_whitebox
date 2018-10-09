#include "block.h"
#include "define.h"
#include "key.h"
#include "sbox.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include <ctype.h>
#include <getopt.h>
#include <string.h>


#define BINARY (strrchr (argv[0], '/') + 1)

#define EXIT(message, ...)						\
  do									\
    {									\
      fprintf (stderr, "%s: " message ".\n", BINARY, __VA_ARGS__);	\
      exit (EXIT_FAILURE);						\
    }									\
  while (0)

    
bool verbose = false;


int
main (int argc, char **argv)
{
  struct option long_options[] =
    {
      { "file-format", required_argument, NULL, 'f' },
      { "help", no_argument, NULL, 'h' },
      { "inverse", no_argument, NULL, 'i' },
      { "mode", required_argument, NULL, 'm' },
      { "output", required_argument, NULL, 'o' },
      { "verbose", no_argument, NULL, 'v' },
      { NULL, 0, NULL, 0 }
    };

  char opt;
  bool inverse = false; /* Determines which cipher to use */
  bool output = false; /* Determines wheter an output file has been specified */
  char *filename = NULL; /* output file name */
  block_t block = CBC; /* Default block cipher mode of operation : CBC */
  format_t format = ascii; /* Default input file format */

  while ((opt = getopt_long (argc , argv,
			     "f:him:o:v", long_options, NULL)) != -1)
    switch (opt) 
      {
      case 'f':
	if (!strcmp (optarg, "ascii"))
	  format = ascii;
	else if (!strcmp (optarg, "hex"))
	  format = hex;
	else
	  EXIT ("%s", "file format not supported");
	break;
	
      case 'h':
	printf ("Usage: %s [-f] [-h] [-i] [-m] [-o] [-v] KEY INPUT\n"
		"Use an AES cipher to crypt or decrypt INPUT with KEY.\n\n"
		"  -f, --file-format=FORMAT\ttreat INPUT as a FORMAT file: "
		"hex and ascii (default) supported\n"
		"  -h, --help\t\t\tdisplay this help\n"
		"  -i, --inverse\t\t\tuse the inverse cipher (decrypt)\n"
		"  -m, --mode=MODE\t\tuse block cipher mode of operation: "
		"ecb and cbc (default) supported\n"
		"  -o, --output=FILE\t\tuse FILE as the output file\n"
		"  -v, --verbose\t\t\tactivate verbose mode\n", BINARY);
	exit (EXIT_SUCCESS);
	  
      case 'i':
	inverse = true;
	break;

      case 'm':
	if (!strcmp (optarg, "ecb"))
	  block = ECB;
	else if (!strcmp (optarg, "cbc"))
	  block = CBC;
	else
	  EXIT ("%s", "block cipher mode of operation is not valid");
	break;
	
      case 'o':
	output = true;
	filename = optarg;
	break;
	  
      case 'v':
	printf ("Verbose mode activated.\n");
	verbose = true;
	break;
	
      default:
	exit (EXIT_FAILURE);
      }

  /* Check argument count */
  if (argc - optind != 2)
    EXIT ("%s", "two arguments expected");
  
  /* Open key file */
  FILE *key_file = fopen (argv[optind], "r");
  if (!key_file)
    EXIT ("cannot open key file '%s'", argv[optind]);
  
  /* Open input file */
  FILE *input_file = fopen (argv[optind + 1], "r");
  if (!input_file)
    EXIT ("cannot open input file '%s'", argv[optind + 1]);

  /* Determine output file */
  if (!output)
    {
      filename = malloc (strlen (argv[optind + 1]) + 5);
      if (!filename)
	EXIT ("%s", "cannot allocate memory for output filename");
      sprintf (filename, "%s.aes", argv[optind + 1]);
    }

  /* Check for file overlap */
  if (!strcmp (argv[optind], filename))
    EXIT ("%s", "key and output files are identical");
  
  if (!strcmp (argv[optind + 1], filename))
    EXIT ("%s", "input and output files are identical");
  
  /* Open output file */
  FILE *output_file = fopen (filename, "w");
  if (!output_file)
    EXIT ("cannot open output file '%s'", filename);

  if (!output)
    free (filename);

  /* Encipher or decipher */
  if (inverse)
    {
      if (decipher (output_file, input_file, key_file, format, block) == -1)
	EXIT ("could not decipher %s", argv[optind + 1]);
    }
  else
    {
      if (encipher (output_file, input_file, key_file, format, block) == -1)
	EXIT ("could not encipher %s", argv[optind + 1]);
    }
  
  /* Clean up */
  fclose (key_file);
  fclose (input_file);
  fclose (output_file);
  
  return EXIT_SUCCESS;
}
