#include "../version.h"
#if (defined(BACKPROP_TOOLS_DISABLE_INCLUDE_GUARDS) || !defined(BACKPROP_TOOLS_DEVICES_CPU_ACCELERATE_H)) && (BACKPROP_TOOLS_USE_THIS_VERSION == 1)
#pragma once
#define BACKPROP_TOOLS_DEVICES_CPU_ACCELERATE_H

#include "../utils/generic/typing.h"
#include "devices.h"

#include "cpu_blas.h"

BACKPROP_TOOLS_NAMESPACE_WRAPPER_START
namespace backprop_tools::devices{
    template <typename T_SPEC>
    struct CPU_ACCELERATE: CPU_BLAS<T_SPEC>{
        static constexpr DeviceId DEVICE_ID = DeviceId::CPU_ACCELERATE;
    };
    using DefaultCPU_ACCELERATE = CPU_ACCELERATE<DefaultCPUSpecification>;
}
BACKPROP_TOOLS_NAMESPACE_WRAPPER_END

#endif
