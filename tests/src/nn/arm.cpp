#include <rl_tools/operations/cpu/group_1.h>
#include <rl_tools/operations/arm/group_1.h>
#include <rl_tools/operations/cpu/group_2.h>
#include <rl_tools/operations/arm/group_2.h>
#include <rl_tools/operations/cpu/group_3.h>
#include <rl_tools/operations/arm/group_3.h>

#include <rl_tools/containers/matrix/persist_code.h>
#include <rl_tools/nn/layers/dense/operations_arm/opt.h>
#include <rl_tools/nn/layers/dense/operations_cpu.h>
#include <rl_tools/nn_models/mlp/operations_generic.h>

namespace rlt = RL_TOOLS_NAMESPACE_WRAPPER ::rl_tools;

#include <gtest/gtest.h>
#include <iostream>
#include <fstream>
#include <filesystem>
//#include "../../../data/test_rl_tools_nn_models_mlp_persist_code.h"

constexpr bool const_declaration = false;


template <typename DTYPE, auto INPUT_DIM, auto OUTPUT_DIM, auto N_LAYERS, auto HIDDEN_DIM, rlt::nn::activation_functions::ActivationFunction HIDDEN_ACTIVATION_FUNCTION, rlt::nn::activation_functions::ActivationFunction ACTIVATION_FUNCTION, auto BATCH_SIZE>
void test_mlp_evaluate() {
    using DEVICE = rlt::devices::DefaultCPU;
    using DEVICE_ARM = rlt::devices::DefaultARM;
    using TI = typename DEVICE::index_t;
    DEVICE device;
    DEVICE_ARM device_arm;
    auto rng = rlt::random::default_engine(DEVICE::SPEC::RANDOM());
    using SPEC = rlt::nn_models::mlp::Specification<DTYPE, typename DEVICE::index_t, INPUT_DIM, OUTPUT_DIM, N_LAYERS, HIDDEN_DIM, HIDDEN_ACTIVATION_FUNCTION, ACTIVATION_FUNCTION, rlt::nn::layers::dense::DefaultInitializer<DTYPE, TI>, rlt::MatrixDynamicTag, true, rlt::matrix::layouts::RowMajorAlignment<typename DEVICE::index_t, 1>>;
    using MLP = rlt::nn_models::mlp::NeuralNetwork<rlt::nn::layer_capability::Forward, SPEC>;
    MLP mlp;
    typename MLP::template Buffer<BATCH_SIZE> buffers;

    rlt::malloc(device, mlp);
    rlt::malloc(device, buffers);
    rlt::init_weights(device, mlp, rng);

    rlt::MatrixDynamic<rlt::matrix::Specification<DTYPE, typename DEVICE::index_t, BATCH_SIZE, SPEC::INPUT_DIM>> input;
    rlt::MatrixDynamic<rlt::matrix::Specification<DTYPE, typename DEVICE::index_t, BATCH_SIZE, SPEC::OUTPUT_DIM>> output_orig, output_arm;
    rlt::malloc(device, input);
    rlt::malloc(device, output_orig);
    rlt::malloc(device, output_arm);
    rlt::randn(device, input, rng);
    rlt::evaluate(device, mlp, input, output_orig, buffers, rng);
    rlt::evaluate(device_arm, mlp, input, output_arm, buffers, rng);
    rlt::print(device, output_orig);

    auto abs_diff = rlt::abs_diff(device, output_orig, output_arm);

    ASSERT_LT(abs_diff, 1e-5);
}TEST(RL_TOOLS_NN_ARM, TEST_MLP_EVALUATE) {
    test_mlp_evaluate<double, 13, 4, 3, 64, rlt::nn::activation_functions::ActivationFunction::RELU, rlt::nn::activation_functions::ActivationFunction::IDENTITY, 1>();
    test_mlp_evaluate<double, 1, 4, 3, 64, rlt::nn::activation_functions::ActivationFunction::RELU, rlt::nn::activation_functions::ActivationFunction::IDENTITY, 1>();
    test_mlp_evaluate<double, 13, 1, 3, 64, rlt::nn::activation_functions::ActivationFunction::RELU, rlt::nn::activation_functions::ActivationFunction::IDENTITY, 1>();
#ifndef _MSC_VER // msvc does not allow zero-sized arrays (hidden_layers are 0 if n layers = 2)
    test_mlp_evaluate<double, 1, 1, 2, 1, rlt::nn::activation_functions::ActivationFunction::RELU, rlt::nn::activation_functions::ActivationFunction::IDENTITY, 1>();
    test_mlp_evaluate<double, 13, 4, 2, 64, rlt::nn::activation_functions::ActivationFunction::RELU, rlt::nn::activation_functions::ActivationFunction::IDENTITY, 1>();
#endif
    test_mlp_evaluate<double, 13, 4, 3, 1, rlt::nn::activation_functions::ActivationFunction::RELU, rlt::nn::activation_functions::ActivationFunction::IDENTITY, 1>();
    test_mlp_evaluate<double, 13, 4, 30, 64, rlt::nn::activation_functions::ActivationFunction::RELU, rlt::nn::activation_functions::ActivationFunction::IDENTITY, 1>();
    test_mlp_evaluate<double, 13, 4, 3, 64, rlt::nn::activation_functions::ActivationFunction::RELU, rlt::nn::activation_functions::ActivationFunction::IDENTITY, 1>();
    test_mlp_evaluate<double, 13, 4, 3, 64, rlt::nn::activation_functions::ActivationFunction::IDENTITY, rlt::nn::activation_functions::ActivationFunction::RELU, 1>();
    test_mlp_evaluate<double, 13, 4, 3, 64, rlt::nn::activation_functions::ActivationFunction::RELU, rlt::nn::activation_functions::ActivationFunction::RELU, 1>();
}

template <typename DTYPE, auto INPUT_DIM, auto OUTPUT_DIM, auto N_LAYERS, auto HIDDEN_DIM, rlt::nn::activation_functions::ActivationFunction HIDDEN_ACTIVATION_FUNCTION, rlt::nn::activation_functions::ActivationFunction ACTIVATION_FUNCTION, auto BATCH_SIZE>
void test_mlp_forward() {
    using DEVICE = rlt::devices::DefaultCPU;
    using DEVICE_ARM = rlt::devices::DefaultARM;
    DEVICE device;
    DEVICE_ARM device_arm;
    using TI = typename DEVICE::index_t;
    auto rng = rlt::random::default_engine(DEVICE::SPEC::RANDOM());
    using SPEC = rlt::nn_models::mlp::Specification<DTYPE, typename DEVICE::index_t, INPUT_DIM, OUTPUT_DIM, N_LAYERS, HIDDEN_DIM, HIDDEN_ACTIVATION_FUNCTION, ACTIVATION_FUNCTION, rlt::nn::layers::dense::DefaultInitializer<DTYPE, TI>, rlt::MatrixDynamicTag, true, rlt::matrix::layouts::RowMajorAlignment<typename DEVICE::index_t, 1>>;
    using TYPE = rlt::nn_models::mlp::NeuralNetwork<rlt::nn::layer_capability::Gradient<rlt::nn::parameters::Adam, BATCH_SIZE>, SPEC>;
    using FORWARD_BACKWARD_BUFFERS = typename TYPE::template Buffer<BATCH_SIZE>;
    TYPE mlp_cpu, mlp_arm;
    FORWARD_BACKWARD_BUFFERS buffers;
    rlt::malloc(device, mlp_cpu);
    rlt::malloc(device, mlp_arm);
    rlt::malloc(device, buffers);
    rlt::init_weights(device, mlp_cpu, rng);
    rlt::zero_gradient(device, mlp_cpu);
    rlt::copy(device, device, mlp_cpu, mlp_arm);

    rlt::MatrixDynamic<rlt::matrix::Specification<DTYPE, typename DEVICE::index_t, BATCH_SIZE, SPEC::INPUT_DIM>> input;
    rlt::malloc(device, input);
    rlt::randn(device, input, rng);
    rlt::forward(device, mlp_cpu, input, buffers, rng);
    rlt::forward(device_arm, mlp_arm, input, buffers, rng);
    rlt::print(device, mlp_arm.output_layer.output);

    auto abs_diff_output = rlt::abs_diff(device, mlp_arm.output_layer.output, mlp_arm.output_layer.output);
    auto abs_diff_network = rlt::abs_diff(device, mlp_arm, mlp_cpu);

    ASSERT_LT(abs_diff_output, 1e-5);
    ASSERT_LT(abs_diff_network, 1e-5);
}

TEST(RL_TOOLS_NN_ARM, TEST_MLP_FORWARD){
    test_mlp_forward<double, 13, 4, 3, 64, rlt::nn::activation_functions::ActivationFunction::RELU, rlt::nn::activation_functions::ActivationFunction::IDENTITY, 1>();
    test_mlp_forward<double, 1, 4, 3, 64, rlt::nn::activation_functions::ActivationFunction::RELU, rlt::nn::activation_functions::ActivationFunction::IDENTITY, 1>();
    test_mlp_forward<double, 13, 1, 3, 64, rlt::nn::activation_functions::ActivationFunction::RELU, rlt::nn::activation_functions::ActivationFunction::IDENTITY, 1>();
#ifndef _MSC_VER // msvc does not allow zero-sized arrays (hidden_layers are 0 if n layers = 2)
    test_mlp_forward<double, 1, 1, 2, 1, rlt::nn::activation_functions::ActivationFunction::RELU, rlt::nn::activation_functions::ActivationFunction::IDENTITY, 1>();
    test_mlp_forward<double, 13, 4, 2, 64, rlt::nn::activation_functions::ActivationFunction::RELU, rlt::nn::activation_functions::ActivationFunction::IDENTITY, 1>();
#endif
    test_mlp_forward<double, 13, 4, 3, 1, rlt::nn::activation_functions::ActivationFunction::RELU, rlt::nn::activation_functions::ActivationFunction::IDENTITY, 1>();
    test_mlp_forward<double, 13, 4, 30, 64, rlt::nn::activation_functions::ActivationFunction::RELU, rlt::nn::activation_functions::ActivationFunction::IDENTITY, 1>();
    test_mlp_forward<double, 13, 4, 3, 64, rlt::nn::activation_functions::ActivationFunction::RELU, rlt::nn::activation_functions::ActivationFunction::IDENTITY, 1>();
    test_mlp_forward<double, 13, 4, 3, 64, rlt::nn::activation_functions::ActivationFunction::IDENTITY, rlt::nn::activation_functions::ActivationFunction::RELU, 1>();
    test_mlp_forward<double, 13, 4, 3, 64, rlt::nn::activation_functions::ActivationFunction::RELU, rlt::nn::activation_functions::ActivationFunction::RELU, 1>();
}
