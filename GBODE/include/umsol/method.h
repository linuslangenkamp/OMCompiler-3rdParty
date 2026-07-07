#ifndef UMSOL_METHOD_H
#define UMSOL_METHOD_H

#include "umsol/config.h"
#include "umsol/status.h"
#include "umsol/types.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct UmsolMethod UmsolMethod;
typedef struct UmsolMethodState UmsolMethodState;

typedef enum UmsolMethodFamily
{
  UMSOL_METHOD_FAMILY_NONE = 0,
  UMSOL_METHOD_FAMILY_RK = 1,
  UMSOL_METHOD_FAMILY_BDF = 2,
  UMSOL_METHOD_FAMILY_ROSENBROCK = 3
} UmsolMethodFamily;

typedef struct UmsolMethodOps
{
  const char *(*name)(const UmsolMethod *method);
  UmsolStatus (*init)(UmsolMethod *method, const UmsolConfig *config);
  UmsolStatus (*create_state)(const UmsolMethod *method, const UmsolConfig *config, UmsolMethodState **state);
  void (*reset_state)(const UmsolMethod *method, UmsolMethodState *state, UmsolInt reason);
  void (*destroy_state)(const UmsolMethod *method, UmsolMethodState *state);
  void (*destroy)(UmsolMethod *method);
} UmsolMethodOps;

struct UmsolMethod
{
  UmsolMethodFamily family;
  const UmsolMethodOps *ops;
  void *impl;
};

typedef struct UmsolRkMethod UmsolRkMethod;
typedef struct UmsolBdfMethod UmsolBdfMethod;
typedef struct UmsolRosenbrockMethod UmsolRosenbrockMethod;

const UmsolRkMethod *umsol_method_as_rk(const UmsolMethod *method);
const UmsolBdfMethod *umsol_method_as_bdf(const UmsolMethod *method);
const UmsolRosenbrockMethod *umsol_method_as_rosenbrock(const UmsolMethod *method);
void umsol_method_destroy(UmsolMethod *method);

#ifdef __cplusplus
}
#endif

#endif
