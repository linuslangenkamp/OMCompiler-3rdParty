# GBODE Rewrite Design Questions

Answer directly in this file. Short answers are fine: `yes`, `no`, `later`, or a sentence.

## Core Scope

1. Should GBODE be a standalone C library first, then vendored into OpenModelica?
   - Answer: Yes, external C library, that gets BLAS LAPACK KLU from somewhere, maybe specified via CMAke interface or so.

2. Or should it live directly in `OMCompiler/3rdParty/GBODE` from day one?
   - Answer: no, we will develop it here, but it really is an external library likle SUNDIALS IDA or so.

3. Should the public API be pure C only, or allow C++ test/harness code?
   - Answer: only C.

4. Is the first goal exact equivalence with current GBODE, or a cleaner solver that may intentionally differ?
   - Answer: Of course cleaner solver with better abstractions and workflows. Key procedures ofcourse the same (internal NLS + error control)

5. Should the first implementation support only ODEs, or also DAEs later by design?
   - Answer: also DAEs (just include) M * y' = f(t, y), so mass matrix form. we just need to change minor details in internal NLS then.

## Method Abstraction

6. Confirm: base `gb_method` has no RK fields, no stages, no tableau, no `A/b/c`.
   - Answer: exactly gb_method is purely an interface with void * for its ineternal data, this data is then a Rosenbrock tableau, or BDF tableau or whatever, and gets passed to step for context (maybe tagged enum idk). gb_method is a generic integration step method

7. Should `gb_method` use `query_interface()` style, e.g. RK/BDF/Rosenbrock interfaces?
   - Answer: idk, that is a bit weird, do like OOP in C. we have initialize() with all the metadata, then the core procedures and then the deallocate() or so.

8. Should method state be separate from method descriptor?
   - Answer: well, its part of some step or so, so not in top level but in the method somewhat yes.

9. Should variable-order methods be first-class from the start?
   - Answer: yes ofcourse, we must not add them yet, but the workspace and its memory is in the strategy itself, so there hould be no problem, the scheme allocates its memory and gb_main or whatever just calls it to receive like pointer views or so idk. or we have some solout callback that is aclaled after every event or successful step like Hairer style

10. Which families must be designed now: RK, BDF, Rosenbrock, extrapolation, adaptive Radau, others?
    - Answer: RK (all that are currently avalible with all the tuff we currently have), BDF also maybe BDF(2) for the start maybe, but no adaptive yet. But it may be possible then.

11. Should fixed Radau/Gauss expose both RK and collocation interfaces?
    - Answer: that Q: makes no sense?! what the fuck

12. Should dense output belong to method, interpolant module, or both via interface?
    - Answer: well its a method property but an interpolant module. so that module calls the dense output. but we can also call directly dense output if we want exactly that right.

## First Implementation

13. What is the first slice to actually code: single-rate RK only, or MR immediately?
    - Answer: SR first, but keep in mind MR

14. Which existing GBODE methods must be supported first?
    - Answer: all RK

15. Should internal Newton be the only nonlinear solver initially?
    - Answer: yes

16. Should KINSOL/Newton legacy support be deleted from the new design entirely?
    - Answer: delete

17. Which linear solvers first: dense LAPACK, KLU, both?
    - Answer: KLU as currently

18. Should contractive filter/defect be first-class in v1, or later?
    - Answer: what does first-class mean, makes no sense. Ofcourse all current error estiamtes

## Error Control

19. Should error estimators be chained fallback objects?
    - Answer: LIKE CURRENTLY STUPID Q

20. Should two-step fallback be configured per method or globally?
    - Answer: LIKE CURRENTLY PER METHOD STUPID Q

21. Should estimator report `used_order`, `tolerance_scale`, and `fallback_reason`?
    - Answer: ye something like that i guess

22. Who owns tolerance scaling: estimator or error-norm module?
    - Answer: not sure

23. Should `GB_ERROR_TOLERANCE_SAFETY` become runtime config?
    - Answer: no its macro

24. Do we need separate norms for full, slow, fast, and interpolation errors?
    - Answer: as currently

## Controller

25. Should controller only see scalar normalized error + used order?
    - Answer: yes as currently

26. Should PI/PID history survive estimator fallback/order changes?
    - Answer: no

27. Should constant-step mode still exist in the new core?
    - Answer: yes (for explicit its needed)

28. Should rejection policy be separate from controller?
    - Answer: no, it should be in controller and we should pass some metadata if avaible, if not avaible then just do 0.5 * 

## Interpolation / Events

29. Should interpolation be optional trait, never base integrator state?
    - Answer: its needed for MR so its necessary

30. Should Hermite endpoint derivatives be allocated only when selected/needed?
    - Answer: well contractiev defect needs them also so not clear

31. Should event detection require a robust interpolant independent of output interpolation?
    - Answer: no, output interpolation is used ofc

32. Should dense output be preferred for events when available?
    - Answer: yes, always used

33. Should interpolation error control be its own wrapper module?
    - Answer: ye, but keep NULL for now, just deisng that it may exist later

## Multirate

34. Should MR be two composed integrators, not a special case inside one integrator?
    - Answer: yes (but we aware of how the callbacks will llok then, also the Jcaobnian is tricky then, because we remove certain cols and rows), maybe we must call some setup to the user so they know what to provide (but then its just a standard itnegrator + also care with interpolation there), we must provde the fixed states then or so?! think how this should be done

35. Should fast state always be packed `0..nFast-1`?
    - Answer: yes!!! the other packing is used never in the code

36. Should slow state be full vector initially, or also packed slow view?
    - Answer: slow is always full (fast is only partion - think that we can also nest fast in fast maybe later); so like real 3-rate, or 4-rate (maybe for future also)

37. When fast states change, exactly which histories reset: error, interpolation, NLS, controller?
    - Answer: all

38. Can fast-state partition change only at slow-step boundaries?
    - Answer: yes, then we call the multirate interface

39. Should partition strategies be pluggable: error threshold, user mask, heuristic, future ML?
    - Answer: yes eaxactly

40. Should fast RHS always scatter to full only at callback boundary?
    - Answer: not sure whats really best for the users? we have this setp i mention before so not clear whats best for the users

## OpenModelica Adapter

41. Should OpenModelica glue be a separate adapter layer with zero OM structs in core?
    - Answer: OM has a adapter with OM structures that calls GBODE then. that is like dassl.c or what we have already.

42. Should logging be callback-based, or should the core return diagnostics and OM logs them?
    - Answer: logging may be with some logger instance that one may pass, to redirect streams or so, do just very important logs for warnings and errors for now

43. Should event iteration remain entirely in OpenModelica?
    - Answer: ye we need that, but proper interfaces and design when and how its called, currently that is terribly deisnged! GBODE should do root finding and the interpolate to that point. also GBODE should be avble to advance without taking a step (e.g. fi the equdisitnat output of openmodelica is very fine or so.) so like a real intergrator; gbode has root fiding but not really more (gets passed ZC functions with directions or so)

44. Should flags map to a `gb_config` object once at init?
    - Answer: not sure?!

## Testing

45. Which models are oracle tests: Robertson, Brusselator, problem1, MR examples, others?
    - Answer: ye Robertson is fine for now. not more 

46. Compare exact accepted step sequence or only final error/work?
    - Answer: L2 norm

47. Should we create an equivalence mode with current GBODE behavior even if ugly?
    - Answer: no

48. Should new bug fixes be guarded by config until equivalence is proven?
    - Answer: not sure

49. Do we need trace output: chosen estimator, fallback, order, h, error norm, fast set?
    - Answer: waht does that mean, we can do a dump_settings or so at the start.

50. Should the old GBODE remain until new GBODE passes all library tests?
    - Answer: yes; call this UMSOL okay? Unified Multirate Solver (DAE and ODE)

## Additional Notes

Add anything missing here:

