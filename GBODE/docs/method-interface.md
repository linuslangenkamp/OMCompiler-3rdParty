# UMSOL Method Interface

The method abstraction is deliberately not a Runge-Kutta abstraction. RK is one possible method family.

## Base Rule

The base `gb_method` has no RK fields:

```text
no nStages
no tableau
no A / b / c
no generic embedded weights
no generic dense-output coefficients
```

The base method is a C-style object:

```c
typedef struct gb_method {
  const gb_method_ops *ops;
  void *impl;
} gb_method;
```

`impl` may point to RK data, BDF data, Rosenbrock data, extrapolation data, or another method implementation.

## Descriptor and State

Separate immutable method definition from mutable per-integrator state:

```text
gb_method
  immutable method implementation and ops

gb_method_state
  mutable state allocated by the method:
    current order
    startup phase
    cached coefficients
    history form
    dense-output state
    variable-order data
```

The top-level integrator stores only an opaque `gb_method_state *`. It does not inspect fields.

This enables:

- Fixed RK with almost no method state.
- BDF with current order and coefficient history.
- Rosenbrock with Jacobian reuse policy.
- Future adaptive-order Radau/collocation.

## Base Method Ops

Sketch:

```c
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

Family-specific access can be implemented with a tagged `impl` and family-specific helper functions. The core does not call those helpers. Steppers, estimators, and interpolants do.

Example:

```c
const gb_rk_method *gb_method_as_rk(const gb_method *method);
const gb_bdf_method *gb_method_as_bdf(const gb_method *method);
const gb_rosenbrock_method *gb_method_as_rosenbrock(const gb_method *method);
```

Those functions return `NULL` if the method is not compatible. Invalid combinations are rejected during solver creation/configuration.

## RK Method Interface

RK-specific code may use:

```c
typedef struct gb_rk_method {
  int n_stages;
  const double *A;
  const double *b;
  const double *c;

  gb_rk_structure structure;       /* explicit, DIRK, ESDIRK, FIRK */
  gb_rk_endpoint_traits endpoints; /* FSAL / left stage / right stage */

  const gb_rk_embedded_set *embedded;
  const gb_rk_twostep_set *twostep;
  const gb_rk_dense_output *dense_output;
} gb_rk_method;
```

This interface is not visible in `gb_integrator`.

## BDF Method Interface

BDF-specific code may use:

```c
typedef struct gb_bdf_method {
  int min_order;
  int max_order;
  gb_history_form history_form;

  gb_status (*coefficients)(const gb_bdf_method *method,
                            gb_method_state *state,
                            const gb_step_ratio_history *ratios,
                            gb_bdf_coefficients *out);

  gb_status (*build_residual)(const gb_bdf_method *method,
                              gb_method_state *state,
                              const gb_bdf_step_context *ctx,
                              gb_nls_problem *nls);
} gb_bdf_method;
```

BDF does not expose stage data because it does not have RK stages.

## Rosenbrock Method Interface

Rosenbrock-specific code may use:

```c
typedef struct gb_rosenbrock_method {
  int n_stages;
  int order;

  const double *alpha;
  const double *gamma;
  const double *b;
  const double *c;

  gb_jacobian_reuse_policy jacobian_policy;
  const gb_rosenbrock_embedded_set *embedded;
  const gb_rosenbrock_dense_output *dense_output;
} gb_rosenbrock_method;
```

Rosenbrock methods are linearly implicit and use the linear solver directly. They should not be forced through the internal Newton NLS interface.

## Dense Output

Dense output is a method capability used by the interpolation module.

```text
method owns dense-output formula
interpolant owns interpolation workflow and buffers
```

If dense output is selected and the method provides it, the interpolant calls the method dense-output function. If no dense output is available, the interpolant may fall back to Hermite or linear interpolation depending on configuration.

## Compatibility

Compatibility is checked during solver creation:

```text
RK stepper          requires gb_method_as_rk(method) != NULL
BDF stepper         requires gb_method_as_bdf(method) != NULL
Rosenbrock stepper  requires gb_method_as_rosenbrock(method) != NULL
RK estimator        requires RK trial data
BDF estimator       requires BDF trial/history data
Dense interpolant   requires dense-output capability or configured fallback
```

The generic driver never branches on RK stages or BDF order details. It calls:

```text
try_step
estimate_error
controller accept/reject
interpolate/output/event
```
