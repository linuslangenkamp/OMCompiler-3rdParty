#include "umsol/status.h"

const char *umsol_status_string(UmsolStatus status)
{
  switch (status)
  {
  case UMSOL_OK:
    return "UMSOL_OK";
  case UMSOL_EVENT_FOUND:
    return "UMSOL_EVENT_FOUND";
  case UMSOL_OUTPUT_READY:
    return "UMSOL_OUTPUT_READY";
  case UMSOL_STEP_REJECTED:
    return "UMSOL_STEP_REJECTED";
  case UMSOL_STEP_FAILED:
    return "UMSOL_STEP_FAILED";
  case UMSOL_CALLBACK_FAILED:
    return "UMSOL_CALLBACK_FAILED";
  case UMSOL_OUT_OF_MEMORY:
    return "UMSOL_OUT_OF_MEMORY";
  case UMSOL_INVALID_ARGUMENT:
    return "UMSOL_INVALID_ARGUMENT";
  case UMSOL_INVALID_CONFIG:
    return "UMSOL_INVALID_CONFIG";
  case UMSOL_NOT_IMPLEMENTED:
    return "UMSOL_NOT_IMPLEMENTED";
  default:
    return "UMSOL_UNKNOWN_STATUS";
  }
}
