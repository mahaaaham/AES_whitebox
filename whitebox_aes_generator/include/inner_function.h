#ifndef INNER_FUNCTION_H
#define INNER_FUNCTION_H

#include "matrix.h"

extern inv_matrix_t *mixing_bijection_1[36];	/* for rounds 1 to 9 */
extern inv_matrix_t *mixing_bijection_2[9][4 * 4];
extern matrix_t *mixing_bijection_2_concat[36];	/* for rounds 2 to 10 */

/* Fonctions prototypes: */

nibble_t table_xor (byte_t indice, byte_t value, byte_t *unused);

byte_t table_tbox (byte_t indice, byte_t value, byte_t *shift_key);

word_t table_a (byte_t indice, byte_t value, byte_t *shift_key);
word_t table_b (byte_t indice, byte_t value, byte_t *shift_key);
word_t table_c (byte_t indice, byte_t value, byte_t *shift_key);
word_t table_d (byte_t indice, byte_t value, byte_t *shift_key);

word_t table_x (byte_t indice, byte_t value, byte_t *shift_key);
word_t table_y (byte_t indice, byte_t value, byte_t *shift_key);
word_t table_w (byte_t indice, byte_t value, byte_t *shift_key);
word_t table_z (byte_t indice, byte_t value, byte_t *shift_key);

#endif /* INNER_FUNCTION_H */
