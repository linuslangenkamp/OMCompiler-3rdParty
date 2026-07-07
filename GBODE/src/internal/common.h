#ifndef UMSOL_INTERNAL_COMMON_H
#define UMSOL_INTERNAL_COMMON_H

#include "umsol/logger.h"
#include "umsol/status.h"
#include "umsol/types.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void umsol_log(const UmsolLogger *logger, UmsolLogLevel level, const char *message)
{
  if (logger && logger->log)
  {
    logger->log(logger->ctx, level, message);
  }
}

static void umsol_logf(const UmsolLogger *logger, UmsolLogLevel level, const char *format, ...)
{
  char buffer[512];
  va_list args;

  if (!logger || !logger->log)
  {
    return;
  }

  va_start(args, format);
  vsnprintf(buffer, sizeof(buffer), format, args);
  va_end(args);

  logger->log(logger->ctx, level, buffer);
}

static UmsolBool umsol_int_mul_overflows_allocation(UmsolInt a, UmsolInt b)
{
  const UmsolInt max_reasonable = (UmsolInt)(2147483647);

  if (a < 0 || b < 0)
  {
    return UMSOL_TRUE;
  }
  if (a == 0 || b == 0)
  {
    return UMSOL_FALSE;
  }
  return a > max_reasonable / b ? UMSOL_TRUE : UMSOL_FALSE;
}

#endif
