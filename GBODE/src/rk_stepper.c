#include "internal/rk_stepper.h"

#include "umsol/linalg.h"

UmsolStatus umsol_rk_explicit_step(const UmsolRkMethod *method, const UmsolProblem *problem, UmsolReal t,
                                   UmsolReal h, const UmsolReal *y, UmsolReal *y_trial,
                                   UmsolRkStepWork *work)
{
  const UmsolInt n = problem ? problem->n_states : 0;
  UmsolInt stage;
  UmsolInt previous;

  if (!method || !problem || !y || !y_trial || !work || !work->stage_y || !work->stage_k || !work->error)
  {
    return UMSOL_INVALID_ARGUMENT;
  }
  if (method->structure != UMSOL_RK_EXPLICIT || !problem->ops.rhs)
  {
    return UMSOL_INVALID_CONFIG;
  }

  for (stage = 0; stage < method->n_stages; stage++)
  {
    UmsolReal *stage_y = work->stage_y;
    UmsolReal *stage_k = work->stage_k + stage * n;
    UmsolStatus status;

    umsol_vec_copy(n, y, (UmsolInt)1, stage_y, (UmsolInt)1);

    for (previous = 0; previous < stage; previous++)
    {
      const UmsolReal a = method->A[stage * method->n_stages + previous];
      if (a != (UmsolReal)0.0)
      {
        umsol_vec_axpy(n, h * a, work->stage_k + previous * n, (UmsolInt)1, stage_y, (UmsolInt)1);
      }
    }

    status = problem->ops.rhs(problem->ctx, t + method->c[stage] * h, stage_y, stage_k);
    if (status != UMSOL_OK)
    {
      return status;
    }
  }

  umsol_vec_copy(n, y, (UmsolInt)1, y_trial, (UmsolInt)1);
  umsol_vec_set(n, (UmsolReal)0.0, work->error, (UmsolInt)1);

  for (stage = 0; stage < method->n_stages; stage++)
  {
    const UmsolReal b = method->b[stage];
    if (b != (UmsolReal)0.0)
    {
      umsol_vec_axpy(n, h * b, work->stage_k + stage * n, (UmsolInt)1, y_trial, (UmsolInt)1);
    }

    if (method->b_embedded)
    {
      const UmsolReal db = method->b[stage] - method->b_embedded[stage];
      if (db != (UmsolReal)0.0)
      {
        umsol_vec_axpy(n, h * db, work->stage_k + stage * n, (UmsolInt)1, work->error, (UmsolInt)1);
      }
    }
  }

  return UMSOL_OK;
}
