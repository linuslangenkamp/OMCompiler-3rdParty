#ifndef UMSOL_PROBLEM_H
#define UMSOL_PROBLEM_H

#include "umsol/matrix.h"
#include "umsol/status.h"
#include "umsol/types.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef enum UmsolProblemForm
{
  UMSOL_PROBLEM_ODE = 0,
  UMSOL_PROBLEM_MASS_MATRIX = 1
} UmsolProblemForm;

typedef struct UmsolActiveView
{
  UmsolInt n_total;
  UmsolInt n_active;
  const UmsolInt *active_to_full;
  const UmsolInt *full_to_active;
} UmsolActiveView;

typedef struct UmsolResidualRequest
{
  UmsolReal t;
  UmsolReal h;
  const UmsolReal *y_old;
  const UmsolReal *y;
  const UmsolReal *ydot;
  const UmsolActiveView *view;
  void *method_context;
} UmsolResidualRequest;

typedef struct UmsolJacobianRequest
{
  UmsolReal t;
  UmsolReal gamma;
  const UmsolReal *y;
  const UmsolReal *ydot;
  const UmsolActiveView *view;
  void *method_context;
} UmsolJacobianRequest;

typedef struct UmsolMassMatrixRequest
{
  UmsolReal t;
  const UmsolReal *y;
  const UmsolActiveView *view;
} UmsolMassMatrixRequest;

typedef struct UmsolProblemOps
{
  UmsolStatus (*rhs)(void *ctx, UmsolReal t, const UmsolReal *y, UmsolReal *f);
  UmsolStatus (*rhs_active)(void *ctx, UmsolReal t, const UmsolReal *y_full, const UmsolActiveView *view,
                            UmsolReal *f_active);
  UmsolStatus (*residual)(void *ctx, const UmsolResidualRequest *request, UmsolReal *residual);
  UmsolStatus (*jacobian)(void *ctx, const UmsolJacobianRequest *request, UmsolMatrix *jacobian);
  UmsolStatus (*mass_matrix)(void *ctx, const UmsolMassMatrixRequest *request, UmsolMatrix *mass);
  UmsolStatus (*events)(void *ctx, UmsolReal t, const UmsolReal *y, UmsolReal *z);
  UmsolStatus (*state_meta)(void *ctx, UmsolReal *nominal, UmsolReal *min, UmsolReal *max);
} UmsolProblemOps;

typedef struct UmsolProblem
{
  UmsolProblemForm form;
  UmsolInt n_states;
  UmsolInt n_events;
  void *ctx;
  UmsolProblemOps ops;
} UmsolProblem;

UmsolStatus umsol_problem_validate(const UmsolProblem *problem);

#ifdef __cplusplus
}
#endif

#endif
