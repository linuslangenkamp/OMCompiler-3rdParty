#ifndef UMSOL_TYPES_H
#define UMSOL_TYPES_H

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef UMSOL_INT_TYPE
#define UMSOL_INT_TYPE int
#endif

#ifndef UMSOL_REAL_TYPE
#define UMSOL_REAL_TYPE double
#endif

typedef UMSOL_INT_TYPE UmsolInt;
typedef UMSOL_REAL_TYPE UmsolReal;

typedef enum UmsolBool
{
  UMSOL_FALSE = 0,
  UMSOL_TRUE = 1
} UmsolBool;

#ifdef __cplusplus
}
#endif

#endif
