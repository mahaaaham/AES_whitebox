#include "signature.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <inttypes.h>

typedef uint8_t byte_t;
typedef uint8_t nibble_t;
typedef uint32_t word_t;
typedef uint64_t double_t;


int
comparaison_double (const void *a, const void *b)
{
  double_t val1 = *(double_t *)a;
  double_t val2 = *(double_t *)b;
  if (val1 == val2)
     return 0;
   return  ((val1 > val2) ? -1 : 1);
}


int
comparaison (const void *a, const void *b)
{
  return (int)(*(nibble_t *)b - *(nibble_t *)a);
}

ddouble_t  
double_frequency_signature (byte_t *array)
{
  nibble_t left[16];
  nibble_t right[16];
  for (int i = 0; i < 16; i++)
    {
      left[i] = (array[i] >> 4) & 0xF; 
      right[i] = (array[i] & 0xF);
    }

  double_t left_value = frequency_signature (left);
  double_t right_value = frequency_signature (right);
  ddouble_t result = { left_value, right_value };
  return result;
}

double_t  
frequency_signature (byte_t *array)
{
  nibble_t array_count[16] = {0};
  for (int i = 0; i < 16; i++)
    array_count[array[i]]++;
  qsort (array_count, 16, sizeof (nibble_t), comparaison);
  double_t array_value = 0;

  int counter = 0;
  while (array_count[counter] != 0)
    {
      array_value = 0x10 * array_value + array_count[counter];
      ++counter;
    }
  return array_value;
}
