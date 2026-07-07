cmake_minimum_required(VERSION 3.16)

project(UMSOL
  VERSION 0.1.0
  LANGUAGES C)

option(UMSOL_BUILD_TESTS "Build UMSOL smoke tests" ON)
option(UMSOL_USE_BLAS "Use BLAS for vector kernels when available" ON)
option(UMSOL_USE_LAPACK "Require LAPACK target for future dense linear solvers" ON)
option(UMSOL_USE_KLU "Enable KLU backend once SuiteSparse discovery is wired" OFF)

add_library(umsol STATIC
  src/config.c
  src/linalg.c
  src/method.c
  src/problem.c
  src/rk_method.c
  src/rk_stepper.c
  src/solver.c
  src/status.c
  src/workspace.c)

add_library(umsol::umsol ALIAS umsol)

target_compile_features(umsol PUBLIC c_std_99)

target_include_directories(umsol
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>)

target_compile_definitions(umsol
  PUBLIC
    UMSOL_VERSION_MAJOR=${PROJECT_VERSION_MAJOR}
    UMSOL_VERSION_MINOR=${PROJECT_VERSION_MINOR}
    UMSOL_VERSION_PATCH=${PROJECT_VERSION_PATCH})

if (UMSOL_USE_BLAS)
  find_package(BLAS)
  if (BLAS_FOUND)
    target_compile_definitions(umsol PRIVATE UMSOL_HAVE_BLAS=1)
    if (TARGET BLAS::BLAS)
      target_link_libraries(umsol PUBLIC BLAS::BLAS)
    else ()
      target_link_libraries(umsol PUBLIC ${BLAS_LIBRARIES})
    endif ()
  else ()
    message(WARNING "BLAS was requested but not found. UMSOL will use scalar fallback vector kernels.")
  endif ()
endif ()

if (UMSOL_USE_LAPACK)
  find_package(LAPACK)
  if (LAPACK_FOUND)
    if (TARGET LAPACK::LAPACK)
      target_link_libraries(umsol PUBLIC LAPACK::LAPACK)
    else ()
      target_link_libraries(umsol PUBLIC ${LAPACK_LIBRARIES})
    endif ()
  else ()
    message(WARNING "LAPACK was requested but not found. Dense linear solver backends are not available.")
  endif ()
endif ()

if (UMSOL_USE_KLU)
  message(FATAL_ERROR "UMSOL_USE_KLU is reserved for the first KLU backend implementation.")
endif ()

if (UMSOL_BUILD_TESTS)
  enable_testing()
  add_executable(umsol_smoke tests/smoke.c)
  target_link_libraries(umsol_smoke PRIVATE umsol::umsol)
  add_test(NAME umsol_smoke COMMAND umsol_smoke)
endif ()
