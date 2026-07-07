#include "umsol/config.h"

void umsol_config_set_defaults(UmsolConfig *config)
{
  if (!config)
  {
    return;
  }

  config->method_object = 0;
  config->method = UMSOL_METHOD_UNSET;
  config->stepper = UMSOL_STEPPER_UNSET;
  config->error = UMSOL_ERROR_EMBEDDED;
  config->controller = UMSOL_CONTROLLER_PI;
  config->interpolant = UMSOL_INTERPOLANT_DENSE_OUTPUT;
  config->nls = UMSOL_NLS_INTERNAL_NEWTON;
  config->linear_solver = UMSOL_LINSOLVER_KLU;
  config->partitioner = UMSOL_PARTITION_NONE;

  config->rel_tol = (UmsolReal)1e-6;
  config->abs_tol = (UmsolReal)1e-6;
  config->h_initial = (UmsolReal)0.0;
  config->h_min = (UmsolReal)0.0;
  config->h_max = (UmsolReal)0.0;
  config->max_steps = (UmsolInt)100000;

  config->multirate_enabled = UMSOL_FALSE;
  config->logger.ctx = 0;
  config->logger.log = 0;
}
