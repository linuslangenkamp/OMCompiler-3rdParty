#ifndef UMSOL_WORKSPACE_H
#define UMSOL_WORKSPACE_H

#include "umsol/status.h"
#include "umsol/types.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct UmsolWorkspace UmsolWorkspace;

UmsolStatus umsol_workspace_create(UmsolInt real_capacity, UmsolInt int_capacity, UmsolWorkspace **workspace);
void umsol_workspace_destroy(UmsolWorkspace *workspace);
void umsol_workspace_reset(UmsolWorkspace *workspace);
UmsolStatus umsol_workspace_take_real(UmsolWorkspace *workspace, UmsolInt count, UmsolReal **ptr);
UmsolStatus umsol_workspace_take_int(UmsolWorkspace *workspace, UmsolInt count, UmsolInt **ptr);
UmsolInt umsol_workspace_real_capacity(const UmsolWorkspace *workspace);
UmsolInt umsol_workspace_int_capacity(const UmsolWorkspace *workspace);

#ifdef __cplusplus
}
#endif

#endif
