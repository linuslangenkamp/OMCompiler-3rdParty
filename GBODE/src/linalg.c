#include "umsol/linalg.h"

#include <limits.h>
#include <math.h>

#if defined(UMSOL_HAVE_BLAS)
extern void dcopy_(const int *n, const double *x, const int *incx, double *y, const int *incy);
extern void dscal_(const int *n, const double *alpha, double *x, const int *incx);
extern void daxpy_(const int *n, const double *alpha, const double *x, const int *incx, double *y, const int *incy);
extern double ddot_(const int *n, const double *x, const int *incx, const double *y, const int *incy);
extern double dnrm2_(const int *n, const double *x, const int *incx);
#endif

static UmsolBool umsol_blas_int(UmsolInt value, int *out)
{
  if (value < 0 || value > (UmsolInt)INT_MAX)
  {
    return UMSOL_FALSE;
  }

  *out = (int)value;
  return UMSOL_TRUE;
}

void umsol_vec_set(UmsolInt n, UmsolReal value, UmsolReal *x, UmsolInt incx)
{
  UmsolInt i;

  if (n <= 0 || !x || incx <= 0)
  {
    return;
  }

  for (i = 0; i < n; i++)
  {
    x[i * incx] = value;
  }
}

void umsol_vec_copy(UmsolInt n, const UmsolReal *x, UmsolInt incx, UmsolReal *y, UmsolInt incy)
{
  UmsolInt i;

  if (n <= 0 || !x || !y || incx <= 0 || incy <= 0)
  {
    return;
  }

#if defined(UMSOL_HAVE_BLAS)
  {
    int bn;
    int bincx;
    int bincy;
    if (umsol_blas_int(n, &bn) && umsol_blas_int(incx, &bincx) && umsol_blas_int(incy, &bincy))
    {
      dcopy_(&bn, x, &bincx, y, &bincy);
      return;
    }
  }
#endif

  for (i = 0; i < n; i++)
  {
    y[i * incy] = x[i * incx];
  }
}

void umsol_vec_scal(UmsolInt n, UmsolReal alpha, UmsolReal *x, UmsolInt incx)
{
  UmsolInt i;

  if (n <= 0 || !x || incx <= 0)
  {
    return;
  }

#if defined(UMSOL_HAVE_BLAS)
  {
    int bn;
    int bincx;
    if (umsol_blas_int(n, &bn) && umsol_blas_int(incx, &bincx))
    {
      dscal_(&bn, &alpha, x, &bincx);
      return;
    }
  }
#endif

  for (i = 0; i < n; i++)
  {
    x[i * incx] *= alpha;
  }
}

void umsol_vec_axpy(UmsolInt n, UmsolReal alpha, const UmsolReal *x, UmsolInt incx, UmsolReal *y, UmsolInt incy)
{
  UmsolInt i;

  if (n <= 0 || !x || !y || incx <= 0 || incy <= 0)
  {
    return;
  }

#if defined(UMSOL_HAVE_BLAS)
  {
    int bn;
    int bincx;
    int bincy;
    if (umsol_blas_int(n, &bn) && umsol_blas_int(incx, &bincx) && umsol_blas_int(incy, &bincy))
    {
      daxpy_(&bn, &alpha, x, &bincx, y, &bincy);
      return;
    }
  }
#endif

  for (i = 0; i < n; i++)
  {
    y[i * incy] += alpha * x[i * incx];
  }
}

UmsolReal umsol_vec_dot(UmsolInt n, const UmsolReal *x, UmsolInt incx, const UmsolReal *y, UmsolInt incy)
{
  UmsolInt i;
  UmsolReal dot = (UmsolReal)0.0;

  if (n <= 0 || !x || !y || incx <= 0 || incy <= 0)
  {
    return dot;
  }

#if defined(UMSOL_HAVE_BLAS)
  {
    int bn;
    int bincx;
    int bincy;
    if (umsol_blas_int(n, &bn) && umsol_blas_int(incx, &bincx) && umsol_blas_int(incy, &bincy))
    {
      return (UmsolReal)ddot_(&bn, x, &bincx, y, &bincy);
    }
  }
#endif

  for (i = 0; i < n; i++)
  {
    dot += x[i * incx] * y[i * incy];
  }
  return dot;
}

UmsolReal umsol_vec_nrm2(UmsolInt n, const UmsolReal *x, UmsolInt incx)
{
  UmsolInt i;
  UmsolReal sum = (UmsolReal)0.0;

  if (n <= 0 || !x || incx <= 0)
  {
    return (UmsolReal)0.0;
  }

#if defined(UMSOL_HAVE_BLAS)
  {
    int bn;
    int bincx;
    if (umsol_blas_int(n, &bn) && umsol_blas_int(incx, &bincx))
    {
      return (UmsolReal)dnrm2_(&bn, x, &bincx);
    }
  }
#endif

  for (i = 0; i < n; i++)
  {
    const UmsolReal value = x[i * incx];
    sum += value * value;
  }
  return (UmsolReal)sqrt((double)sum);
}

UmsolReal umsol_vec_weighted_rms(UmsolInt n, const UmsolReal *x, const UmsolReal *weights)
{
  UmsolInt i;
  UmsolReal sum = (UmsolReal)0.0;

  if (n <= 0 || !x || !weights)
  {
    return (UmsolReal)0.0;
  }

  for (i = 0; i < n; i++)
  {
    const UmsolReal value = x[i] * weights[i];
    sum += value * value;
  }

  return (UmsolReal)sqrt((double)(sum / (UmsolReal)n));
}
