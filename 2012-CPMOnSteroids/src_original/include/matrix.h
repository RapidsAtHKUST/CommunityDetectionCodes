#ifndef MATRIX_H
#define	MATRIX_H

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "debug.h"

extern unsigned long int WINDOW_SIZE;

enum boolean{
  TRUE=1,
  FALSE=0,
};

enum errors{
  E_ROW_OUT_OF_WINDOW=-100,
  E_COL_OUT_OF_MATRIX_BOUNDS,
  E_MALLOC,
  E_WINDOW_TOO_SMALL,
  E_NULL_POINTER,
  E_INEQ_SOLUTIONS,
};

enum info{
  I_END_OF_MATRIX_REACHED=1,
};

#ifdef	__cplusplus
extern "C" {
#endif

  struct matrix{
    uint8_t *window;
    long int window_capacity;
    long int num_rows;
    long int starts_at;
    long int ends_at;
    uint8_t **begin_of_row;
    long int matrix_diagonal_size;
  };

  typedef struct matrix matrix;

  /*
   * initialize a matrix structure and do the first slide forward.
   * So do not call slide_forward immediately after this init.
   * Specify the diagonal size of the matrix as the second argument (i.e.
   * the number of maximal cliques.
   */
  int matrix_init(matrix **m, long int matrix_size);

  /*
   * deallocate data structures previously allocated. After this call
   * the pointer m is no more significant so it is better to set it to null
   */
  int matrix_destroy(matrix *m);
    
  /*
   * slide the matrix forward.
   */
  int matrix_slide_forward(matrix *m);

  /*
   * set the element at the i-th row and j-th column to value value
   */
  int matrix_set(matrix *m, int i, int j, uint8_t value);

  /*
   * read the element at the i-th row and j-th column and place the value read
   * in the location pointed by *value
   */
  int matrix_get(const matrix *m, int i, int j, uint8_t *value);

  /*
   * returns TRUE if the j-th row is in the window. It is good to call
   * this function before inserting values with the matrix_set. In this way
   * you are sure you are inserting an element which is present
   */
  int matrix_row_in_window(const matrix *m, int i);

  /*
   * returns TRUE if the colum is in the bound of the matrix
   */
  int matrix_col_in_matrix_bounds(const matrix *m, int j);
  int matrix_get_num_rows_in_window(const matrix *m, int *rows_in_window);
#ifdef	__cplusplus
}
#endif

#endif	/* MATRIX_H */

