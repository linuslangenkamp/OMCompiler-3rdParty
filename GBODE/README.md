# UMSOL / GBODE Rewrite Design Draft

This directory is a design draft for **UMSOL**, the Unified Multirate Solver. The goal is not to reorganize the current OpenModelica GBODE runtime implementation line by line. The current GBODE code should be used as a reference for important numerical procedures while the new library gets a clean architecture.

The intended model is similar to Uno for optimization: a small solver core plus composable algorithmic modules. For simulation this means an integration framework where methods, steppers, nonlinear solvers, linear solvers, error estimators, controllers, interpolation, event handling, and multirate partitioning are separate strategies with explicit contracts.

## Goals

- Provide a generic time-integration framework, not only a Runge-Kutta implementation.
- Support ODEs and mass-matrix DAEs by design.
- Support single-rate and multirate integration from the same orchestration model.
- Store fast-state data in packed buffers `0..nFast-1`; scatter/gather to full state only at model callback boundaries.
- Keep all allocations in initialization or explicit reconfiguration steps, never in the hot stepping loop.
- Make estimator order, tolerance scaling, fallbacks, and controller input explicit.
- Treat interpolation and dense output as optional method traits, not generic integrator fields.
- Keep OpenModelica-specific callbacks, logging, flags, and event glue outside the numerical core.
- Preserve current GBODE behavior first, then replace internals module by module.

## Non-goals

- Do not model the new library after current `DATA_GBODE` / `DATA_GBODEF`.
- Do not expose OpenModelica runtime structs in the numerical core.
- Do not make multirate a copy of single-rate with different field names.
- Do not allocate per step because fast-state membership changes.
- Do not force every integrator to store Hermite endpoint derivatives.

## Draft Documents

- [Architecture](docs/architecture.md)
- [Decisions](docs/decisions.md)
- [Agent Brief](docs/agent-brief.md)
- [Method Interface](docs/method-interface.md)
- [Workflows](docs/workflows.md)
- [API Sketch](docs/api-sketch.md)
- [Design Questions](docs/design-questions.md)
- [Open Questions](docs/open-questions.md)

## Standalone Build

UMSOL is not wired into OpenModelica yet. Build it directly:

```sh
cmake -S OMCompiler/3rdParty/GBODE -B /tmp/umsol-build
cmake --build /tmp/umsol-build
ctest --test-dir /tmp/umsol-build --output-on-failure
```

The current implementation is only the library foundation: public C API, config/problem/logger types, opaque method and module interfaces, workspace allocation, BLAS-backed vector kernels, and solver lifecycle. Actual RK stepping is the next implementation layer.

## Design Principle

GBODE should be a framework for composing time integration algorithms:

```text
Problem callbacks
      |
      v
OM glue adapter  ->  GB simulation core  ->  stepper/error/controller/interpolant/nls/linsolver/partition modules
      |
      v
OpenModelica runtime: events, flags, logging, result emission
```

The core should know that it is integrating an initial value problem with optional events and optional multirate partitions. It should not know how OpenModelica stores states, where logs go, or how model callbacks are implemented.
