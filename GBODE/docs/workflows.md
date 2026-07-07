# GBODE Workflows

This document describes intended control flow. It is not a translation of current `gbode_main.c`.

## Library Initialization

```text
OpenModelica runtime
  |
  v
create gb_problem adapter
  |
  v
choose gb_config from flags/XML/defaults
  |
  v
gb_create(config, problem)
  |
  v
select modules:
  method
  stepper
  error estimator
  controller
  interpolant
  event locator
  NLS solver
  linear solver
  partitioner if multirate
  |
  v
create method state
  |
  v
validate method/stepper/error/interpolant compatibility
  |
  v
query work requirements
  |
  v
allocate workspace once
  |
  v
bind module work slices
  |
  v
initialize accepted history and callback state
```

No stepper-specific buffers are allocated by the OpenModelica glue.

## Main OpenModelica Call

OpenModelica should call the library through a narrow adapter:

```c
gb_status gb_omc_integrate_to(gb_omc_solver *solver, double target_time);
```

Inside:

```text
receive target time from OM runtime
  |
  v
gb_integrator_advance(core, target_time)
  |
  v
adapter emits results / handles events returned by core
```

OpenModelica-specific responsibilities remain outside the core:

- Parse flags.
- Own original `DATA`, `threadData_t`, `SOLVER_INFO`.
- Translate callback failures.
- Log with OpenModelica streams.
- Emit results.
- Apply event iteration.

## Single-Rate Step

```text
while t < target:
  h = limiter.propose(t, target, h)

  repeat:
    trial = stepper.try_step(t, y, h)
    if trial failed:
      h = controller.reject(step_failure, metadata)
      continue

    error = error_estimator.estimate(trial)
    norm = error_norm(error.componentwise, tolerances)

    if norm > 1 and controller is adaptive:
      h = controller.reject(error_failure, metadata)
      continue

    interpolation.prepare_interval(trial)
    event = event_locator.check_interval(interpolation)
    if event found:
      state_at_event = interpolation.eval(event.time)
      accept_event_boundary(event, state_at_event)
      return GB_EVENT

    accept trial:
      history.push(t+h, y_trial, derivative_or_stage_data)
      controller.accept(norm, error.used_order, h)
      t = t+h
      h = controller.next_h()
      output if requested
      break
```

The driver does not know whether the stepper is DIRK, FIRK, BDF, or Rosenbrock. Family-specific data stays behind the method/stepper/error/interpolant modules.

## Error Estimator Workflow

```text
gb_error_estimator.estimate(context):
  if preferred estimator preconditions hold:
    run preferred
    used_order = preferred.order
    fallback = none
  else if fallback exists:
    run fallback
    used_order = fallback.order
    fallback = reason
  else:
    return GB_ERR_ESTIMATOR_UNAVAILABLE

  return componentwise error and used_order
```

Preconditions can include:

- Previous accepted step exists.
- No event invalidated history.
- Fast-state partition did not change.
- Required method weights exist.
- Required NLS filter/factorization exists.

The controller receives `used_order`, never the preferred order.

## Interpolation Workflow

Interpolation is a module selected by requested output/event behavior and method traits.

```text
on accepted trial:
  interpolant.prepare(trial)

on output/event query:
  interpolant.eval(theta, state_view)
```

Examples:

- Linear interpolant stores only endpoint states.
- Hermite interpolant stores endpoint states and endpoint derivatives.
- Dense-output interpolant references method-specific dense-output data.
- Error-controlled interpolant wraps another interpolant and a comparison rule.

Endpoint derivative buffers are not part of the base integrator.

## DIRK/FIRK NLS Workflow

```text
stepper.try_step:
  for each implicit stage or coupled stage block:
    build residual descriptor
    nls.solve(residual, initial_guess)
    if failed:
      return GB_STEP_CONVERGENCE_FAILURE
    unpack accepted stage solution
  assemble trial state
```

NLS owns Newton state:

- Iteration counters.
- Convergence thresholds.
- Newton damping/contractivity logic.
- Linear solve requests.

Linear solver owns:

- Matrix storage.
- Symbolic/numeric factorization.
- Solve/filter operations.

## Multirate Step

Multirate is a composition, not a special case in the single-rate loop.

```text
slow trial step
  |
  v
slow error estimate over full/slow state
  |
  v
partitioner.classify(error)
  |
  +-- no fast states:
  |     accept slow trial
  |
  +-- fast states exist:
        configure fast view:
          fastToFull
          packed fast buffers
          selected RHS equations
          NLS active size
        run fast integrator over slow interval:
          slow interpolation is supplied by coupling module
          fast state is packed
        merge fast result into full state
        accept coupled result
```

Partition changes:

```text
partition changed
  |
  v
fast view remaps packed slots
  |
  v
invalidate estimator/interpolant history that depends on previous fast meaning
  |
  v
schedule NLS/Jacobian structure update
```

This avoids reallocating fast arrays. Buffers have capacity `nStates`, active size `nFast`.

## Event Handling

The core should return events, not perform OpenModelica event iteration.

```text
accepted interval candidate
  |
  v
event_locator checks indicators using interpolant
  |
  +-- no event:
  |     accept normally
  |
  +-- event:
        locate event time
        evaluate state at event
        accept boundary state
        invalidate histories as configured
        return GB_EVENT_FOUND
```

The OpenModelica adapter then:

- Updates runtime state.
- Runs event iteration.
- Requests reinitialization/restart/no-restart behavior from GBODE.

## Restart / No-Restart

Restart behavior should be a policy:

```text
gb_restart_policy
  on_startup
  on_event
  on_partition_change
  on_step_failure
```

Examples:

- Full restart: reset history and choose initial step size.
- No restart after event: keep valid endpoint/interpolation data but invalidate estimators requiring continuous history.
- Partition change: preserve full accepted state but invalidate packed fast two-step history.

## External Library Integration

OpenModelica wrapper should eventually look like:

```text
OMCompiler runtime
  gb_omc_create()
    builds gb_problem adapter
    converts flags to gb_config
    creates gb_solver

  gb_omc_step_to(target)
    calls gb_solver_advance()
    maps returned event/output status to SOLVER_INFO

  gb_omc_free()
```

Only the wrapper includes OpenModelica headers.
