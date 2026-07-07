#ifndef UMSOL_STATUS_H
#define UMSOL_STATUS_H

#ifdef __cplusplus
extern "C"
{
#endif

typedef enum UmsolStatus
{
  UMSOL_OK = 0,
  UMSOL_EVENT_FOUND = 1,
  UMSOL_OUTPUT_READY = 2,
  UMSOL_STEP_REJECTED = 3,
  UMSOL_STEP_FAILED = -1,
  UMSOL_CALLBACK_FAILED = -2,
  UMSOL_OUT_OF_MEMORY = -3,
  UMSOL_INVALID_ARGUMENT = -4,
  UMSOL_INVALID_CONFIG = -5,
  UMSOL_NOT_IMPLEMENTED = -6
} UmsolStatus;

const char *umsol_status_string(UmsolStatus status);

#ifdef __cplusplus
}
#endif

#endif
