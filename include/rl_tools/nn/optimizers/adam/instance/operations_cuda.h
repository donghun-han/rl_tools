#include "../../../../version.h"
#if (defined(RL_TOOLS_DISABLE_INCLUDE_GUARDS) || !defined(RL_TOOLS_NN_OPTIMIZERS_ADAM_INSTANCE_OPERATIONS_CUDA_H)) && (RL_TOOLS_USE_THIS_VERSION == 1)
#pragma once
#define RL_TOOLS_NN_OPTIMIZERS_ADAM_INSTANCE_OPERATIONS_CUDA_H

#include "../adam.h"
#include "operations_generic.h"
RL_TOOLS_NAMESPACE_WRAPPER_START
namespace rl_tools {
    namespace nn::optimizers::adam::cuda {
        template<typename DEV_SPEC, typename PARAMETER_SPEC, typename SPEC>
        __global__
        void update_kernel(devices::CUDA<DEV_SPEC>& device, nn::parameters::Adam::instance<PARAMETER_SPEC> parameter, nn::optimizers::Adam<SPEC> optimizer) {
            // fully fused adam update
            // note some of this is fused into the Layer update: include/rl_tools/nn/layers/dense/operations_cuda.h
            using DEVICE = devices::CUDA<DEV_SPEC>;
            using T = typename PARAMETER_SPEC::CONTAINER::T;
            using TI = typename DEVICE::index_t;

            TI col_i = blockIdx.x * blockDim.x + threadIdx.x;
            TI row_i = blockIdx.y * blockDim.y + threadIdx.y;
            if(col_i < PARAMETER_SPEC::CONTAINER::COLS && row_i < PARAMETER_SPEC::CONTAINER::ROWS){
                T d_weight = get(parameter.gradient, row_i, col_i);
                T d_weight_first_order_moment = optimizer.parameters.beta_1 * get(parameter.gradient_first_order_moment, row_i, col_i) + (1 - optimizer.parameters.beta_1) * d_weight;
                set(parameter.gradient_first_order_moment, row_i, col_i, d_weight_first_order_moment);
                T d_weight_second_order_moment = optimizer.parameters.beta_2 * get(parameter.gradient_second_order_moment, row_i, col_i) + (1 - optimizer.parameters.beta_2) * d_weight * d_weight;
                set(parameter.gradient_second_order_moment, row_i, col_i, d_weight_second_order_moment);
                T pre_sqrt_term = d_weight_second_order_moment * optimizer.second_order_moment_bias_correction;
                pre_sqrt_term = math::max(device.math, pre_sqrt_term, (T)optimizer.parameters.epsilon_sqrt);
                T parameter_update = optimizer.parameters.alpha * optimizer.first_order_moment_bias_correction * d_weight_first_order_moment / (math::sqrt(typename DEVICE::SPEC::MATH_DEVICE_ACCURATE(), pre_sqrt_term) + optimizer.parameters.epsilon);
                if constexpr(utils::typing::is_same_v<typename PARAMETER_SPEC::CATEGORY_TAG, nn::parameters::categories::Biases> && SPEC::ENABLE_BIAS_LR_FACTOR){
                    parameter_update *= optimizer.parameters.bias_lr_factor;
                }
                if constexpr(utils::typing::is_same_v<typename PARAMETER_SPEC::CATEGORY_TAG, nn::parameters::categories::Weights>){
                    if constexpr(utils::typing::is_same_v<typename PARAMETER_SPEC::GROUP_TAG, nn::parameters::groups::Normal> && SPEC::ENABLE_WEIGHT_DECAY){
                        parameter_update += get(parameter.parameters, row_i, col_i) * optimizer.parameters.weight_decay / 2;
                    }
                    if constexpr(utils::typing::is_same_v<typename PARAMETER_SPEC::GROUP_TAG, nn::parameters::groups::Input> && SPEC::ENABLE_WEIGHT_DECAY){
                        parameter_update += get(parameter.parameters, row_i, col_i) * optimizer.parameters.weight_decay_input / 2;
                    }
                    if constexpr(utils::typing::is_same_v<typename PARAMETER_SPEC::GROUP_TAG, nn::parameters::groups::Output> && SPEC::ENABLE_WEIGHT_DECAY){
                        parameter_update += get(parameter.parameters, row_i, col_i) * optimizer.parameters.weight_decay_output / 2;
                    }
                }
                increment(parameter.parameters, row_i, col_i, -parameter_update);
            }
        }
    }
    template<typename DEV_SPEC, typename SPEC, typename PARAMETERS>
    void update(devices::CUDA<DEV_SPEC>& device, nn::parameters::Adam::instance<SPEC>& p, nn::optimizers::Adam<PARAMETERS>& optimizer) {
        constexpr typename devices::CUDA<DEV_SPEC>::index_t BLOCKSIZE_ACTIVATION_OUTPUT = 32;
        constexpr typename devices::CUDA<DEV_SPEC>::index_t BLOCKSIZE_ACTIVATION_INPUT = 32;
        constexpr typename devices::CUDA<DEV_SPEC>::index_t N_BLOCKS_ACTIVATION_OUTPUT = RL_TOOLS_DEVICES_CUDA_CEIL(SPEC::CONTAINER::ROWS, BLOCKSIZE_ACTIVATION_OUTPUT);
        constexpr typename devices::CUDA<DEV_SPEC>::index_t N_BLOCKS_ACTIVATION_INPUT = RL_TOOLS_DEVICES_CUDA_CEIL(SPEC::CONTAINER::COLS, BLOCKSIZE_ACTIVATION_INPUT);
        dim3 activation_grid(N_BLOCKS_ACTIVATION_INPUT, N_BLOCKS_ACTIVATION_OUTPUT);
        dim3 activation_block(BLOCKSIZE_ACTIVATION_INPUT, BLOCKSIZE_ACTIVATION_OUTPUT);
        nn::optimizers::adam::cuda::update_kernel<<<activation_grid, activation_block, 0, device.stream>>>(device, p, optimizer);
        check_status(device);
    }
}
RL_TOOLS_NAMESPACE_WRAPPER_END

#endif