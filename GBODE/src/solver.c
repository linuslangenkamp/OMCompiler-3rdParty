#include "umsol/solver.h"

#include "internal/common.h"
#include "umsol/linalg.h"
#include "umsol/workspace.h"

struct UmsolSolver
{
  UmsolConfig config;
  UmsolProblem problem;
  UmsolWorkspace *workspace;

  UmsolReal t;
  UmsolReal h;
  UmsolReal *y;
  UmsolReal *weights;
  UmsolInt steps_taken;
  UmsolBool initialized;
};

static const char *umsol_method_name(UmsolMethodId method)
{
  switch (method)
  {
  case UMSOL_METHOD_UNSET:
    return "unset";
  case UMSOL_METHOD_RK_USER:
    return "rk_user";
  case UMSOL_METHOD_BDF_USER:
    return "bdf_user";
  case UMSOL_METHOD_ROSENBROCK_USER:
    return "rosenbrock_user";
  default:
    return "unknown";
  }
}

static const char *umsol_error_name(UmsolErrorId error)
{
  switch (error)
  {
  case UMSOL_ERROR_NONE:
    return "none";
  case UMSOL_ERROR_EMBEDDED:
    return "embedded";
  case UMSOL_ERROR_TWO_STEP:
    return "two_step";
  case UMSOL_ERROR_CONTRACTIVE_DEFECT:
    return "contractive_defect";
  case UMSOL_ERROR_CONTRACTIVE_FILTER:
    return "contractive_filter";
  case UMSOL_ERROR_RICHARDSON:
    return "richardson";
  default:
    return "unknown";
  }
}

static const char *umsol_controller_name(UmsolControllerId controller)
{
  switch (controller)
  {
  case UMSOL_CONTROLLER_CONSTANT:
    return "constant";
  case UMSOL_CONTROLLER_I:
    return "i";
  case UMSOL_CONTROLLER_PI:
    return "pi";
  case UMSOL_CONTROLLER_PID:
    return "pid";
  default:
    return "unknown";
  }
}

static UmsolStatus umsol_config_validate(const UmsolConfig *config)
{
  if (!config)
  {
    return UMSOL_INVALID_ARGUMENT;
  }
  if (config->rel_tol <= (UmsolReal)0.0 || config->abs_tol <= (UmsolReal)0.0)
  {
    return UMSOL_INVALID_CONFIG;
  }
  if (config->h_min < (UmsolReal)0.0 || config->h_max < (UmsolReal)0.0)
  {
    return UMSOL_INVALID_CONFIG;
  }
  if (config->h_max > (UmsolReal)0.0 && config->h_min > config->h_max)
  {
    return UMSOL_INVALID_CONFIG;
  }
  if (config->max_steps <= 0)
  {
    return UMSOL_INVALID_CONFIG;
  }
  return UMSOL_OK;
}

UmsolStatus umsol_solver_create(const UmsolConfig *config, const UmsolProblem *problem, UmsolSolver **solver)
{
  UmsolStatus status;
  UmsolSolver *created;

  if (!solver)
  {
    return UMSOL_INVALID_ARGUMENT;
  }
  *solver = 0;

  status = umsol_config_validate(config);
  if (status != UMSOL_OK)
  {
    return status;
  }

  status = umsol_problem_validate(problem);
  if (status != UMSOL_OK)
  {
    return status;
  }

  created = (UmsolSolver *)calloc(1, sizeof(UmsolSolver));
  if (!created)
  {
    return UMSOL_OUT_OF_MEMORY;
  }

  created->config = *config;
  created->problem = *problem;

  status = umsol_workspace_create(problem->n_states * (UmsolInt)4, problem->n_states, &created->workspace);
  if (status != UMSOL_OK)
  {
    free(created);
    return status;
  }

  umsol_workspace_reset(created->workspace);
  status = umsol_workspace_take_real(created->workspace, problem->n_states, &created->y);
  if (status == UMSOL_OK)
  {
    status = umsol_workspace_take_real(created->workspace, problem->n_states, &created->weights);
  }
  if (status != UMSOL_OK)
  {
    umsol_workspace_destroy(created->workspace);
    free(created);
    return status;
  }

  umsol_vec_set(problem->n_states, (UmsolReal)1.0, created->weights, (UmsolInt)1);

  *solver = created;
  return UMSOL_OK;
}

void umsol_solver_destroy(UmsolSolver *solver)
{
  if (!solver)
  {
    return;
  }

  umsol_workspace_destroy(solver->workspace);
  free(solver);
}

UmsolStatus umsol_solver_reinit(UmsolSolver *solver, UmsolReal t0, const UmsolReal *y0)
{
  if (!solver || !y0)
  {
    return UMSOL_INVALID_ARGUMENT;
  }

  solver->t = t0;
  solver->h = solver->config.h_initial;
  solver->steps_taken = 0;
  solver->initialized = UMSOL_TRUE;
  umsol_vec_copy(solver->problem.n_states, y0, (UmsolInt)1, solver->y, (UmsolInt)1);
  return UMSOL_OK;
}

UmsolStatus umsol_solver_advance(UmsolSolver *solver, UmsolReal target_time, UmsolStepResult *result)
{
  if (!solver || !result)
  {
    return UMSOL_INVALID_ARGUMENT;
  }
  if (!solver->initialized)
  {
    return UMSOL_INVALID_CONFIG;
  }

  result->status = UMSOL_NOT_IMPLEMENTED;
  result->t = solver->t;
  result->h_accepted = (UmsolReal)0.0;
  result->h_next = solver->h;
  result->error_norm = (UmsolReal)0.0;
  result->error_order = 0;
  result->error_kind = solver->config.error;
  result->fallback_reason = 0;

  if (target_time == solver->t)
  {
    result->status = UMSOL_OK;
    return UMSOL_OK;
  }

  umsol_log(&solver->config.logger, UMSOL_LOG_WARNING,
            "umsol_solver_advance is not implemented yet; RK stepper comes next.");
  return UMSOL_NOT_IMPLEMENTED;
}

void umsol_solver_dump_settings(const UmsolSolver *solver)
{
  if (!solver)
  {
    return;
  }

  umsol_log(&solver->config.logger, UMSOL_LOG_INFO, "UMSOL settings:");
  umsol_logf(&solver->config.logger, UMSOL_LOG_INFO, "  states: %d", (int)solver->problem.n_states);
  umsol_logf(&solver->config.logger, UMSOL_LOG_INFO, "  events: %d", (int)solver->problem.n_events);
  umsol_logf(&solver->config.logger, UMSOL_LOG_INFO, "  method: %s", umsol_method_name(solver->config.method));
  umsol_logf(&solver->config.logger, UMSOL_LOG_INFO, "  error: %s", umsol_error_name(solver->config.error));
  umsol_logf(&solver->config.logger, UMSOL_LOG_INFO, "  controller: %s",
             umsol_controller_name(solver->config.controller));
  umsol_logf(&solver->config.logger, UMSOL_LOG_INFO, "  rel_tol: %.16g", (double)solver->config.rel_tol);
  umsol_logf(&solver->config.logger, UMSOL_LOG_INFO, "  abs_tol: %.16g", (double)solver->config.abs_tol);
  umsol_logf(&solver->config.logger, UMSOL_LOG_INFO, "  multirate: %s",
             solver->config.multirate_enabled ? "enabled" : "disabled");
}
