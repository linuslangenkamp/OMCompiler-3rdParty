#ifndef UMSOL_LOGGER_H
#define UMSOL_LOGGER_H

#ifdef __cplusplus
extern "C"
{
#endif

typedef enum UmsolLogLevel
{
  UMSOL_LOG_DEBUG = 0,
  UMSOL_LOG_INFO = 1,
  UMSOL_LOG_WARNING = 2,
  UMSOL_LOG_ERROR = 3
} UmsolLogLevel;

typedef void (*UmsolLogFunction)(void *ctx, UmsolLogLevel level, const char *message);

typedef struct UmsolLogger
{
  void *ctx;
  UmsolLogFunction log;
} UmsolLogger;

#ifdef __cplusplus
}
#endif

#endif
