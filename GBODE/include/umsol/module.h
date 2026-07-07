#ifndef UMSOL_MODULE_H
#define UMSOL_MODULE_H

#include "umsol/method.h"
#include "umsol/problem.h"
#include "umsol/status.h"
#include "umsol/types.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct UmsolWorkspace UmsolWorkspace;
typedef struct UmsolStepper UmsolStepper;
typedef struct UmsolErrorEstimator UmsolErrorEstimator;
typedef struct UmsolController UmsolController;
typedef struct UmsolInterpolant UmsolInterpolant;
typedef struct UmsolNlsSolver UmsolNlsSolver;
typedef struct UmsolLinearSolver UmsolLinearSolver;
typedef struct UmsolPartitioner UmsolPartitioner;

typedef struct UmsolWorkRequirements
{
  UmsolInt real_count;
  UmsolInt int_count;
} UmsolWorkRequirements;

typedef struct UmsolTrialStep
{
  UmsolReal t0;
  UmsolReal h;
  const UmsolReal *y0;
  const UmsolReal *y1;
  void *impl;
} UmsolTrialStep;

typedef struct UmsolStepContext
{
  UmsolProblem *problem;
  UmsolMethod *method;
  UmsolMethodState *method_state;
  UmsolNlsSolver *nls;
  UmsolLinearSolver *linear_solver;
  UmsolWorkspace *workspace;
} UmsolStepContext;

typedef struct UmsolStepperOps
{
  UmsolStatus (*configure)(UmsolStepper *stepper, const UmsolMethod *method, UmsolMethodState *method_state,
                           const UmsolConfig *config);
  UmsolStatus (*query_work)(const UmsolStepper *stepper, UmsolWorkRequirements *requirements);
  UmsolStatus (*bind_work)(UmsolStepper *stepper, UmsolWorkspace *workspace);
  UmsolStatus (*try_step)(UmsolStepper *stepper, UmsolStepContext *context, UmsolTrialStep *trial);
  void (*reset)(UmsolStepper *stepper, UmsolInt reason);
  void (*destroy)(UmsolStepper *stepper);
} UmsolStepperOps;

struct UmsolStepper
{
  const UmsolStepperOps *ops;
  void *impl;
};

typedef struct UmsolErrorResult
{
  UmsolReal *component_error;
  UmsolReal tolerance_scale;
  UmsolInt used_order;
  UmsolInt used_kind;
  UmsolInt fallback_reason;
} UmsolErrorResult;

typedef struct UmsolErrorOps
{
  UmsolStatus (*estimate)(UmsolErrorEstimator *estimator, const UmsolTrialStep *trial, UmsolErrorResult *result);
  void (*reset)(UmsolErrorEstimator *estimator, UmsolInt reason);
  void (*destroy)(UmsolErrorEstimator *estimator);
} UmsolErrorOps;

struct UmsolErrorEstimator
{
  const UmsolErrorOps *ops;
  void *impl;
  UmsolErrorEstimator *fallback;
};

typedef struct UmsolControllerOps
{
  UmsolReal (*accept)(UmsolController *controller, UmsolReal error_norm, UmsolInt error_order, UmsolReal h);
  UmsolReal (*reject)(UmsolController *controller, UmsolInt reason, UmsolReal h);
  void (*reset)(UmsolController *controller, UmsolInt reason);
  void (*destroy)(UmsolController *controller);
} UmsolControllerOps;

struct UmsolController
{
  const UmsolControllerOps *ops;
  void *impl;
};

typedef struct UmsolAcceptedStep
{
  UmsolReal t0;
  UmsolReal t1;
  UmsolReal h;
  const UmsolReal *y0;
  const UmsolReal *y1;
  const UmsolTrialStep *trial;
} UmsolAcceptedStep;

typedef struct UmsolInterpolantTraits
{
  UmsolBool needs_left_state;
  UmsolBool needs_right_state;
  UmsolBool needs_left_derivative;
  UmsolBool needs_right_derivative;
  UmsolBool uses_method_dense_output;
  UmsolBool uses_method_trial_data;
} UmsolInterpolantTraits;

typedef struct UmsolInterpolantOps
{
  UmsolStatus (*configure)(UmsolInterpolant *interpolant, const UmsolMethod *method, UmsolMethodState *method_state,
                           const UmsolConfig *config);
  UmsolStatus (*query_work)(const UmsolInterpolant *interpolant, UmsolWorkRequirements *requirements);
  UmsolStatus (*bind_work)(UmsolInterpolant *interpolant, UmsolWorkspace *workspace);
  UmsolStatus (*prepare)(UmsolInterpolant *interpolant, const UmsolAcceptedStep *step);
  UmsolStatus (*eval)(UmsolInterpolant *interpolant, UmsolReal theta, const UmsolActiveView *view, UmsolReal *y_out);
  UmsolInterpolantTraits (*traits)(const UmsolInterpolant *interpolant);
  void (*reset)(UmsolInterpolant *interpolant, UmsolInt reason);
  void (*destroy)(UmsolInterpolant *interpolant);
} UmsolInterpolantOps;

struct UmsolInterpolant
{
  const UmsolInterpolantOps *ops;
  void *impl;
};

typedef struct UmsolNlsProblem
{
  void *ctx;
  UmsolInt size;
  UmsolStatus (*residual)(void *ctx, const UmsolReal *x, UmsolReal *residual);
  UmsolStatus (*jacobian)(void *ctx, const UmsolReal *x, UmsolMatrix *jacobian);
} UmsolNlsProblem;

typedef struct UmsolNlsOps
{
  UmsolStatus (*configure)(UmsolNlsSolver *nls, const UmsolConfig *config);
  UmsolStatus (*query_work)(const UmsolNlsSolver *nls, UmsolWorkRequirements *requirements);
  UmsolStatus (*bind_work)(UmsolNlsSolver *nls, UmsolWorkspace *workspace);
  UmsolStatus (*solve)(UmsolNlsSolver *nls, const UmsolNlsProblem *problem, UmsolReal *x);
  void (*reset)(UmsolNlsSolver *nls, UmsolInt reason);
  void (*destroy)(UmsolNlsSolver *nls);
} UmsolNlsOps;

struct UmsolNlsSolver
{
  const UmsolNlsOps *ops;
  void *impl;
};

typedef struct UmsolLinearSolverOps
{
  UmsolStatus (*configure)(UmsolLinearSolver *solver, const UmsolConfig *config);
  UmsolStatus (*analyze)(UmsolLinearSolver *solver, const UmsolMatrix *matrix);
  UmsolStatus (*factor)(UmsolLinearSolver *solver, const UmsolMatrix *matrix);
  UmsolStatus (*solve)(UmsolLinearSolver *solver, UmsolReal *rhs);
  UmsolStatus (*apply_filter)(UmsolLinearSolver *solver, UmsolReal scale, UmsolReal *x);
  void (*reset)(UmsolLinearSolver *solver, UmsolInt reason);
  void (*destroy)(UmsolLinearSolver *solver);
} UmsolLinearSolverOps;

struct UmsolLinearSolver
{
  const UmsolLinearSolverOps *ops;
  void *impl;
};

typedef struct UmsolPartitionResult
{
  UmsolInt n_fast;
  UmsolInt n_slow;
  UmsolInt *fast_to_full;
  UmsolInt *slow_to_full;
  UmsolBool changed;
} UmsolPartitionResult;

typedef struct UmsolPartitionOps
{
  UmsolStatus (*configure)(UmsolPartitioner *partitioner, const UmsolConfig *config, UmsolInt n_states);
  UmsolStatus (*query_work)(const UmsolPartitioner *partitioner, UmsolWorkRequirements *requirements);
  UmsolStatus (*bind_work)(UmsolPartitioner *partitioner, UmsolWorkspace *workspace);
  UmsolStatus (*classify)(UmsolPartitioner *partitioner, const UmsolReal *component_error,
                          UmsolPartitionResult *result);
  void (*reset)(UmsolPartitioner *partitioner, UmsolInt reason);
  void (*destroy)(UmsolPartitioner *partitioner);
} UmsolPartitionOps;

struct UmsolPartitioner
{
  const UmsolPartitionOps *ops;
  void *impl;
};

#ifdef __cplusplus
}
#endif

#endif
