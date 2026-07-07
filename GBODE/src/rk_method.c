#include "umsol/rk.h"

#include "umsol/linalg.h"

#include <stdlib.h>

typedef struct UmsolFixedMethodState
{
  UmsolInt placeholder;
} UmsolFixedMethodState;

static const char *umsol_rk_name(const UmsolMethod *method)
{
  const UmsolRkMethod *rk = umsol_method_as_rk(method);
  return rk ? rk->name : "rk";
}

static UmsolStatus umsol_rk_init(UmsolMethod *method, const UmsolConfig *config)
{
  (void)method;
  (void)config;
  return UMSOL_OK;
}

static UmsolStatus umsol_rk_create_state(const UmsolMethod *method, const UmsolConfig *config,
                                         UmsolMethodState **state)
{
  UmsolFixedMethodState *created;

  (void)method;
  (void)config;

  if (!state)
  {
    return UMSOL_INVALID_ARGUMENT;
  }

  *state = 0;
  created = (UmsolFixedMethodState *)calloc(1, sizeof(UmsolFixedMethodState));
  if (!created)
  {
    return UMSOL_OUT_OF_MEMORY;
  }

  *state = (UmsolMethodState *)created;
  return UMSOL_OK;
}

static void umsol_rk_reset_state(const UmsolMethod *method, UmsolMethodState *state, UmsolInt reason)
{
  (void)method;
  (void)state;
  (void)reason;
}

static void umsol_rk_destroy_state(const UmsolMethod *method, UmsolMethodState *state)
{
  (void)method;
  free(state);
}

static void umsol_rk_destroy(UmsolMethod *method)
{
  UmsolRkMethod *rk;

  if (!method)
  {
    return;
  }

  rk = (UmsolRkMethod *)method->impl;
  if (rk)
  {
    free(rk->A);
    free(rk->b);
    free(rk->b_embedded);
    free(rk->c);
    free(rk);
  }
  free(method);
}

static const UmsolMethodOps umsol_rk_ops = {
    umsol_rk_name, umsol_rk_init, umsol_rk_create_state, umsol_rk_reset_state, umsol_rk_destroy_state,
    umsol_rk_destroy};

static UmsolStatus umsol_copy_coefficients(UmsolInt n, const UmsolReal *src, UmsolReal **dst)
{
  if (!dst || n < 0)
  {
    return UMSOL_INVALID_ARGUMENT;
  }

  *dst = 0;
  if (n == 0)
  {
    return UMSOL_OK;
  }
  if (!src)
  {
    return UMSOL_INVALID_CONFIG;
  }

  *dst = (UmsolReal *)calloc(n, sizeof(UmsolReal));
  if (!*dst)
  {
    return UMSOL_OUT_OF_MEMORY;
  }

  umsol_vec_copy(n, src, (UmsolInt)1, *dst, (UmsolInt)1);
  return UMSOL_OK;
}

UmsolStatus umsol_method_create_rk(const UmsolRkMethodSpec *spec, UmsolMethod **method)
{
  UmsolStatus status;
  UmsolMethod *created;
  UmsolRkMethod *rk;

  if (!spec || !method)
  {
    return UMSOL_INVALID_ARGUMENT;
  }
  if (!spec->name || spec->n_stages <= 0 || spec->order <= 0)
  {
    return UMSOL_INVALID_CONFIG;
  }
  if (!spec->A || !spec->b || !spec->c)
  {
    return UMSOL_INVALID_CONFIG;
  }

  *method = 0;

  created = (UmsolMethod *)calloc(1, sizeof(UmsolMethod));
  rk = (UmsolRkMethod *)calloc(1, sizeof(UmsolRkMethod));
  if (!created || !rk)
  {
    free(created);
    free(rk);
    return UMSOL_OUT_OF_MEMORY;
  }

  rk->name = spec->name;
  rk->structure = spec->structure;
  rk->n_stages = spec->n_stages;
  rk->order = spec->order;
  rk->embedded_order = spec->embedded_order;

  status = umsol_copy_coefficients(spec->n_stages * spec->n_stages, spec->A, &rk->A);
  if (status == UMSOL_OK)
  {
    status = umsol_copy_coefficients(spec->n_stages, spec->b, &rk->b);
  }
  if (status == UMSOL_OK)
  {
    status = umsol_copy_coefficients(spec->n_stages, spec->c, &rk->c);
  }
  if (status == UMSOL_OK && spec->b_embedded)
  {
    status = umsol_copy_coefficients(spec->n_stages, spec->b_embedded, &rk->b_embedded);
  }
  if (status != UMSOL_OK)
  {
    free(rk->A);
    free(rk->b);
    free(rk->b_embedded);
    free(rk->c);
    free(rk);
    free(created);
    return status;
  }

  created->family = UMSOL_METHOD_FAMILY_RK;
  created->ops = &umsol_rk_ops;
  created->impl = rk;
  *method = created;
  return UMSOL_OK;
}

UmsolStatus umsol_method_create_rk_bogacki_shampine_3_2(UmsolMethod **method)
{
  static const UmsolReal A[16] = {
      0.0, 0.0, 0.0, 0.0,
      0.5, 0.0, 0.0, 0.0,
      0.0, 0.75, 0.0, 0.0,
      2.0 / 9.0, 1.0 / 3.0, 4.0 / 9.0, 0.0};
  static const UmsolReal b[4] = {2.0 / 9.0, 1.0 / 3.0, 4.0 / 9.0, 0.0};
  static const UmsolReal bhat[4] = {7.0 / 24.0, 0.25, 1.0 / 3.0, 0.125};
  static const UmsolReal c[4] = {0.0, 0.5, 0.75, 1.0};
  const UmsolRkMethodSpec spec = {
      "bogacki_shampine_3_2", UMSOL_RK_EXPLICIT, 4, 3, 2, A, b, bhat, c};
  return umsol_method_create_rk(&spec, method);
}

UmsolStatus umsol_method_create_rk_classical_4(UmsolMethod **method)
{
  static const UmsolReal A[16] = {
      0.0, 0.0, 0.0, 0.0,
      0.5, 0.0, 0.0, 0.0,
      0.0, 0.5, 0.0, 0.0,
      0.0, 0.0, 1.0, 0.0};
  static const UmsolReal b[4] = {1.0 / 6.0, 1.0 / 3.0, 1.0 / 3.0, 1.0 / 6.0};
  static const UmsolReal c[4] = {0.0, 0.5, 0.5, 1.0};
  const UmsolRkMethodSpec spec = {"classical_rk4", UMSOL_RK_EXPLICIT, 4, 4, 0, A, b, 0, c};
  return umsol_method_create_rk(&spec, method);
}
