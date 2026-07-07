# GBODE API Sketch

The API below is a design sketch. Names are provisional.

## Public Core API

```c
typedef struct gb_solver gb_solver;
typedef struct gb_problem gb_problem;
typedef struct gb_config gb_config;

typedef enum gb_status {
  GB_OK,
  GB_EVENT_FOUND,
  GB_OUTPUT_READY,
  GB_STEP_REJECTED,
  GB_STEP_FAILED,
  GB_CALLBACK_FAILED,
  GB_OUT_OF_MEMORY,
  GB_INVALID_CONFIG
} gb_status;

gb_status gb_solver_create(const gb_config *config,
                           gb_problem *problem,
                           gb_solver **solver);

gb_status gb_solver_reinit(gb_solver *solver,
                           double t0,
                           const double *y0,
                           unsigned flags);

gb_status gb_solver_advance(gb_solver *solver,
                            double target_time,
                            gb_step_result *result);

void gb_solver_destroy(gb_solver *solver);
```

## Problem Interface

```c
typedef enum gb_problem_form {
  GB_PROBLEM_ODE,          /* y' = f(t, y) */
  GB_PROBLEM_MASS_MATRIX   /* M(t, y) * y' = f(t, y) */
} gb_problem_form;

typedef struct gb_problem_ops {
  gb_status (*rhs_full)(void *ctx, double t,
                        const double *y_full,
                        double *f_full);

  gb_status (*rhs_selected)(void *ctx, double t,
                            const double *y_full,
                            const int *active_full_idx,
                            int n_active,
                            double *f_active_packed);

  gb_status (*residual)(void *ctx,
                        const gb_residual_request *req,
                        double *residual);

  gb_status (*jacobian)(void *ctx,
                        const gb_jacobian_request *req,
                        gb_matrix *matrix);

  gb_status (*mass_matrix)(void *ctx,
                           const gb_mass_matrix_request *req,
                           gb_matrix *mass);

  gb_status (*events)(void *ctx, double t,
                      const double *y_full,
                      double *event_indicators);

  gb_status (*state_meta)(void *ctx,
                          double *nominal,
                          double *min,
                          double *max);
} gb_problem_ops;

struct gb_problem {
  gb_problem_form form;
  int n_states;
  int n_events;
  void *ctx;
  gb_problem_ops ops;
};
```

OpenModelica implements this interface in `gb_omc_problem`.

## Module Interfaces

### Method

```c
typedef struct gb_method {
  const gb_method_ops *ops;
  void *impl;
} gb_method;

typedef struct gb_method_state gb_method_state;

typedef struct gb_method_ops {
  const char *(*name)(const gb_method *method);

  gb_status (*init)(gb_method *method,
                    const gb_method_config *config);

  gb_status (*create_state)(const gb_method *method,
                            const gb_method_config *config,
                            gb_method_state **state);

  void (*reset_state)(const gb_method *method,
                      gb_method_state *state,
                      gb_reset_reason reason);

  void (*destroy_state)(const gb_method *method,
                        gb_method_state *state);

  void (*destroy)(gb_method *method);
} gb_method_ops;
```

The base method is not RK-shaped. Family-specific modules access typed implementation data through helpers such as:

```c
const gb_rk_method *gb_method_as_rk(const gb_method *method);
const gb_bdf_method *gb_method_as_bdf(const gb_method *method);
const gb_rosenbrock_method *gb_method_as_rosenbrock(const gb_method *method);
```

These return `NULL` for incompatible method/stepper combinations.

### Stepper

```c
typedef struct gb_stepper_ops {
  gb_status (*configure)(gb_stepper *stepper,
                         const gb_method *method,
                         gb_method_state *method_state,
                         const gb_config *config);

  gb_status (*query_work)(const gb_stepper *stepper,
                          gb_work_requirements *work);

  gb_status (*bind_work)(gb_stepper *stepper,
                         gb_workspace *workspace);

  gb_status (*try_step)(gb_stepper *stepper,
                        gb_step_context *ctx,
                        gb_trial_step *trial);

  void (*reset)(gb_stepper *stepper, gb_reset_reason reason);
} gb_stepper_ops;
```

### Error Estimator

```c
typedef struct gb_error_ops {
  gb_status (*estimate)(gb_error_estimator *estimator,
                        gb_error_context *ctx,
                        gb_error_result *result);

  void (*reset)(gb_error_estimator *estimator,
                gb_reset_reason reason);
} gb_error_ops;

typedef struct gb_error_result {
  double *component_error;
  int used_order;
  double tolerance_scale;
  gb_error_kind used_kind;
  gb_fallback_reason fallback_reason;
} gb_error_result;
```

Fallbacks are nested estimators:

```c
typedef struct gb_error_estimator {
  gb_error_kind kind;
  const gb_error_ops *ops;
  void *state;
  struct gb_error_estimator *fallback;
} gb_error_estimator;
```

### Controller

```c
typedef struct gb_controller_ops {
  double (*propose_factor)(gb_controller *controller,
                           double error_norm,
                           int error_order,
                           double h);

  void (*accept)(gb_controller *controller,
                 double error_norm,
                 int error_order,
                 double h);

  void (*reject)(gb_controller *controller,
                 gb_reject_reason reason);
} gb_controller_ops;
```

### Interpolant

```c
typedef struct gb_interpolant_ops {
  gb_status (*prepare)(gb_interpolant *interp,
                       const gb_accepted_step *step);

  gb_status (*eval)(gb_interpolant *interp,
                    double theta,
                    gb_state_view view,
                    double *y_out);

  gb_interpolant_traits (*traits)(const gb_interpolant *interp);
} gb_interpolant_ops;
```

Traits:

```c
typedef struct gb_interpolant_traits {
  unsigned needs_left_state : 1;
  unsigned needs_right_state : 1;
  unsigned needs_left_derivative : 1;
  unsigned needs_right_derivative : 1;
  unsigned uses_stage_data : 1;
} gb_interpolant_traits;
```

### Nonlinear Solver

```c
typedef struct gb_nls_ops {
  gb_status (*configure)(gb_nls *nls,
                         const gb_nls_config *config);

  gb_status (*solve)(gb_nls *nls,
                     const gb_nls_problem *problem,
                     double *x);

  void (*reset)(gb_nls *nls, gb_reset_reason reason);
} gb_nls_ops;
```

The first implementation can provide only internal Newton.

### Linear Solver

```c
typedef struct gb_linsolver_ops {
  gb_status (*analyze)(gb_linsolver *solver,
                       const gb_sparsity *pattern);

  gb_status (*factor)(gb_linsolver *solver,
                      const gb_matrix *matrix);

  gb_status (*solve)(gb_linsolver *solver,
                     double *rhs);

  gb_status (*apply_filter)(gb_linsolver *solver,
                            double scale,
                            double *vec);
} gb_linsolver_ops;
```

`apply_filter` covers contractive filter/defect operations without hard-wiring KLU internals into error estimation.

## State Views

```c
typedef enum gb_rate_kind {
  GB_RATE_FULL,
  GB_RATE_SLOW,
  GB_RATE_FAST_PACKED
} gb_rate_kind;

typedef struct gb_state_view {
  gb_rate_kind kind;
  int n_total;
  int n_active;
  const int *active_to_full;
  const int *full_to_active;
} gb_state_view;
```

Fast packed data:

```text
fast_y[0]       -> full_y[fastToFull[0]]
fast_y[1]       -> full_y[fastToFull[1]]
...
fast_y[nFast-1] -> full_y[fastToFull[nFast-1]]
```

No numerical module should loop over scattered full-state indices unless it is explicitly a scatter/gather adapter.

## Configuration

```c
typedef struct gb_config {
  gb_method_id method;
  gb_stepper_id stepper;
  gb_error_id error;
  gb_controller_id controller;
  gb_interpolant_id interpolant;
  gb_nls_id nls;
  gb_linsolver_id linsolver;

  gb_multirate_config multirate;
  gb_tolerance_config tolerance;
  gb_restart_config restart;
  gb_logging_config logging;
} gb_config;
```

The OpenModelica adapter fills this from flags and defaults.

## Result Object

```c
typedef struct gb_step_result {
  gb_status status;
  double t;
  double h_accepted;
  double h_next;
  double error_norm;
  int error_order;
  gb_error_kind error_kind;
  gb_fallback_reason fallback_reason;
  gb_event_result event;
} gb_step_result;
```

The result should expose enough information for diagnostics without reaching into internal structs.
