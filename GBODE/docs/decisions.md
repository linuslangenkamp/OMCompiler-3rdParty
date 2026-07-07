# UMSOL Design Decisions

This file distills the answered design questions into concrete decisions. It is the main reference for future implementation work.

Working name: **UMSOL**, Unified Multirate Solver for ODEs and DAEs.

The current directory is still `GBODE` because it was created as a GBODE rewrite draft, but the intended library name is UMSOL.

## Scope

- Build UMSOL as an external pure C library.
- It may be developed in the OpenModelica tree initially, but it should be designed like SUNDIALS/IDA: a standalone numerical library with a thin OpenModelica adapter.
- The public API is C only.
- BLAS, LAPACK, and KLU are external dependencies supplied through CMake targets/interfaces.
- The target is not bit-for-bit equivalence with current GBODE. The target is a cleaner solver with the same key numerical procedures where needed: internal NLS, KLU-based linear solves, current RK methods, and current error-control logic.
- ODEs and mass-matrix DAEs are in scope from the design stage:

```text
M(t, y) * y' = f(t, y)
```

The first implementation may focus on ODEs, but the residual/NLS interfaces must not make DAEs hard to add.

## First Implementation Slice

- Single-rate first.
- Support all current Runge-Kutta methods first.
- Keep multirate in the architecture from day one, but do not implement it before the single-rate RK core is clean.
- Use only the internal Newton nonlinear solver.
- Delete KINSOL/Newton legacy support from the new design.
- Use KLU first, matching the current important implicit-path behavior.
- Implement all current error estimates, including embedded, two-step, contractive defect/filter, and Richardson where applicable.
- Keep constant-step mode because explicit methods need it.

## Method Model

`gb_method` is a generic integration method object, not an RK tableau.

Non-negotiable rule:

```text
base gb_method has no nStages, no A/b/c, no tableau, no generic RK dense-output fields
```

The base method is C-style OOP:

```text
gb_method
  const gb_method_ops *ops
  void *impl
```

The implementation data may be an RK tableau, BDF data, Rosenbrock data, extrapolation data, or something else. The core does not inspect it.

Method state is separate from the descriptor but owned/managed by the method strategy. The top-level integrator stores only an opaque method-state pointer. Variable-order methods are therefore possible because the method strategy allocates and manages its own state/work.

Families to design now:

- RK: all current methods.
- BDF: design for BDF, likely begin with BDF2 later.
- Rosenbrock: design now, implement later.
- Extrapolation: design now, implement later.
- Adaptive Radau/order-adaptive collocation: design should not block it, but no first implementation.

Fixed Radau/Gauss do not need a forced "collocation interface" distinction in the first design. They can be RK-family methods unless a later collocation module needs extra structure.

Dense output is a method property used by an interpolation module. The interpolant module may call the method's dense output directly when that is the selected interpolation path.

## Error Control

- Two-step fallback behavior is per method, as in the current design.
- Error estimator results should report at least:
  - `used_order`
  - `tolerance_scale` or equivalent scaling metadata
  - `fallback_reason` when a fallback was used
- `GB_ERROR_TOLERANCE_SAFETY` remains a macro, not runtime config.
- Norm separation follows current behavior: full, slow, fast, and interpolation errors exist where relevant.
- PI/PID history should not blindly survive estimator fallback/order changes. Reset or mark it invalid when the estimator/order changes in a way that makes the history inconsistent.

Open point:

- Exact ownership of tolerance scaling is not settled. The current best design is: estimator reports scaling metadata; error-norm/tolerance module applies it.

## Controller / Rejection

- Controller sees scalar normalized error and the used estimator order.
- Rejection policy belongs with the controller, not a separate high-level module.
- If no method/controller metadata is available, default rejection factor is `0.5`.
- Event/startup special rejection factors can be controller metadata.

## Interpolation / Events

- Interpolation is necessary for multirate and output/event handling, but it must not force all integrators to carry Hermite-specific endpoint fields.
- Dense output is preferred for events when available.
- Output interpolation is used for event location; no separate independent event interpolant for now.
- Interpolation error control should be designed as a wrapper module but can be `NULL`/disabled initially.
- Hermite endpoint derivative allocation is not settled because contractive defect/filter and event/output needs interact. Do not put endpoint derivative buffers into the base integrator; allocate them through the interpolation/error modules that need them.

## Multirate

- Multirate is composed from integrators, not implemented as one giant integrator with special-case fields.
- Fast state is always packed:

```text
fast_y[0..nFast-1]
```

- The current scattered full-vector fast storage should not be copied into UMSOL.
- Slow state remains full vector initially.
- Future nested multirate should be possible: fast inside fast, three-rate/four-rate, etc.
- Fast-state partition changes only at slow-step boundaries.
- When fast states change, reset all dependent histories: error, interpolation, NLS/Jacobian state, controller.
- Partition strategies should be pluggable:
  - error threshold
  - user mask
  - heuristic
  - future strategies
- Fast RHS callback design is still open. We know the numerical core should store fast data packed; the best user-facing callback API for scatter/full-state reconstruction still needs careful design.

## OpenModelica Adapter

- OpenModelica gets an adapter layer, similar in spirit to other solver wrapper files.
- No OpenModelica runtime structs in the UMSOL core.
- The adapter owns `DATA`, `threadData_t`, `SOLVER_INFO`, flags, result emission, and event iteration.
- UMSOL should do root finding and interpolate to the event point, then return event status to OpenModelica.
- UMSOL should be able to advance or emit/interpolate without taking a new accepted step when output grid requests are finer than integrator steps.
- Logging should use a logger instance/callback passed into the library. Keep only important warnings/errors initially.
- Whether OpenModelica flags map into one `gb_config` at init is not settled, but a config object is still the cleanest likely adapter boundary.

## Testing

- First oracle/problem: Robertson.
- Compare solution quality with an L2 norm, not exact accepted step sequences.
- Do not create an "ugly equivalence mode" for current GBODE behavior.
- Keep old GBODE until UMSOL passes the relevant library tests.
- Startup diagnostics should include a settings dump: selected method, estimator, controller, NLS, linear solver, interpolation, MR policy if active.

## Important Open Points

- Exact tolerance-scaling ownership.
- Best user-facing callback shape for packed fast RHS/Jacobian in multirate.
- Whether new bug fixes need guard config before full test coverage.
- Exact format of the UMSOL config object.
