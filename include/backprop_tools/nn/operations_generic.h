#include "../version.h"
#if (defined(BACKPROP_TOOLS_DISABLE_INCLUDE_GUARDS) || !defined(BACKPROP_TOOLS_NN_OPERATIONS_GENERIC_H)) && (BACKPROP_TOOLS_USE_THIS_VERSION == 1)
#pragma once
#define BACKPROP_TOOLS_NN_OPERATIONS_GENERIC_H
#ifndef BACKPROP_TOOLS_FUNCTION_PLACEMENT
#define BACKPROP_TOOLS_FUNCTION_PLACEMENT
#endif

/*
 * Generic operations can run on the CPU or GPU depending on the setting of the BACKPROP_TOOLS_FUNCTION_PLACEMENT macro.
 */

#include "layers/operations_generic.h"
#include "loss_functions/operations_generic.h"


#endif