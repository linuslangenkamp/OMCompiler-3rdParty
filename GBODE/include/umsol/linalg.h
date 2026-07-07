#ifndef UMSOL_LINALG_H
#define UMSOL_LINALG_H

#include "umsol/types.h"

#ifdef __cplusplus
extern "C"
{
#endif

void umsol_vec_set(UmsolInt n, UmsolReal value, UmsolReal *x, UmsolInt incx);
void umsol_vec_copy(UmsolInt n, const UmsolReal *x, UmsolInt incx, UmsolReal *y, UmsolInt incy);
void umsol_vec_scal(UmsolInt n, UmsolReal alpha, UmsolReal *x, UmsolInt incx);
void umsol_vec_axpy(UmsolInt n, UmsolReal alpha, const UmsolReal *x, UmsolInt incx, UmsolReal *y, UmsolInt incy);
UmsolReal umsol_vec_dot(UmsolInt n, const UmsolReal *x, UmsolInt incx, const UmsolReal *y, UmsolInt incy);
UmsolReal umsol_vec_nrm2(UmsolInt n, const UmsolReal *x, UmsolInt incx);
UmsolReal umsol_vec_weighted_rms(UmsolInt n, const UmsolReal *x, const UmsolReal *weights);

#ifdef __cplusplus
}
#endif

#endif
