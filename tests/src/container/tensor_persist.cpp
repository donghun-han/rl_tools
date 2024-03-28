#include <gtest/gtest.h>
#include <rl_tools/operations/cpu.h>
#include <rl_tools/containers/tensor/operations_generic.h>
#include <rl_tools/containers/tensor/operations_cpu.h>
#include <rl_tools/containers/tensor/persist.h>

namespace rlt = rl_tools;


#include "../utils/utils.h"


constexpr double EPSILON = 1e-8;



TEST(RL_TOOLS_CONTAINERS_TENSOR_PERSIST, LOAD_GRU){
    using DEVICE = rlt::devices::DefaultCPU;
    using T = double;
    using TI = DEVICE::index_t;
    DEVICE device;
    constexpr T EPSILON = 1e-6;
    constexpr TI SEQUENCE_LENGTH = 50;
    constexpr TI BATCH_SIZE = 128;
    constexpr TI INPUT_DIM = 1;
    constexpr TI OUTPUT_DIM = 1;
    constexpr TI HIDDEN_DIM = 16;
    using INPUT_SHAPE = rlt::tensor::Shape<TI, SEQUENCE_LENGTH, BATCH_SIZE, INPUT_DIM>;
    rlt::Tensor<rlt::tensor::Specification<T, TI, INPUT_SHAPE>> input;
    using OUTPUT_SHAPE = rlt::tensor::Shape<TI, SEQUENCE_LENGTH, BATCH_SIZE, OUTPUT_DIM>;
    rlt::Tensor<rlt::tensor::Specification<T, TI, INPUT_SHAPE>> output;
    using WI_SHAPE = rlt::tensor::Shape<TI, HIDDEN_DIM*3, INPUT_DIM>;
    rlt::Tensor<rlt::tensor::Specification<T, TI, WI_SHAPE>> weight_in, weight_in_grad;
    using BI_SHAPE = rlt::tensor::Shape<TI, HIDDEN_DIM*3>;
    rlt::Tensor<rlt::tensor::Specification<T, TI, BI_SHAPE>> bias_in, bias_in_grad;
    using WH_SHAPE = rlt::tensor::Shape<TI, HIDDEN_DIM*3, HIDDEN_DIM>;
    rlt::Tensor<rlt::tensor::Specification<T, TI, WH_SHAPE>> weight_hidden, weight_hidden_grad;
    using BH_SHAPE = rlt::tensor::Shape<TI, HIDDEN_DIM*3>;
    rlt::Tensor<rlt::tensor::Specification<T, TI, BH_SHAPE>> bias_hidden, bias_hidden_grad;
    using WOUT_SHAPE = rlt::tensor::Shape<TI, OUTPUT_DIM, HIDDEN_DIM>;
    rlt::Tensor<rlt::tensor::Specification<T, TI, WOUT_SHAPE>> weight_out, weight_out_grad;
    using BOUT_SHAPE = rlt::tensor::Shape<TI, OUTPUT_DIM>;
    rlt::Tensor<rlt::tensor::Specification<T, TI, BOUT_SHAPE>> bias_out, bias_out_grad;


    rlt::malloc(device, input);
    rlt::malloc(device, output);
    rlt::malloc(device, weight_in);
    rlt::malloc(device, weight_in_grad);
    rlt::malloc(device, bias_in);
    rlt::malloc(device, bias_in_grad);
    rlt::malloc(device, weight_hidden);
    rlt::malloc(device, weight_hidden_grad);
    rlt::malloc(device, bias_hidden);
    rlt::malloc(device, bias_hidden_grad);
    rlt::malloc(device, weight_out);
    rlt::malloc(device, weight_out_grad);
    rlt::malloc(device, bias_out);
    rlt::malloc(device, bias_out_grad);

    auto W_ir = rlt::view_range(device, weight_in, 0*HIDDEN_DIM, rlt::tensor::ViewSpec<0, HIDDEN_DIM>{});
    auto W_iz = rlt::view_range(device, weight_in, 1*HIDDEN_DIM, rlt::tensor::ViewSpec<0, HIDDEN_DIM>{});
    auto W_in = rlt::view_range(device, weight_in, 2*HIDDEN_DIM, rlt::tensor::ViewSpec<0, HIDDEN_DIM>{});

    auto b_ir = rlt::view_range(device, bias_in, 0*HIDDEN_DIM, rlt::tensor::ViewSpec<0, HIDDEN_DIM>{});
    auto b_iz = rlt::view_range(device, bias_in, 1*HIDDEN_DIM, rlt::tensor::ViewSpec<0, HIDDEN_DIM>{});
    auto b_in = rlt::view_range(device, bias_in, 2*HIDDEN_DIM, rlt::tensor::ViewSpec<0, HIDDEN_DIM>{});

    auto W_hr = rlt::view_range(device, weight_hidden, 0*HIDDEN_DIM, rlt::tensor::ViewSpec<0, HIDDEN_DIM>{});
    auto W_hz = rlt::view_range(device, weight_hidden, 1*HIDDEN_DIM, rlt::tensor::ViewSpec<0, HIDDEN_DIM>{});
    auto W_hn = rlt::view_range(device, weight_hidden, 2*HIDDEN_DIM, rlt::tensor::ViewSpec<0, HIDDEN_DIM>{});

    auto b_hr = rlt::view_range(device, bias_hidden, 0*HIDDEN_DIM, rlt::tensor::ViewSpec<0, HIDDEN_DIM>{});
    auto b_hz = rlt::view_range(device, bias_hidden, 1*HIDDEN_DIM, rlt::tensor::ViewSpec<0, HIDDEN_DIM>{});
    auto b_hn = rlt::view_range(device, bias_hidden, 2*HIDDEN_DIM, rlt::tensor::ViewSpec<0, HIDDEN_DIM>{});

    auto W_out = rlt::view_range(device, weight_out, 0*OUTPUT_DIM, rlt::tensor::ViewSpec<0, OUTPUT_DIM>{});
    auto b_out = rlt::view_range(device, bias_out, 0*OUTPUT_DIM, rlt::tensor::ViewSpec<0, OUTPUT_DIM>{});

    std::string DATA_FILE_NAME = "gru_training_trace.h5";
    const char *data_path_stub = RL_TOOLS_MACRO_TO_STR(RL_TOOLS_TESTS_DATA_PATH);
    std::string DATA_FILE_PATH = std::string(data_path_stub) + "/" + DATA_FILE_NAME;
    std::cout << "DATA_FILE_PATH: " << DATA_FILE_PATH << std::endl;
    auto output_file = HighFive::File(std::string(DATA_FILE_PATH), HighFive::File::ReadOnly);
    for(auto epoch_group_name : output_file.listObjectNames()){
        auto epoch_group = output_file.getGroup(epoch_group_name);
        for(auto batch_group_name: epoch_group.listObjectNames()){
            auto batch_group = epoch_group.getGroup(batch_group_name);
            auto input_ds = batch_group.getDataSet("input");
            auto output_ds = batch_group.getDataSet("output");
            rlt::load(device, input_ds, input);
            rlt::load(device, output_ds, output);
            auto input_slice = rlt::view_range(device, input, 0, rlt::tensor::ViewSpec<0, 1>{});
            std::vector<std::vector<std::vector<T>>> input_data;
            input_ds.read(input_data);
            for(TI i=0; i < SEQUENCE_LENGTH; i++){
                for(TI j=0; j < BATCH_SIZE; j++){
                    for(TI k=0; k < INPUT_DIM; k++){
                        T diff = rlt::math::abs(device.math, static_cast<T>(input_data[i][j][k]) - static_cast<T>(rlt::get(device, input, i, j, k)));
                        ASSERT_LT(diff, EPSILON);
                    }
                }
            }
            std::vector<std::vector<std::vector<T>>> output_data;
            output_ds.read(output_data);
            for(TI i=0; i < SEQUENCE_LENGTH; i++){
                for(TI j=0; j < BATCH_SIZE; j++){
                    for(TI k=0; k < OUTPUT_DIM; k++){
                        T diff = rlt::math::abs(device.math, static_cast<T>(output_data[i][j][k]) - static_cast<T>(rlt::get(device, output, i, j, k)));
                        ASSERT_LT(diff, EPSILON);
                    }
                }
            }
            auto weight_group = batch_group.getGroup("weights");
            auto gradient_group = batch_group.getGroup("gradient");
            auto W_ir_ds = weight_group.getDataSet("W_ir");
             rlt::load(device, W_ir_ds, W_ir);
            std::vector<std::vector<T>> W_ir_data;
            W_ir_ds.read(W_ir_data);
            for(TI i=0; i < HIDDEN_DIM; i++){
                for(TI j=0; j < INPUT_DIM; j++){
                    T diff = rlt::math::abs(device.math, static_cast<T>(W_ir_data[i][j]) - static_cast<T>(rlt::get(device, W_ir, i, j)));
                    ASSERT_LT(diff, EPSILON);
                }
            }
            auto W_iz_ds = weight_group.getDataSet("W_iz");
             rlt::load(device, W_iz_ds, W_iz);
            std::vector<std::vector<T>> W_iz_data;
            W_iz_ds.read(W_iz_data);
            for(TI i=0; i < HIDDEN_DIM; i++){
                for(TI j=0; j < INPUT_DIM; j++){
                    T diff = rlt::math::abs(device.math, static_cast<T>(W_iz_data[i][j]) - static_cast<T>(rlt::get(device, W_iz, i, j)));
                    ASSERT_LT(diff, EPSILON);
                }
            }
            auto W_in_ds = weight_group.getDataSet("W_in");
             rlt::load(device, W_in_ds, W_in);
            std::vector<std::vector<T>> W_in_data;
            W_in_ds.read(W_in_data);
            for(TI i=0; i < HIDDEN_DIM; i++){
                for(TI j=0; j < INPUT_DIM; j++){
                    T diff = rlt::math::abs(device.math, static_cast<T>(W_in_data[i][j]) - static_cast<T>(rlt::get(device, W_in, i, j)));
                    ASSERT_LT(diff, EPSILON);
                }
            }
            auto W_hr_ds = weight_group.getDataSet("W_hr");
             rlt::load(device, W_hr_ds, W_hr);
            std::vector<std::vector<T>> W_hr_data;
            W_hr_ds.read(W_hr_data);
            for(TI i=0; i < HIDDEN_DIM; i++){
                for(TI j=0; j < HIDDEN_DIM; j++){
                    T diff = rlt::math::abs(device.math, static_cast<T>(W_hr_data[i][j]) - static_cast<T>(rlt::get(device, W_hr, i, j)));
                    ASSERT_LT(diff, EPSILON);
                }
            }
            auto W_hz_ds = weight_group.getDataSet("W_hz");
             rlt::load(device, W_hz_ds, W_hz);
            std::vector<std::vector<T>> W_hz_data;
            W_hz_ds.read(W_hz_data);
            for(TI i=0; i < HIDDEN_DIM; i++){
                for(TI j=0; j < HIDDEN_DIM; j++){
                    T diff = rlt::math::abs(device.math, static_cast<T>(W_hz_data[i][j]) - static_cast<T>(rlt::get(device, W_hz, i, j)));
                    ASSERT_LT(diff, EPSILON);
                }
            }
            auto W_hn_ds = weight_group.getDataSet("W_hn");
             rlt::load(device, W_hn_ds, W_hn);
            std::vector<std::vector<T>> W_hn_data;
            W_hn_ds.read(W_hn_data);
            for(TI i=0; i < HIDDEN_DIM; i++){
                for(TI j=0; j < HIDDEN_DIM; j++){
                    T diff = rlt::math::abs(device.math, static_cast<T>(W_hn_data[i][j]) - static_cast<T>(rlt::get(device, W_hn, i, j)));
                    ASSERT_LT(diff, EPSILON);
                }
            }
            auto b_ir_ds = weight_group.getDataSet("b_ir");
             rlt::load(device, b_ir_ds, b_ir);
            std::vector<T> b_ir_data;
            b_ir_ds.read(b_ir_data);
            for(TI i=0; i < HIDDEN_DIM; i++){
                T diff = rlt::math::abs(device.math, static_cast<T>(b_ir_data[i]) - static_cast<T>(rlt::get(device, b_ir, i)));
                ASSERT_LT(diff, EPSILON);
            }
            auto b_iz_ds = weight_group.getDataSet("b_iz");
             rlt::load(device, b_iz_ds, b_iz);
            std::vector<T> b_iz_data;
            b_iz_ds.read(b_iz_data);
            for(TI i=0; i < HIDDEN_DIM; i++){
                T diff = rlt::math::abs(device.math, static_cast<T>(b_iz_data[i]) - static_cast<T>(rlt::get(device, b_iz, i)));
                ASSERT_LT(diff, EPSILON);
            }
            auto b_in_ds = weight_group.getDataSet("b_in");
             rlt::load(device, b_in_ds, b_in);
            std::vector<T> b_in_data;
            b_in_ds.read(b_in_data);
            for(TI i=0; i < HIDDEN_DIM; i++){
                T diff = rlt::math::abs(device.math, static_cast<T>(b_in_data[i]) - static_cast<T>(rlt::get(device, b_in, i)));
                ASSERT_LT(diff, EPSILON);
            }
            auto b_hr_ds = weight_group.getDataSet("b_hr");
             rlt::load(device, b_hr_ds, b_hr);
            std::vector<T> b_hr_data;
            b_hr_ds.read(b_hr_data);
            for(TI i=0; i < HIDDEN_DIM; i++){
                T diff = rlt::math::abs(device.math, static_cast<T>(b_hr_data[i]) - static_cast<T>(rlt::get(device, b_hr, i)));
                ASSERT_LT(diff, EPSILON);
            }
            auto b_hz_ds = weight_group.getDataSet("b_hz");
             rlt::load(device, b_hz_ds, b_hz);
            std::vector<T> b_hz_data;
            b_hz_ds.read(b_hz_data);
            for(TI i=0; i < HIDDEN_DIM; i++){
                T diff = rlt::math::abs(device.math, static_cast<T>(b_hz_data[i]) - static_cast<T>(rlt::get(device, b_hz, i)));
                ASSERT_LT(diff, EPSILON);
            }
            auto b_hn_ds = weight_group.getDataSet("b_hn");
             rlt::load(device, b_hn_ds, b_hn);
            std::vector<T> b_hn_data;
            b_hn_ds.read(b_hn_data);
            for(TI i=0; i < HIDDEN_DIM; i++){
                T diff = rlt::math::abs(device.math, static_cast<T>(b_hn_data[i]) - static_cast<T>(rlt::get(device, b_hn, i)));
                ASSERT_LT(diff, EPSILON);
            }
            auto W_out_ds = weight_group.getDataSet("W_out");
             rlt::load(device, W_out_ds, W_out);
            std::vector<std::vector<T>> W_out_data;
            W_out_ds.read(W_out_data);
            for(TI i=0; i < OUTPUT_DIM; i++){
                for(TI j=0; j < HIDDEN_DIM; j++){
                    T diff = rlt::math::abs(device.math, static_cast<T>(W_out_data[i][j]) - static_cast<T>(rlt::get(device, W_out, i, j)));
                    ASSERT_LT(diff, EPSILON);
                }
            }
            auto b_out_ds = weight_group.getDataSet("b_out");
             rlt::load(device, b_out_ds, b_out);
            std::vector<T> b_out_data;
            b_out_ds.read(b_out_data);
            for(TI i=0; i < OUTPUT_DIM; i++){
                T diff = rlt::math::abs(device.math, static_cast<T>(b_out_data[i]) - static_cast<T>(rlt::get(device, b_out, i)));
                ASSERT_LT(diff, EPSILON);
            }
        }
    }
}

template <typename DEVICE, typename SHAPE>
bool serialize(DEVICE& device){
    using T = double;
    using TI = typename DEVICE::index_t;
    auto rng = rlt::random::default_engine(device.random);
    rlt::Tensor<rlt::tensor::Specification<T, TI, SHAPE>> tensor, tensor2, diff;
    rlt::malloc(device, tensor);
    rlt::malloc(device, tensor2);
    rlt::malloc(device, diff);
    rlt::randn(device, tensor, rng);
    auto vector_data = rlt::to_vector(device, tensor);
    rlt::from_vector(device, vector_data, tensor2);
    rlt::subtract(device, tensor, tensor2, diff);
    rlt::abs(device, diff);
    T abs_diff = rlt::sum(device, diff);
    return abs_diff < EPSILON;
}
TEST(RL_TOOLS_CONTAINERS_TENSOR_PERSIST, SERIALIZE) {
    using DEVICE = rlt::devices::DefaultCPU;
    DEVICE device;
    using TI = DEVICE::index_t;
    {
        using SHAPE = rlt::tensor::Shape<TI, 10, 2>;
        bool good = serialize<DEVICE, SHAPE>(device);
        ASSERT_TRUE(good);
    }
    {
        using SHAPE = rlt::tensor::Shape<TI, 1>;
        bool good = serialize<DEVICE, SHAPE>(device);
        ASSERT_TRUE(good);
    }
    {
        using SHAPE = rlt::tensor::Shape<TI, 100>;
        bool good = serialize<DEVICE, SHAPE>(device);
        ASSERT_TRUE(good);
    }
    {
        using SHAPE = rlt::tensor::Shape<TI, 100, 101, 110>;
        bool good = serialize<DEVICE, SHAPE>(device);
        ASSERT_TRUE(good);
    }
}

template <typename DEVICE, typename SHAPE, typename DEVICE::index_t VIEW_DIM, typename DEVICE::index_t VIEW_START, typename DEVICE::index_t VIEW_SIZE>
bool serialize_view(DEVICE& device){
    using T = double;
    using TI = typename DEVICE::index_t;
    auto rng = rlt::random::default_engine(device.random);
    rlt::Tensor<rlt::tensor::Specification<T, TI, SHAPE>> generator, tensor;
    rlt::malloc(device, generator);
    rlt::malloc(device, tensor);
    rlt::randn(device, generator, rng);
    auto vector_data = rlt::to_vector(device, generator);
    rlt::from_vector(device, vector_data, tensor);
    auto view = rlt::view_range(device, tensor, VIEW_START, rlt::tensor::ViewSpec<VIEW_DIM, VIEW_SIZE>{});
    auto vector_data_view = rlt::to_vector(device, view);

    for(TI i=(VIEW_DIM == 0 ? VIEW_START : 0); i < (VIEW_DIM == 0 ? VIEW_START + VIEW_SIZE : rlt::get<0>(SHAPE{})); i++){
        for(TI j=(VIEW_DIM == 1 ? VIEW_START : 0); j < (VIEW_DIM == 1 ? VIEW_START + VIEW_SIZE : rlt::get<1>(SHAPE{})); j++){
            for(TI k=(VIEW_DIM == 2 ? VIEW_START : 0); k < (VIEW_DIM == 2 ? VIEW_START + VIEW_SIZE : rlt::get<2>(SHAPE{})); k++){
                T data_value = vector_data[i][j][k];
                T view_value = vector_data_view[i - (VIEW_DIM==0 ? VIEW_START : 0)][j - (VIEW_DIM==1 ? VIEW_START : 0)][k - (VIEW_DIM==2 ? VIEW_START : 0)];
                T diff = data_value - view_value;
                if(rlt::math::abs(device.math, diff) > EPSILON){
                    return false;
                }
            }
        }
    }
    return true;
}
TEST(RL_TOOLS_CONTAINERS_TENSOR_PERSIST, SERIALIZE_VIEW){
    using DEVICE = rlt::devices::DefaultCPU;
    DEVICE device;
    using TI = DEVICE::index_t;
    {
        using SHAPE = rlt::tensor::Shape<TI, 10, 2, 10>;
        constexpr TI VIEW_DIM = 0;
        constexpr TI VIEW_START = 2;
        constexpr TI VIEW_SIZE = 5;
        bool good = serialize_view<DEVICE, SHAPE, VIEW_DIM, VIEW_START, VIEW_SIZE>(device);
        ASSERT_TRUE(good);
    }
    {
        using SHAPE = rlt::tensor::Shape<TI, 10, 20, 10>;
        constexpr TI VIEW_DIM = 1;
        constexpr TI VIEW_START = 0;
        constexpr TI VIEW_SIZE = 5;
        bool good = serialize_view<DEVICE, SHAPE, VIEW_DIM, VIEW_START, VIEW_SIZE>(device);
        ASSERT_TRUE(good);
    }
    {
        using SHAPE = rlt::tensor::Shape<TI, 10, 20, 10>;
        constexpr TI VIEW_DIM = 1;
        constexpr TI VIEW_START = 0;
        constexpr TI VIEW_SIZE = 20;
        bool good = serialize_view<DEVICE, SHAPE, VIEW_DIM, VIEW_START, VIEW_SIZE>(device);
        ASSERT_TRUE(good);
    }
    {
        using SHAPE = rlt::tensor::Shape<TI, 10, 20, 10>;
        constexpr TI VIEW_DIM = 1;
        constexpr TI VIEW_START = 2;
        constexpr TI VIEW_SIZE = 15;
        bool good = serialize_view<DEVICE, SHAPE, VIEW_DIM, VIEW_START, VIEW_SIZE>(device);
        ASSERT_TRUE(good);
    }
    {
        using SHAPE = rlt::tensor::Shape<TI, 10, 20, 10>;
        constexpr TI VIEW_DIM = 2;
        constexpr TI VIEW_START = 0;
        constexpr TI VIEW_SIZE = 10;
        bool good = serialize_view<DEVICE, SHAPE, VIEW_DIM, VIEW_START, VIEW_SIZE>(device);
        ASSERT_TRUE(good);
    }
    {
        using SHAPE = rlt::tensor::Shape<TI, 10, 20, 10>;
        constexpr TI VIEW_DIM = 2;
        constexpr TI VIEW_START = 1;
        constexpr TI VIEW_SIZE = 9;
        bool good = serialize_view<DEVICE, SHAPE, VIEW_DIM, VIEW_START, VIEW_SIZE>(device);
        ASSERT_TRUE(good);
    }
    {
        using SHAPE = rlt::tensor::Shape<TI, 10, 20, 10>;
        constexpr TI VIEW_DIM = 2;
        constexpr TI VIEW_START = 5;
        constexpr TI VIEW_SIZE = 5;
        bool good = serialize_view<DEVICE, SHAPE, VIEW_DIM, VIEW_START, VIEW_SIZE>(device);
        ASSERT_TRUE(good);
    }
}

template <typename DEVICE, typename SHAPE>
bool save_and_load(DEVICE& device){
    using T = double;
    using TI = typename DEVICE::index_t;
    auto rng = rlt::random::default_engine(device.random);
    rlt::Tensor<rlt::tensor::Specification<T, TI, SHAPE>> tensor;
    rlt::malloc(device, tensor);
    rlt::randn(device, tensor, rng);
    std::string DATA_FILE_NAME = "tensor_persist_test_save_load.h5";
    const char *data_path_stub = RL_TOOLS_MACRO_TO_STR(RL_TOOLS_TESTS_DATA_PATH);
    std::string DATA_FILE_PATH = std::string(data_path_stub) + "/" + DATA_FILE_NAME;
    std::cout << "DATA_FILE_PATH: " << DATA_FILE_PATH << std::endl;
    auto output_file = HighFive::File(std::string(DATA_FILE_PATH), HighFive::File::ReadWrite | HighFive::File::Create | HighFive::File::Truncate);
    auto group = output_file.createGroup("test");
    rlt::save(device, tensor, group, "tensor");
    rlt::Tensor<rlt::tensor::Specification<T, TI, SHAPE>> tensor_loaded, diff;
    rlt::malloc(device, tensor_loaded);
    rlt::malloc(device, diff);
    auto dataset = group.getDataSet("tensor");
    rlt::load(device, dataset, tensor_loaded);
    rlt::subtract(device, tensor, tensor_loaded, diff);
    rlt::abs(device, diff);
    T abs_diff = rlt::sum(device, diff);
    return abs_diff < EPSILON;
}

TEST(RL_TOOLS_CONTAINERS_TENSOR_PERSIST, SAVE_AND_LOAD) {
    using DEVICE = rlt::devices::DefaultCPU;
    using T = double;
    using TI = DEVICE::index_t;
    DEVICE device;
    {
        using SHAPE = rlt::tensor::Shape<TI, 1>;
        bool good = save_and_load<DEVICE, SHAPE>(device);
        ASSERT_TRUE(good);
    };
    {
        using SHAPE = rlt::tensor::Shape<TI, 2>;
        bool good = save_and_load<DEVICE, SHAPE>(device);
        ASSERT_TRUE(good);
    };
    {
        using SHAPE = rlt::tensor::Shape<TI, 100, 1>;
        bool good = save_and_load<DEVICE, SHAPE>(device);
        ASSERT_TRUE(good);
    };
    {
        using SHAPE = rlt::tensor::Shape<TI, 100, 10>;
        bool good = save_and_load<DEVICE, SHAPE>(device);
        ASSERT_TRUE(good);
    };
    {
        using SHAPE = rlt::tensor::Shape<TI, 10, 2, 34, 23, 2, 3>;
        bool good = save_and_load<DEVICE, SHAPE>(device);
        ASSERT_TRUE(good);
    };
}