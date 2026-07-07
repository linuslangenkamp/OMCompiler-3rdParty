#include "umsol/workspace.h"

#include "internal/common.h"

struct UmsolWorkspace
{
  UmsolReal *real_data;
  UmsolInt real_capacity;
  UmsolInt real_used;

  UmsolInt *int_data;
  UmsolInt int_capacity;
  UmsolInt int_used;
};

UmsolStatus umsol_workspace_create(UmsolInt real_capacity, UmsolInt int_capacity, UmsolWorkspace **workspace)
{
  UmsolWorkspace *created;

  if (!workspace || real_capacity < 0 || int_capacity < 0)
  {
    return UMSOL_INVALID_ARGUMENT;
  }

  *workspace = 0;
  created = (UmsolWorkspace *)calloc(1, sizeof(UmsolWorkspace));
  if (!created)
  {
    return UMSOL_OUT_OF_MEMORY;
  }

  if (real_capacity > 0)
  {
    created->real_data = (UmsolReal *)calloc(real_capacity, sizeof(UmsolReal));
    if (!created->real_data)
    {
      free(created);
      return UMSOL_OUT_OF_MEMORY;
    }
  }

  if (int_capacity > 0)
  {
    created->int_data = (UmsolInt *)calloc(int_capacity, sizeof(UmsolInt));
    if (!created->int_data)
    {
      free(created->real_data);
      free(created);
      return UMSOL_OUT_OF_MEMORY;
    }
  }

  created->real_capacity = real_capacity;
  created->int_capacity = int_capacity;
  *workspace = created;
  return UMSOL_OK;
}

void umsol_workspace_destroy(UmsolWorkspace *workspace)
{
  if (!workspace)
  {
    return;
  }

  free(workspace->real_data);
  free(workspace->int_data);
  free(workspace);
}

void umsol_workspace_reset(UmsolWorkspace *workspace)
{
  if (!workspace)
  {
    return;
  }

  workspace->real_used = 0;
  workspace->int_used = 0;
}

UmsolStatus umsol_workspace_take_real(UmsolWorkspace *workspace, UmsolInt count, UmsolReal **ptr)
{
  if (!workspace || !ptr || count < 0)
  {
    return UMSOL_INVALID_ARGUMENT;
  }
  if (workspace->real_used + count > workspace->real_capacity)
  {
    return UMSOL_OUT_OF_MEMORY;
  }

  *ptr = workspace->real_data + workspace->real_used;
  workspace->real_used += count;
  return UMSOL_OK;
}

UmsolStatus umsol_workspace_take_int(UmsolWorkspace *workspace, UmsolInt count, UmsolInt **ptr)
{
  if (!workspace || !ptr || count < 0)
  {
    return UMSOL_INVALID_ARGUMENT;
  }
  if (workspace->int_used + count > workspace->int_capacity)
  {
    return UMSOL_OUT_OF_MEMORY;
  }

  *ptr = workspace->int_data + workspace->int_used;
  workspace->int_used += count;
  return UMSOL_OK;
}

UmsolInt umsol_workspace_real_capacity(const UmsolWorkspace *workspace)
{
  return workspace ? workspace->real_capacity : 0;
}

UmsolInt umsol_workspace_int_capacity(const UmsolWorkspace *workspace)
{
  return workspace ? workspace->int_capacity : 0;
}
