#ifndef UMSOL_INTERNAL_RK_STEPPER_H
#define UMSOL_INTERNAL_RK_STEPPER_H

#include "umsol/problem.h"
#include "umsol/rk.h"
#include "umsol/status.h"

typedef struct UmsolRkStepWork
{
  UmsolReal *stage_y;
  UmsolReal *stage_k;
  UmsolReal *error;
} UmsolRkStepWork;

UmsolStatus umsol_rk_explicit_step(const UmsolRkMethod *method, const UmsolProblem *problem, UmsolReal t,
                                   UmsolReal h, const UmsolReal *y, UmsolReal *y_trial,
                                   UmsolRkStepWork *work);

#endif
