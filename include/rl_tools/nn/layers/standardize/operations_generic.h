#include "../../../version.h"
#if (defined(RL_TOOLS_DISABLE_INCLUDE_GUARDS) || !defined(RL_TOOLS_NN_LAYERS_STANDARDIZE_OPERATIONS_GENERIC_H)) && (RL_TOOLS_USE_THIS_VERSION == 1)
#pragma once
#define RL_TOOLS_NN_LAYERS_STANDARDIZE_OPERATIONS_GENERIC_H
#include "../../../utils/generic/typing.h"
#include "../../../containers.h"

#include "../../../nn/nn.h"
#include "../../../nn/mode.h"
#include "../../../nn/parameters/parameters.h"
#include "layer.h"

RL_TOOLS_NAMESPACE_WRAPPER_START
namespace rl_tools{
    template <typename DEVICE, typename SPEC>
    void malloc(DEVICE& device, nn::layers::standardize::LayerForward<SPEC>& layer){
        malloc(device, layer.mean);
        malloc(device, layer.precision);
    }
    template <typename DEVICE, typename SPEC>
    void free(DEVICE& device, nn::layers::standardize::LayerForward<SPEC>& layer){
        free(device, layer.mean);
        free(device, layer.precision);
    }
    template <typename DEVICE, typename SPEC>
    void malloc(DEVICE& device, nn::layers::standardize::LayerGradient<SPEC>& layer){
        malloc(device, static_cast<nn::layers::standardize::LayerForward<SPEC>&>(layer));
        malloc(device, layer.output);
    }
    template <typename DEVICE, typename SPEC>
    void free(DEVICE& device, nn::layers::standardize::LayerGradient<SPEC>& layer){
        free(device, static_cast<nn::layers::standardize::LayerForward<SPEC>&>(layer));
        free(device, layer.output);
    }
    template <typename DEVICE>
    void malloc(DEVICE& device, nn::layers::standardize::Buffer& buffer){ }
    template <typename DEVICE>
    void free(DEVICE& device, nn::layers::standardize::Buffer& buffer){ }
    template<typename DEVICE, typename SPEC, typename RNG>
    void init_weights(DEVICE& device, nn::layers::standardize::LayerForward<SPEC>& layer, RNG& rng) {
        set_all(device, layer.mean.parameters, 0);
        set_all(device, layer.precision.parameters, 1);
    }
    template<typename DEVICE, typename LAYER_SPEC, typename MEAN_SPEC, typename STD_SPEC>
    void set_statistics(DEVICE& device, nn::layers::standardize::LayerForward<LAYER_SPEC>& layer, const Matrix<MEAN_SPEC>& mean, const Matrix<STD_SPEC>& std) {
        using T = typename LAYER_SPEC::T;
        using TI = typename DEVICE::index_t;
        using LAYER = nn::layers::standardize::LayerForward<LAYER_SPEC>;
        static_assert(containers::check_structure<MEAN_SPEC, typename LAYER::STATISTICS_CONTAINER_SPEC>);
        static_assert(containers::check_structure<STD_SPEC, typename LAYER::STATISTICS_CONTAINER_SPEC>);
        static_assert(MEAN_SPEC::ROWS == 1);
        copy(device, device, mean, layer.mean.parameters);
        for(TI i=0; i < LAYER_SPEC::INPUT_DIM; i++){
            T precision = 1/get(std, 0, i);
            set(layer.precision.parameters, 0, i, precision);
        }
    }
    template<typename DEVICE, typename LAYER_SPEC, typename INPUT_SPEC, typename OUTPUT_SPEC, typename RNG, typename MODE = nn::mode::Default>
    void evaluate(DEVICE& device, const nn::layers::standardize::LayerForward<LAYER_SPEC>& layer, const Matrix<INPUT_SPEC>& input, Matrix<OUTPUT_SPEC>& output, nn::layers::standardize::Buffer& buffer, RNG& rng, const nn::Mode<MODE>& mode = nn::Mode<nn::mode::Default>{}) {
        static_assert(nn::layers::standardize::check_input_output<LAYER_SPEC, INPUT_SPEC, OUTPUT_SPEC>);
        using T = typename LAYER_SPEC::T;
        using TI = typename DEVICE::index_t;
        constexpr TI BATCH_SIZE = INPUT_SPEC::ROWS;
        for(TI batch_i=0; batch_i < BATCH_SIZE; batch_i++){
            for(TI output_i = 0; output_i < LAYER_SPEC::OUTPUT_DIM; output_i++) {
                T mean = get(layer.mean.parameters, 0, output_i);
                T precision = get(layer.precision.parameters, 0, output_i);
                T input_value = get(input, batch_i, output_i);
                T output_value = (input_value - mean) * precision;
                set(output, batch_i, output_i, output_value);
            }
        }
    }
    template<typename DEVICE, typename LAYER_SPEC, typename INPUT_SPEC, typename OUTPUT_SPEC, typename RNG, typename MODE = nn::mode::Default>
    void forward(DEVICE& device, nn::layers::standardize::LayerBackward<LAYER_SPEC>& layer, const Matrix<INPUT_SPEC>& input, Matrix<OUTPUT_SPEC>& output, RNG& rng, const nn::Mode<MODE>& mode = nn::Mode<nn::mode::Default>{}){
        static_assert(nn::layers::standardize::check_input_output<LAYER_SPEC, INPUT_SPEC, OUTPUT_SPEC>);
        nn::layers::standardize::Buffer buffer;
        evaluate(device, layer, input, output, buffer, rng);
    }
    template<typename DEVICE, typename LAYER_SPEC, typename INPUT_SPEC, typename RNG, typename MODE = nn::mode::Default>
    void forward(DEVICE& device, nn::layers::standardize::LayerGradient<LAYER_SPEC>& layer, const Matrix<INPUT_SPEC>& input, RNG& rng, const nn::Mode<MODE>& mode = nn::Mode<nn::mode::Default>{}){
        static_assert(nn::layers::standardize::check_input_output<LAYER_SPEC, INPUT_SPEC, typename decltype(layer.output)::SPEC>);
        forward(device, layer, input, layer.output, rng);
    }
//    template<typename DEVICE, typename LAYER_SPEC, typename INPUT_SPEC, typename OUTPUT_SPEC, typename RNG>
//    void forward(DEVICE& device, nn::layers::standardize::LayerGradient<LAYER_SPEC>& layer, const Matrix<INPUT_SPEC>& input, Matrix<OUTPUT_SPEC>& output, RNG& rng) {
//        static_assert(nn::layers::standardize::check_input_output<LAYER_SPEC, INPUT_SPEC, OUTPUT_SPEC>);
//        forward(device, layer, input, layer.output, rng);
//        copy(device, device, layer.output, output);
//    }

    template<typename DEVICE, typename LAYER_SPEC, typename D_OUTPUT_SPEC, typename D_INPUT_SPEC, typename MODE = nn::mode::Default>
    void backward_input(DEVICE& device, nn::layers::standardize::LayerBackward<LAYER_SPEC>& layer, const Matrix<D_OUTPUT_SPEC>& d_output, Matrix<D_INPUT_SPEC>& d_input, nn::layers::standardize::Buffer& buffer, const nn::Mode<MODE>& mode = nn::Mode<nn::mode::Default>{}){
        static_assert(nn::layers::standardize::check_input_output<LAYER_SPEC, D_INPUT_SPEC, D_OUTPUT_SPEC>);
        static_assert(nn::layers::standardize::check_input_output<LAYER_SPEC, D_INPUT_SPEC, D_OUTPUT_SPEC>);
        static_assert(LAYER_SPEC::INPUT_DIM == LAYER_SPEC::OUTPUT_DIM);
        using TI = typename DEVICE::index_t;
        constexpr TI INPUT_DIM = LAYER_SPEC::INPUT_DIM;
        constexpr TI OUTPUT_DIM = LAYER_SPEC::OUTPUT_DIM;
        constexpr TI BATCH_SIZE = D_OUTPUT_SPEC::ROWS;
        using T = typename LAYER_SPEC::T;

        for(TI batch_i=0; batch_i < BATCH_SIZE; batch_i++){
            for(TI output_i = 0; output_i < OUTPUT_DIM; output_i++) {
//                out = (in - mu) * prec
                T d_output_value = get(d_output, batch_i, output_i);
                T precision = get(layer.precision.parameters, 0, output_i);
                T d_input_value = d_output_value * precision;
                set(d_input, batch_i, output_i, d_input_value);
            }
        }
    }

    template<typename DEVICE, typename LAYER_SPEC, typename INPUT_SPEC, typename D_OUTPUT_SPEC, typename MODE = nn::mode::Default>
    void backward(DEVICE& device, nn::layers::standardize::LayerGradient<LAYER_SPEC>& layer, const Matrix<INPUT_SPEC>& input, Matrix<D_OUTPUT_SPEC>& d_output, nn::layers::standardize::Buffer& buffer, const nn::Mode<MODE>& mode = nn::Mode<nn::mode::Default>{}) {
        // this is a no-op as the standardize layer does not have trainable parameters
    }

    template<typename DEVICE, typename LAYER_SPEC, typename INPUT_SPEC, typename D_OUTPUT_SPEC, typename D_INPUT_SPEC, typename MODE = nn::mode::Default>
    void backward_full(DEVICE& device, nn::layers::standardize::LayerGradient<LAYER_SPEC>& layer, const Matrix<INPUT_SPEC>& input, Matrix<D_OUTPUT_SPEC>& d_output, Matrix<D_INPUT_SPEC>& d_input, nn::layers::standardize::Buffer& buffer, const nn::Mode<MODE>& mode = nn::Mode<nn::mode::Default>{}) {
        // this is the same as the standardize layer does not have trainable parameters
        backward_input(device, layer, d_output, d_input, buffer);
    }
    template<typename DEVICE, typename SPEC>
    void zero_gradient(DEVICE& device, nn::layers::standardize::LayerGradient<SPEC>& layer) {
        // this is a no-op as the standardize layer does not have trainable parameters
    }
    template<typename DEVICE, typename SPEC, typename OPTIMIZER>
    void update(DEVICE& device, nn::layers::standardize::LayerGradient<SPEC>& layer, OPTIMIZER& optimizer){
        // this is a no-op as the standardize layer does not have trainable parameters
    }

    template<typename DEVICE, typename SPEC, typename OPTIMIZER>
    void _reset_optimizer_state(DEVICE& device, nn::layers::standardize::LayerGradient<SPEC>& layer, OPTIMIZER& optimizer) {
        // this is a no-op as the standardize layer does not have trainable parameters
    }
    template<typename SOURCE_DEVICE, typename TARGET_DEVICE, typename SOURCE_SPEC, typename TARGET_SPEC>
    void copy(SOURCE_DEVICE& source_device, TARGET_DEVICE& target_device, const  nn::layers::standardize::LayerForward<SOURCE_SPEC>& source, nn::layers::standardize::LayerForward<TARGET_SPEC>& target){
        static_assert(nn::layers::standardize::check_compatibility<SOURCE_SPEC, TARGET_SPEC>);
        copy(source_device, target_device, source.mean, target.mean);
        copy(source_device, target_device, source.precision, target.precision);
    }
    template<typename SOURCE_DEVICE, typename TARGET_DEVICE, typename SOURCE_SPEC, typename TARGET_SPEC>
    void copy(SOURCE_DEVICE& source_device, TARGET_DEVICE& target_device, const nn::layers::standardize::LayerGradient<SOURCE_SPEC>& source, nn::layers::standardize::LayerGradient<TARGET_SPEC>& target){
        static_assert(nn::layers::standardize::check_compatibility<SOURCE_SPEC, TARGET_SPEC>);
        copy(source_device, target_device, static_cast<const nn::layers::standardize::LayerForward<SOURCE_SPEC>&>(source), static_cast<nn::layers::standardize::LayerForward<TARGET_SPEC>&>(target));
        copy(source_device, target_device, source.output, target.output);
    }
    template <typename DEVICE, typename SPEC_1, typename SPEC_2>
    typename SPEC_1::T abs_diff(DEVICE& device, const rl_tools::nn::layers::standardize::LayerForward<SPEC_1>& l1, const rl_tools::nn::layers::standardize::LayerForward<SPEC_2>& l2) {
        static_assert(nn::layers::standardize::check_compatibility<SPEC_1, SPEC_2>);
        using T = typename SPEC_1::T;
        T acc = 0;
        acc += abs_diff(device, l1.mean, l2.mean);
        acc += abs_diff(device, l1.precision, l2.precision);
        return acc;
    }
}
RL_TOOLS_NAMESPACE_WRAPPER_END

#endif