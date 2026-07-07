#include "umsol/method.h"

const UmsolRkMethod *umsol_method_as_rk(const UmsolMethod *method)
{
  if (!method || method->family != UMSOL_METHOD_FAMILY_RK)
  {
    return 0;
  }
  return (const UmsolRkMethod *)method->impl;
}

const UmsolBdfMethod *umsol_method_as_bdf(const UmsolMethod *method)
{
  if (!method || method->family != UMSOL_METHOD_FAMILY_BDF)
  {
    return 0;
  }
  return 0;
}

const UmsolRosenbrockMethod *umsol_method_as_rosenbrock(const UmsolMethod *method)
{
  if (!method || method->family != UMSOL_METHOD_FAMILY_ROSENBROCK)
  {
    return 0;
  }
  return 0;
}

void umsol_method_destroy(UmsolMethod *method)
{
  if (!method)
  {
    return;
  }
  if (method->ops && method->ops->destroy)
  {
    method->ops->destroy(method);
    return;
  }
}
