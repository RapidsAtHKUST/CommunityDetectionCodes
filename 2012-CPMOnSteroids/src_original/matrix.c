#include "include/matrix.h"
#include "include/debug.h"

static int matrix_ends_at(matrix *m){
	
    double delta;
    double result1,result2;
	
    // get the capacity of the matrix
    double cp = (double)m->window_capacity;
    debug((DEBUG_NORMAL, "m->window_capacity %li", (long int)cp));
	
    // the current starting index
    double sa = (double)m->starts_at;
    debug((DEBUG_FAT, "m->starts_at %li", (long int)sa));
	
    // and the total number of entries of the matrix
    double t = (double)m->matrix_diagonal_size;
    debug((DEBUG_FAT, "m->matrix_diagonal_size %li", (long int)t));
	
    if(m == NULL)
        return E_NULL_POINTER;
	
    // compute delta
    delta = -8 * cp + 4 * pow(sa, 2) + 8 * t * (1 - sa) + 4 * sa + 4 * pow(t, 2) - 12 * t + 1;
    
	if(delta < 0){
        // every x is a solution. We can accomodate every row
        // in the matrix. So set the ends_at variable to the value of
        // the last entry and return
        m->ends_at=m->matrix_diagonal_size-1;
        return 0;
    }
	
    // and get the result
    result1 = t - 1.5l - .5l * sqrt(delta);
    result2 = t - 1.5l + .5l * sqrt(delta);
    debug((DEBUG_FAT, "(double)delta = %f", delta));
    debug((DEBUG_FAT, "(double)result1 = %f", result1));
    debug((DEBUG_FAT, "(double)result2 = %f", result2));
    result1 = floor(result1);
    result2 = floor(result2);
    debug((DEBUG_NORMAL, "floor(result1)= %f", result1));
    debug((DEBUG_NORMAL, "floor(result2)= %f", result2));
    
    if(result1 >= 0 && result1 < t) {
        m->ends_at = (long int) result1;
        return 0;
    } 
	else if(result2 >=0 && result2 < t){
		
        m->ends_at = (long int) result2;
        return 0;
    } 
	else{
        // this else is reached only on anomalous contidions
        // such as matrix_diagonal_size > WINDOW_SIZE thus we have only
        // to return an error and to mark ends_at with an unsignificant value
        m->ends_at = -1;
        return E_INEQ_SOLUTIONS;
    }

}



static int matrix_offsets(matrix *m){
    
  int i;
  long int cur_index = m->starts_at;
  long int incremental_offset = 0;
  if(m == NULL)
    return E_NULL_POINTER;
  if(m->begin_of_row != NULL)
    free(m->begin_of_row);
  if((m->begin_of_row = (uint8_t**)malloc(sizeof(uint8_t*) * m->num_rows)) == NULL)
    return E_MALLOC;
  for(i=0; i<m->num_rows; i++){
    m->begin_of_row[i] = m->window + incremental_offset;
    incremental_offset += m->matrix_diagonal_size - (cur_index + 1);
    cur_index++;
  }
	
#ifdef DEBUG
  for(i=0; i<m->num_rows; i++){
    debug((DEBUG_OBESE, "Row %i starts at offset: %p", i, m->begin_of_row[i]));
  }
  debug((DEBUG_FAT, "Current Chunk: starts_at=%i ends_at=%i num_rows=%i",
         (int)m->starts_at, (int)m->ends_at, (int)m->num_rows));
#endif

  return 0;
	
}

int matrix_init(matrix **m, long int matrix_diagonal_size){
    int ret;
    if ((*m = (matrix*)malloc(sizeof(matrix))) == NULL)
        return E_MALLOC;
    (*m)->matrix_diagonal_size = matrix_diagonal_size;
    // if we cannot accomodate at least a row in the matrix
    // we have to return
    if(matrix_diagonal_size > WINDOW_SIZE)
        return E_WINDOW_TOO_SMALL;
    (*m)->window = (uint8_t*)malloc(sizeof(uint8_t)*WINDOW_SIZE);
    (*m)->starts_at = 0l;
    (*m)->window_capacity = WINDOW_SIZE / sizeof(uint8_t);
	
    if((ret = matrix_ends_at(*m)) < 0)
        return ret;
    (*m)->num_rows = (*m)->ends_at - (*m)->starts_at + 1;
	
    (*m)->begin_of_row = NULL;
    if((ret = matrix_offsets((*m))) < 0)
        return ret;
    return 0;
}

int matrix_slide_forward(matrix *m){
    int ret;
    if(m == NULL)
        return -1;
    // check if we have reached the end of the matrix
    if(m->ends_at == m->matrix_diagonal_size-1)
        return I_END_OF_MATRIX_REACHED;
	
    m->starts_at = m->ends_at + 1;
    if((ret = matrix_ends_at(m)) != 0)
        return ret;
    m->num_rows = m->ends_at - m->starts_at + 1;
	
    if((ret = matrix_offsets(m)) < 0)
        return ret;
	
    return 0;
}

int matrix_set(matrix *m, int i, int j, uint8_t value){
    long int row_offset;
    if(i<m->starts_at || i>m->ends_at)
        return E_ROW_OUT_OF_WINDOW;
    if(j<=i || j>m->matrix_diagonal_size-1)
        return E_COL_OUT_OF_MATRIX_BOUNDS;
    row_offset = j - i - 1;
    debug((DEBUG_EXPLODING, "Writing %i at address %p", value, m->begin_of_row[i - m->starts_at] + row_offset));
    *(m->begin_of_row[i - m->starts_at] + row_offset) = value;
    return 0;
}

int matrix_get(const matrix *m, int i, int j, uint8_t *value){
    long int row_offset;
    if(i<m->starts_at || i>m->ends_at)
        return E_ROW_OUT_OF_WINDOW;
    if(j<=i || j>m->matrix_diagonal_size-1)
        return E_COL_OUT_OF_MATRIX_BOUNDS;
    row_offset = j - i - 1;
    *value = *(m->begin_of_row[i - m->starts_at] + row_offset);
    debug((DEBUG_EXPLODING, "Getting value %i at address %p", *value, m->begin_of_row[i - m->starts_at] + row_offset));
    return 0;
}

int matrix_row_in_window(const matrix *m, int i){
    return (i>=m->starts_at && i<=m->ends_at) ? TRUE : FALSE;
}

int matrix_col_in_matrix_bounds(const matrix *m, int j){
    return (j>=0 && j<m->matrix_diagonal_size) ? TRUE : FALSE;
}

int matrix_get_num_rows_in_window(const matrix *m, int *rows_in_window){
  if(m == NULL)
    return E_NULL_POINTER;
  *rows_in_window = m->num_rows;
  return 0;
}

int matrix_destroy(matrix *m){
    if(m == NULL)
        return E_NULL_POINTER;
    if(m->window != NULL)
        free(m->window);
    if(m->begin_of_row != NULL)
        free(m->begin_of_row);
    return 0;
}
