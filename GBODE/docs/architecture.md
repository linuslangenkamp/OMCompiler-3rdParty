# GBODE Architecture

## Shape

The rewrite should be the simulation analogue of Uno: one framework, many strategy combinations.

```text
                              +----------------+
                              |   controller   |
                              +----------------+
                                      ^
                                      |
             +---------+      +---------------+      +---------------+
             | method  | ---> |    stepper    | ---> | error estimate |
             +---------+      +---------------+      +---------------+
                  ^                   |                       ^
                  |                   v                       |
+----------+  +----------+      +------------+        +----------------+
| problem  |  |  core    | ---> | nonlinear  | -----> | linear solver  |
| adapter  |  | driver   |      | solver     |        +----------------+
+----------+  +----------+      +------------+
                  |
                  v
          +----------------+
          | interpolation  |
          | dense output   |
          +----------------+
                  |
                  v
          +----------------+
          | event/output   |
          +----------------+

Multirate adds a partition strategy and one or more child integrators:

          +----------------+
          | partitioner    |
          +----------------+
             |          |
             v          v
      slow/full view   fast packed view
             |          |
             v          v
       slow core     fast core
```

## Main Objects

### `gb_problem`

Pure numerical callback interface. This is the only thing the core uses to evaluate the model.

Responsibilities:

- Evaluate RHS/residual for a given state view.
- Evaluate Jacobian or Jacobian-vector information.
- Provide nominal/min/max data for tolerance scaling.
- Report event indicators through an optional event interface.
- Expose dimensions and callback capabilities.

It must not contain OpenModelica structs. OpenModelica provides an adapter implementation.

### `gb_integrator`

Top-level orchestration object. It owns generic integration state:

- Current time and accepted step size.
- Current state view.
- Accepted-step history needed by selected modules.
- Controller history.
- Active module pointers.
- Workspaces required by the active module graph.

It does not own every possible buffer. Optional modules own optional work.

### `gb_method`

Generic C-style method object, not a Runge-Kutta tableau.

The base method contains only ops and opaque implementation data. It has no stages, no tableau, and no `A/b/c` fields.

```text
gb_method
  const gb_method_ops *ops
  void *impl
```

The implementation data may describe RK, BDF, Rosenbrock, extrapolation, or another integration method family. Family-specific steppers and estimators validate and access the matching family interface; the generic core does not.

Method state is per integrator and owned by the method strategy. This keeps variable-order methods possible without adding BDF/Rosenbrock/adaptive-order fields to the top-level integrator.

Examples:

- RK implementation: tableau, RK dense output, embedded/two-step RK estimates.
- BDF implementation: order range, coefficient generator, history form.
- Rosenbrock implementation: linearly implicit stage coefficients and Jacobian reuse policy.
- Extrapolation implementation: sequence and extrapolation table policy.
- Future adaptive Radau/collocation implementation.

### `gb_stepper`

Implements one trial step for a method family. It takes current integrator state and writes a trial object.

Contract:

- Input: accepted state, step size, method, problem, NLS/linear solver services.
- Output: trial state plus family-specific trial data owned by the stepper/method module.
- No accept/reject policy.
- No controller update.
- No event handling.

Stepper families:

- `gb_step_explicit_rk`
- `gb_step_dirk`
- `gb_step_firk`
- `gb_step_multistep`
- `gb_step_rosenbrock` later

The driver never loops over stages. RK steppers do that internally; BDF/Rosenbrock steppers do their own family-specific work.

### `gb_error_estimator`

Computes componentwise error for a trial step.

Contract:

- Input: accepted state/history, trial state, family-specific trial data if needed, method, current step ratio.
- Output: componentwise unscaled or scaled error plus actual estimator order.
- May select fallback internally and must report which fallback/order was used.

Examples:

- Embedded RK weights.
- Two-step estimator with startup/event fallback.
- Contractive defect.
- Contractive filter.
- Richardson.

Important rule: controller exponents and tolerance scaling use the estimator that actually ran, not the preferred estimator.

### `gb_controller`

Turns accepted error history into the next proposed step size.

Contract:

- Input: normalized scalar error, estimator order, old step history.
- Output: next step size factor.
- Does not compute error norms.
- Does not know which estimator generated the error.

### `gb_interpolant`

Optional output/event interpolation trait.

Responsibilities:

- Provide state at arbitrary output/event time inside an accepted step.
- Own only the data it needs.
- Distinguish method dense output from Hermite endpoint interpolation.

Examples:

- Linear: needs left/right states only.
- Hermite: needs endpoint derivatives.
- Dense output: needs method stage data and accepted state, not generic `kLeft/kRight`.
- Error-controlled interpolation: wraps a primary interpolant plus a check.

Endpoint derivative buffers must not live in the base integrator.

### `gb_event_locator`

Uses an interpolant to locate events in an accepted interval.

Responsibilities:

- Detect sign changes.
- Refine event time.
- Request state reconstruction from interpolant.
- Return event action to the OpenModelica adapter.

The numerical core should not manipulate OpenModelica event lists directly.

### `gb_nls_solver`

Nonlinear solve strategy.

For the first rewrite target, only internal Newton is needed. The interface should still be generic enough that solvers are replaceable later.

Responsibilities:

- Solve a stage residual system.
- Own Newton iteration data and convergence logic.
- Ask linear solver module for matrix factorization/solve.
- Report convergence/failure status to stepper.

It should not know about OpenModelica flags or logging streams directly.

### `gb_linear_solver`

Linear algebra backend used by NLS and contractive filters.

Backends:

- Dense LAPACK.
- KLU sparse.
- Future iterative solvers if useful.

Responsibilities:

- Analyze/factor/solve.
- Support matrix rebuild policy.
- Provide filtered solve operations needed by error estimators.

### `gb_partition`

Multirate partition strategy.

Responsibilities:

- Compute fast/slow classification from error, state, or user policy.
- Maintain `fastToFull` and optional `fullToFast`.
- Detect partition changes.
- Provide packed views for fast integrator.
- Trigger NLS/Jacobian reconfiguration when active fast set changes.

Fast integrator state uses packed slots `0..nFast-1`. The meaning of a slot changes after repartitioning, but the storage capacity stays `nStates`.

### `gb_workspace`

Central allocation plan.

Responsibilities:

- Query all selected modules for required work sizes.
- Allocate once at initialization with maximum capacities.
- Hand out typed slices to modules.
- Reinitialize slices on repartition without reallocating.

## Ownership Model

```text
gb_integrator
  owns generic time/control/state/history
  references method/problem/module ops
  contains module states:
    gb_method_state
    gb_stepper_state
    gb_error_state
    gb_controller_state
    gb_interpolant_state
    gb_nls_state
    gb_partition_state
    gb_workspace
```

Module state is not a bag of all possible fields. Each module exposes:

```c
typedef struct gb_module_ops {
  gb_status (*configure)(void *state, const gb_config *cfg);
  gb_status (*query_work)(const void *state, gb_work_requirements *req);
  gb_status (*bind_work)(void *state, gb_workspace *work);
  void      (*reset)(void *state, gb_reset_reason reason);
  void      (*destroy)(void *state);
} gb_module_ops;
```

## Multirate Design

Multirate should not duplicate the full integrator. It should compose two integrators with different views:

```text
gb_multirate_integrator
  slow: gb_integrator using full view or slow view
  fast: gb_integrator using packed fast view
  partition: gb_partition
  coupling: gb_mr_coupling
```

Coupling responsibilities:

- Interpolate slow state for fast RHS/residual calls.
- Merge accepted fast values into the full output state.
- Decide synchronization points.
- Handle fast-state changes.
- Invalidate two-step/error/interpolation history when needed.

Fast RHS callbacks:

```text
packed fast y[0..nFast-1]
      |
      v
scatter to full callback state only for fODE/residual/Jacobian
      |
      v
evaluate selected equations
      |
      v
pack fast derivative/result back to 0..nFast-1
```

This keeps BLAS-friendly packed buffers in the numerical core.

## Fallbacks

Fallbacks should be explicit strategy links, not ad hoc checks in the main loop.

Examples:

```text
two_step_estimator
  preferred: two-step weights
  fallback when history invalid: embedded estimator
  reports: used_order, used_tolerance_scale, fallback_reason
```

```text
dense_output_interpolant
  preferred: method dense output
  fallback when unavailable: Hermite or linear
  reports: interpolation_order, endpoint_derivative_need
```

Fallbacks must report enough information for controller and diagnostics.

## Performance Rules

- No allocation in `try_step`, `estimate_error`, `accept`, or `reject`.
- Fast-state data is packed in the core.
- Scatter/gather happens only at callback boundaries.
- BLAS operates over packed contiguous arrays where possible.
- Sparse symbolic analysis is reused until structure changes.
- Partition changes trigger rebind/reinitialize, not full teardown.
- Optional buffers are owned by the module that needs them.
