#include <stdint.h>

typedef uint64_t double_t;
typedef uint32_t word_t;
typedef uint8_t byte_t;
typedef uint8_t nibble_t;

struct ddouble_t {
  double_t left;
  double_t right;
};
typedef struct ddouble_t ddouble_t;

int comparaison_double (const void *a, const void *b);

byte_t analyse_word_array (word_t *array);

/* function prototypes */

double_t frequency_signature (byte_t *array);
ddouble_t double_frequency_signature (byte_t *array);
