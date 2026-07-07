#ifndef UMSOL_RK_H
#define UMSOL_RK_H

#include "umsol/method.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef enum UmsolRkStructure
{
  UMSOL_RK_EXPLICIT = 0,
  UMSOL_RK_DIRK = 1,
  UMSOL_RK_FIRK = 2
} UmsolRkStructure;

struct UmsolRkMethod
{
  const char *name;
  UmsolRkStructure structure;
  UmsolInt n_stages;
  UmsolInt order;
  UmsolInt embedded_order;
  UmsolReal *A;
  UmsolReal *b;
  UmsolReal *b_embedded;
  UmsolReal *c;
};

typedef struct UmsolRkMethodSpec
{
  const char *name;
  UmsolRkStructure structure;
  UmsolInt n_stages;
  UmsolInt order;
  UmsolInt embedded_order;
  const UmsolReal *A;
  const UmsolReal *b;
  const UmsolReal *b_embedded;
  const UmsolReal *c;
} UmsolRkMethodSpec;

UmsolStatus umsol_method_create_rk(const UmsolRkMethodSpec *spec, UmsolMethod **method);
UmsolStatus umsol_method_create_rk_bogacki_shampine_3_2(UmsolMethod **method);
UmsolStatus umsol_method_create_rk_classical_4(UmsolMethod **method);

#ifdef __cplusplus
}
#endif

#endif
