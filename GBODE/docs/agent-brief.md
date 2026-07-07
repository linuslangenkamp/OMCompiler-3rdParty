# UMSOL Agent Brief

Read this before changing the UMSOL design or starting implementation.

## One-Sentence Target

Build UMSOL as the Uno-style numerical simulation framework: a pure C external library with a small integration core and replaceable method, stepper, error, controller, interpolation, nonlinear solver, linear solver, event, and multirate partition modules.

## Hard Constraints

- Do not refactor current GBODE in-place as the main strategy.
- Do not base the new design on `DATA_GBODE` or `DATA_GBODEF`.
- Do not put RK fields in the base method object.
- Do not force every integrator to store `kLeft`, `kRight`, stage arrays, or dense-output fields.
- Do not include OpenModelica structs in the UMSOL core.
- Do not allocate in hot paths such as trial step, error estimate, accept, reject, or interpolation query.
- Do not keep KINSOL/Newton legacy solver choices in the new design.

## Current Decisions

- Library name: UMSOL, Unified Multirate Solver.
- Language/API: pure C.
- External dependencies: BLAS/LAPACK/KLU via CMake.
- Problem class: ODE plus mass-matrix DAE design.
- First code slice: single-rate, all current RK methods, internal Newton, KLU, all current error estimators.
- Multirate: design from start, implement after SR core.
- Fast state: always packed `0..nFast-1`.
- Slow state: full vector initially.
- Controller: owns rejection policy.
- Event handling: UMSOL locates roots/interpolates state; OpenModelica performs event iteration.
- Testing: Robertson first, compare L2 norm, not exact step sequence.

## Base Method Shape

Use C-style OOP:

```c
typedef struct gb_method {
  const gb_method_ops *ops;
  void *impl;
} gb_method;
```

The base method is only a generic integration-method handle. Method-specific data lives in `impl` and method-specific ops/state.

Wrong:

```c
struct gb_method {
  int nStages;
  double *A;
  double *b;
  double *c;
};
```

Right:

```text
gb_method -> RK implementation
gb_method -> BDF implementation
gb_method -> Rosenbrock implementation
```

The stepper is responsible for validating that the method object is compatible with the stepper.

## Main Runtime Workflow

```text
gb_solver_advance(target)
  while current time < target:
    h = controller/limiter proposed h
    trial = stepper.try_step(method, problem, h)
    error = estimator.estimate(trial)
    norm = error_norm(error, tolerances)
    if rejected:
      h = controller.reject(...)
      retry
    interpolant.prepare(trial)
    event = root_finder.check(interpolant)
    if event:
      accept boundary at event time
      return GB_EVENT_FOUND
    accept trial
    controller.accept(norm, used_order)
    maybe output/interpolate
```

The driver must not know whether the method is RK, BDF, Rosenbrock, or something else.

## Multirate Workflow

```text
slow/full integrator proposes accepted interval
  -> partitioner classifies fast states
  -> if fast set nonempty:
       configure fast packed view
       run fast integrator over slow interval
       merge packed fast result into full state
  -> accept coupled result
```

On fast partition changes, reset all dependent histories.

## Open Points To Preserve

- Tolerance scaling ownership is still undecided.
- Packed-fast callback API needs design care.
- `gb_config` exact structure is open.
- Hermite endpoint derivative allocation must account for contractive defect/filter needs.

## Files

- [decisions.md](decisions.md): current decisions distilled from user answers.
- [method-interface.md](method-interface.md): method abstraction that avoids RK-shaped base structs.
- [api-sketch.md](api-sketch.md): provisional C API.
- [workflows.md](workflows.md): control-flow sketches.
- [design-questions.md](design-questions.md): original answered questionnaire.
