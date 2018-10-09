#ifndef TEST_H
#define TEST_H

/* Some internal test used to be sure the fonctions defined 
   really do what we want. */

/* For nb_test times, create a random inv_matrix_t
   M with inv_matrix_random and: 
   - evaluate M.in on WORD;
   - M.out the previous result: 
   - checks that the result if WORD;
   */
void matrix_word_test (word_t word, size_t nb_test);

/* For nb_test times, create a random inv_matrix_t
   M with inv_matrix_random and: 
   - evaluate M.in on BYTE;
   - M.out the previous result: 
   - checks that the result is BYTE;
   */
void matrix_byte_test (byte_t byte, size_t nb_test);

/* Concatenate 4 random matrix and show the result. */
void matrix_concatenate_test (char mode);


#endif /* TEST_H */
