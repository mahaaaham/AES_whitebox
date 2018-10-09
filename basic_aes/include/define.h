#ifndef DEFINE_H
#define DEFINE_H


#include <stdint.h>


#define NB 4   /* Number of columns in the state */
#define NK 4   /* Number of words in the key */
#define NR 10  /* Number of rounds */
#define WORD 4 /* A word is 4 bytes */


typedef uint8_t byte_t;

typedef enum { ascii, hex } format_t;

/* Mode of operation is either electronic codebook (ECB)
 * or cipher block chaining (CBC) */
typedef enum { ECB, CBC } block_t;


#endif /* DEFINE_H */
