#include "umsol/umsol.h"

#include <stdio.h>

static void smoke_log(void *ctx, UmsolLogLevel level, const char *message)
{
  (void)ctx;
  (void)level;
  puts(message);
}

static UmsolStatus rhs(void *ctx, UmsolReal t, const UmsolReal *y, UmsolReal *f)
{
  (void)ctx;
  (void)t;
  f[0] = -y[0];
  return UMSOL_OK;
}

int main(void)
{
  UmsolConfig config;
  UmsolProblem problem;
  UmsolSolver *solver = 0;
  UmsolReal y0[1] = {1.0};
  UmsolStepResult result;
  UmsolStatus status;

  umsol_config_set_defaults(&config);
  config.method = UMSOL_METHOD_RK_USER;
  config.stepper = UMSOL_STEPPER_RK;
  config.logger.ctx = 0;
  config.logger.log = smoke_log;

  problem.form = UMSOL_PROBLEM_ODE;
  problem.n_states = 1;
  problem.n_events = 0;
  problem.ctx = 0;
  problem.ops.rhs = rhs;
  problem.ops.rhs_active = 0;
  problem.ops.residual = 0;
  problem.ops.jacobian = 0;
  problem.ops.mass_matrix = 0;
  problem.ops.events = 0;
  problem.ops.state_meta = 0;

  status = umsol_solver_create(&config, &problem, &solver);
  if (status != UMSOL_OK)
  {
    fprintf(stderr, "create failed: %s\n", umsol_status_string(status));
    return 1;
  }

  umsol_solver_dump_settings(solver);

  status = umsol_solver_reinit(solver, 0.0, y0);
  if (status != UMSOL_OK)
  {
    fprintf(stderr, "reinit failed: %s\n", umsol_status_string(status));
    umsol_solver_destroy(solver);
    return 1;
  }

  status = umsol_solver_advance(solver, 0.0, &result);
  if (status != UMSOL_OK)
  {
    fprintf(stderr, "zero advance failed: %s\n", umsol_status_string(status));
    umsol_solver_destroy(solver);
    return 1;
  }

  umsol_solver_destroy(solver);
  return 0;
}
