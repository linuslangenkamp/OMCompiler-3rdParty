#ifndef UMSOL_SOLVER_H
#define UMSOL_SOLVER_H

#include "umsol/config.h"
#include "umsol/problem.h"
#include "umsol/status.h"
#include "umsol/types.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct UmsolSolver UmsolSolver;

typedef struct UmsolStepResult
{
  UmsolStatus status;
  UmsolReal t;
  UmsolReal h_accepted;
  UmsolReal h_next;
  UmsolReal error_norm;
  UmsolInt error_order;
  UmsolInt error_kind;
  UmsolInt fallback_reason;
} UmsolStepResult;

UmsolStatus umsol_solver_create(const UmsolConfig *config, const UmsolProblem *problem, UmsolSolver **solver);
void umsol_solver_destroy(UmsolSolver *solver);
UmsolStatus umsol_solver_reinit(UmsolSolver *solver, UmsolReal t0, const UmsolReal *y0);
UmsolStatus umsol_solver_advance(UmsolSolver *solver, UmsolReal target_time, UmsolStepResult *result);
UmsolStatus umsol_solver_get_state(const UmsolSolver *solver, UmsolReal *t, UmsolReal *y);
void umsol_solver_dump_settings(const UmsolSolver *solver);

#ifdef __cplusplus
}
#endif

#endif
