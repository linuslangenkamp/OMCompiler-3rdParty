#include "umsol/problem.h"

UmsolStatus umsol_problem_validate(const UmsolProblem *problem)
{
  if (!problem)
  {
    return UMSOL_INVALID_ARGUMENT;
  }
  if (problem->n_states <= 0)
  {
    return UMSOL_INVALID_CONFIG;
  }
  if (problem->n_events < 0)
  {
    return UMSOL_INVALID_CONFIG;
  }
  if (!problem->ops.rhs && !problem->ops.residual)
  {
    return UMSOL_INVALID_CONFIG;
  }
  if (problem->form == UMSOL_PROBLEM_MASS_MATRIX && !problem->ops.mass_matrix)
  {
    return UMSOL_INVALID_CONFIG;
  }
  return UMSOL_OK;
}
