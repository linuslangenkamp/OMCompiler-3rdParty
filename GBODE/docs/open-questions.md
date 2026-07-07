# Open Questions

These are the decisions that should be settled before writing real code.

## Library Placement

1. Should the new library live as `OMCompiler/3rdParty/GBODE`, or should it be developed in a separate repository and vendored later?
2. Should it be pure C from the start, or C core with optional C++ test/reference harness?

Recommended: pure C library in a separate repository or clean subdirectory, with an OpenModelica adapter as a thin integration layer.

## First Supported Scope

1. Should the first implementation support only the methods currently used by GBODE tests?
2. Should multirate be included from day one, or should single-rate be made clean first with the same abstractions?
3. Should internal Newton be the only NLS target initially?

Recommended first slice:

- Single-rate DIRK/FIRK plus current error framework.
- Internal Newton plus KLU/dense linear solver interface.
- Dense output/Hermite/linear interpolation as traits.
- Then add multirate on top of the state-view/partition API.

## Multirate Semantics

1. Is fast-state selection always error-threshold based, or should user-provided partition policies be first-class?
2. Should slow integration operate on full state with fast states frozen, or on a true slow packed view?
3. On fast-state changes, which histories are invalidated?
4. Should partition changes be allowed inside an outer accepted interval, or only at synchronization points?

Recommended:

- Use full slow state first for equivalence.
- Use packed fast state immediately.
- Repartition only at synchronization points.
- Invalidate fast two-step/error/interpolation history on partition changes.

## Interpolation / Events

1. Which interpolation modes are required for event location equivalence?
2. Can event location always use the selected output interpolant, or does it need a separate robust event interpolant?
3. Should endpoint derivatives be computed eagerly only when Hermite/event logic requires them?

Recommended:

- Make event locator request an interpolant by capability.
- Dense output preferred when method supports it.
- Hermite only allocates endpoint derivative buffers when selected or needed by events.

## Error Estimators

1. Should estimator fallback be configured globally or per method?
2. Should tolerance scaling be owned by the estimator or by an error-norm module?
3. Should `GB_ERROR_TOLERANCE_SAFETY` become a runtime config, compile-time constant, or estimator parameter?

Recommended:

- Estimator reports `used_order` and `tolerance_scale`.
- Error norm applies tolerances using that result.
- Safety factor is a config parameter with method defaults.

## Testing Strategy

1. Which current GBODE examples become oracle tests?
2. Do we compare exact step sequences or only final accuracy/work?
3. How much divergence is acceptable when the new architecture fixes known bugs?

Recommended:

- Build a replay-style oracle for selected models and fixed tolerances.
- Compare accepted times, errors, estimator choices, and event times for equivalence mode.
- Allow a separate "improved mode" after equivalence is established.
