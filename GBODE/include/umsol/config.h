#ifndef UMSOL_CONFIG_H
#define UMSOL_CONFIG_H

#include "umsol/logger.h"
#include "umsol/types.h"

#ifdef __cplusplus
extern "C"
{
#endif

struct UmsolMethod;

typedef enum UmsolMethodId
{
  UMSOL_METHOD_UNSET = 0,
  UMSOL_METHOD_RK_USER = 1,
  UMSOL_METHOD_BDF_USER = 2,
  UMSOL_METHOD_ROSENBROCK_USER = 3
} UmsolMethodId;

typedef enum UmsolStepperId
{
  UMSOL_STEPPER_UNSET = 0,
  UMSOL_STEPPER_RK = 1,
  UMSOL_STEPPER_BDF = 2,
  UMSOL_STEPPER_ROSENBROCK = 3
} UmsolStepperId;

typedef enum UmsolErrorId
{
  UMSOL_ERROR_NONE = 0,
  UMSOL_ERROR_EMBEDDED = 1,
  UMSOL_ERROR_TWO_STEP = 2,
  UMSOL_ERROR_CONTRACTIVE_DEFECT = 3,
  UMSOL_ERROR_CONTRACTIVE_FILTER = 4,
  UMSOL_ERROR_RICHARDSON = 5
} UmsolErrorId;

typedef enum UmsolControllerId
{
  UMSOL_CONTROLLER_CONSTANT = 0,
  UMSOL_CONTROLLER_I = 1,
  UMSOL_CONTROLLER_PI = 2,
  UMSOL_CONTROLLER_PID = 3
} UmsolControllerId;

typedef enum UmsolInterpolantId
{
  UMSOL_INTERPOLANT_NONE = 0,
  UMSOL_INTERPOLANT_LINEAR = 1,
  UMSOL_INTERPOLANT_HERMITE = 2,
  UMSOL_INTERPOLANT_DENSE_OUTPUT = 3
} UmsolInterpolantId;

typedef enum UmsolNlsId
{
  UMSOL_NLS_NONE = 0,
  UMSOL_NLS_INTERNAL_NEWTON = 1
} UmsolNlsId;

typedef enum UmsolLinearSolverId
{
  UMSOL_LINSOLVER_NONE = 0,
  UMSOL_LINSOLVER_KLU = 1,
  UMSOL_LINSOLVER_DENSE_LAPACK = 2
} UmsolLinearSolverId;

typedef enum UmsolPartitionId
{
  UMSOL_PARTITION_NONE = 0,
  UMSOL_PARTITION_ERROR_THRESHOLD = 1,
  UMSOL_PARTITION_USER_MASK = 2
} UmsolPartitionId;

typedef struct UmsolConfig
{
  const struct UmsolMethod *method_object;

  UmsolMethodId method;
  UmsolStepperId stepper;
  UmsolErrorId error;
  UmsolControllerId controller;
  UmsolInterpolantId interpolant;
  UmsolNlsId nls;
  UmsolLinearSolverId linear_solver;
  UmsolPartitionId partitioner;

  UmsolReal rel_tol;
  UmsolReal abs_tol;
  UmsolReal h_initial;
  UmsolReal h_min;
  UmsolReal h_max;
  UmsolInt max_steps;

  UmsolBool multirate_enabled;
  UmsolLogger logger;
} UmsolConfig;

void umsol_config_set_defaults(UmsolConfig *config);

#ifdef __cplusplus
}
#endif

#endif
