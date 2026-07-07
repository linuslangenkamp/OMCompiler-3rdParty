#ifndef UMSOL_MATRIX_H
#define UMSOL_MATRIX_H

#include "umsol/types.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef enum UmsolMatrixKind
{
  UMSOL_MATRIX_NONE = 0,
  UMSOL_MATRIX_DENSE_COLUMN_MAJOR = 1,
  UMSOL_MATRIX_SPARSE_CSC = 2
} UmsolMatrixKind;

typedef struct UmsolMatrix
{
  UmsolMatrixKind kind;
  UmsolInt rows;
  UmsolInt cols;

  UmsolReal *x;
  UmsolInt ld;

  UmsolInt nnz;
  UmsolInt *col_ptr;
  UmsolInt *row_idx;
} UmsolMatrix;

#ifdef __cplusplus
}
#endif

#endif
