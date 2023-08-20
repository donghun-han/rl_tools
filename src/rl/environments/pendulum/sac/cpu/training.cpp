#include <backprop_tools/operations/cpu_mux.h>
#include <backprop_tools/nn/operations_cpu_mux.h>
//#include <backprop_tools/nn_models/output_view/model.h>
#include <backprop_tools/rl/environments/pendulum/operations_cpu.h>
#include <backprop_tools/nn_models/sequential/operations_generic.h>

#include <backprop_tools/rl/algorithms/sac/loop.h>
#ifdef BACKPROP_TOOLS_ENABLE_HDF5
#include <highfive/H5DataSet.hpp>
#include <highfive/H5DataSpace.hpp>
#include <highfive/H5File.hpp>
#endif

#include <chrono>

namespace training_config{
    using namespace backprop_tools::nn_models::sequential::interface;
    struct TrainingConfig{
        using DEV_SPEC = bpt::devices::DefaultCPUSpecification;
//    using DEVICE = bpt::devices::CPU<DEV_SPEC>;
        using DEVICE = bpt::DEVICE_FACTORY<DEV_SPEC>;
        using T = float;
        using TI = typename DEVICE::index_t;

        using PENDULUM_SPEC = bpt::rl::environments::pendulum::Specification<T, TI, bpt::rl::environments::pendulum::DefaultParameters<T>>;
        using ENVIRONMENT = bpt::rl::environments::Pendulum<PENDULUM_SPEC>;
        using UI = bool;

        struct DEVICE_SPEC: bpt::devices::DefaultCPUSpecification {
            using LOGGING = bpt::devices::logging::CPU;
        };
        struct SACPendulumParameters: bpt::rl::algorithms::sac::DefaultParameters<T, DEVICE::index_t, ENVIRONMENT::ACTION_DIM>{
            constexpr static typename DEVICE::index_t CRITIC_BATCH_SIZE = 100;
            constexpr static typename DEVICE::index_t ACTOR_BATCH_SIZE = 100;
        };

        using SAC_PARAMETERS = SACPendulumParameters;


        template <typename PARAMETER_TYPE, template<typename> class LAYER_TYPE = bpt::nn::layers::dense::LayerBackwardGradient>
        struct ACTOR{
            static constexpr TI HIDDEN_DIM = 64;
            static constexpr TI BATCH_SIZE = SAC_PARAMETERS::ACTOR_BATCH_SIZE;
            using LAYER_1_SPEC = bpt::nn::layers::dense::Specification<T, TI, ENVIRONMENT::OBSERVATION_DIM, HIDDEN_DIM, bpt::nn::activation_functions::ActivationFunction::RELU, PARAMETER_TYPE, BATCH_SIZE>;
            using LAYER_1 = LAYER_TYPE<LAYER_1_SPEC>;
            using LAYER_2_SPEC = bpt::nn::layers::dense::Specification<T, TI, HIDDEN_DIM, HIDDEN_DIM, bpt::nn::activation_functions::ActivationFunction::RELU, PARAMETER_TYPE, BATCH_SIZE>;
            using LAYER_2 = LAYER_TYPE<LAYER_2_SPEC>;
            static constexpr TI ACTOR_OUTPUT_DIM = ENVIRONMENT::ACTION_DIM * 2; // to express mean and log_std for each action
            using LAYER_3_SPEC = bpt::nn::layers::dense::Specification<T, TI, HIDDEN_DIM, ACTOR_OUTPUT_DIM, bpt::nn::activation_functions::ActivationFunction::IDENTITY, PARAMETER_TYPE, BATCH_SIZE>; // note the output activation should be identity because we want to sample from a gaussian and then squash afterwards (taking into account the squashing in the distribution)
            using LAYER_3 = LAYER_TYPE<LAYER_3_SPEC>;

            using MODEL = Module<LAYER_1, Module<LAYER_2, Module<LAYER_3>>>;
        };

        template <typename PARAMETER_TYPE, template<typename> class LAYER_TYPE = bpt::nn::layers::dense::LayerBackwardGradient>
        struct CRITIC{
            static constexpr TI HIDDEN_DIM = 64;
            static constexpr TI BATCH_SIZE = SAC_PARAMETERS::CRITIC_BATCH_SIZE;

            using LAYER_1_SPEC = bpt::nn::layers::dense::Specification<T, TI, ENVIRONMENT::OBSERVATION_DIM + ENVIRONMENT::ACTION_DIM, HIDDEN_DIM, bpt::nn::activation_functions::ActivationFunction::RELU, PARAMETER_TYPE, BATCH_SIZE>;
            using LAYER_1 = LAYER_TYPE<LAYER_1_SPEC>;
            using LAYER_2_SPEC = bpt::nn::layers::dense::Specification<T, TI, HIDDEN_DIM, HIDDEN_DIM, bpt::nn::activation_functions::ActivationFunction::RELU, PARAMETER_TYPE, BATCH_SIZE>;
            using LAYER_2 = LAYER_TYPE<LAYER_2_SPEC>;
            using LAYER_3_SPEC = bpt::nn::layers::dense::Specification<T, TI, HIDDEN_DIM, 1, bpt::nn::activation_functions::ActivationFunction::IDENTITY, PARAMETER_TYPE, BATCH_SIZE>;
            using LAYER_3 = LAYER_TYPE<LAYER_3_SPEC>;

            using MODEL = Module<LAYER_1, Module<LAYER_2, Module<LAYER_3>>>;
        };

        struct OPTIMIZER_PARAMETERS: bpt::nn::optimizers::adam::DefaultParametersTorch<T, typename DEVICE::index_t>{
            static constexpr T ALPHA = 3e-4;
        };

        using OPTIMIZER = bpt::nn::optimizers::Adam<OPTIMIZER_PARAMETERS>;
        using ALPHA_OPTIMIZER = bpt::nn::optimizers::Adam<OPTIMIZER_PARAMETERS>;

        using ACTOR_TYPE = ACTOR<bpt::nn::parameters::Adam>::MODEL;
        using ACTOR_TARGET_TYPE = ACTOR<bpt::nn::parameters::Adam, bpt::nn::layers::dense::Layer>::MODEL;
        using CRITIC_TYPE = CRITIC<bpt::nn::parameters::Adam>::MODEL;
        using CRITIC_TARGET_TYPE = CRITIC<bpt::nn::parameters::Adam, bpt::nn::layers::dense::Layer>::MODEL;

        using ALPHA_PARAMETER_TYPE = bpt::nn::parameters::Adam;

        using ACTOR_CRITIC_SPEC = bpt::rl::algorithms::sac::Specification<T, DEVICE::index_t, ENVIRONMENT, ACTOR_TYPE, ACTOR_TARGET_TYPE, CRITIC_TYPE, CRITIC_TARGET_TYPE, ALPHA_PARAMETER_TYPE, OPTIMIZER, OPTIMIZER, ALPHA_OPTIMIZER, SAC_PARAMETERS>;
        using ACTOR_CRITIC_TYPE = bpt::rl::algorithms::sac::ActorCritic<ACTOR_CRITIC_SPEC>;

        static constexpr int N_WARMUP_STEPS = ACTOR_CRITIC_TYPE::SPEC::PARAMETERS::ACTOR_BATCH_SIZE;
        static constexpr DEVICE::index_t STEP_LIMIT = 20000; //2 * N_WARMUP_STEPS;
        static constexpr bool DETERMINISTIC_EVALUATION = true;
        static constexpr DEVICE::index_t EVALUATION_INTERVAL = 1000;
        static constexpr TI NUM_EVALUATION_EPISODES = 10;
        static constexpr typename DEVICE::index_t REPLAY_BUFFER_CAP = STEP_LIMIT;
        static constexpr typename DEVICE::index_t ENVIRONMENT_STEP_LIMIT = 200;
        static constexpr bool COLLECT_EPISODE_STATS = true;
        static constexpr DEVICE::index_t EPISODE_STATS_BUFFER_SIZE = 1000;
        using OFF_POLICY_RUNNER_SPEC = bpt::rl::components::off_policy_runner::Specification<
                T,
                DEVICE::index_t,
                ENVIRONMENT,
                1,
                false,
                REPLAY_BUFFER_CAP,
                ENVIRONMENT_STEP_LIMIT,
                bpt::rl::components::off_policy_runner::DefaultParameters<T>,
                true,
                COLLECT_EPISODE_STATS,
                EPISODE_STATS_BUFFER_SIZE
        >;
        static_assert(ACTOR_CRITIC_TYPE::SPEC::PARAMETERS::ACTOR_BATCH_SIZE == ACTOR_CRITIC_TYPE::SPEC::PARAMETERS::CRITIC_BATCH_SIZE);
    };
}
using TrainingConfig = training_config::TrainingConfig;

int main(){
    using T = typename TrainingConfig::T;
    using TI = typename TrainingConfig::TI;
    using DEVICE = typename TrainingConfig::DEVICE;
    TI NUM_RUNS = 20;
#ifdef BACKPROP_TOOLS_ENABLE_HDF5
    std::string DATA_FILE_PATH = "rl_environments_pendulum_sac_learning_curves.h5";
    auto data_file = HighFive::File(DATA_FILE_PATH, HighFive::File::Overwrite);
#endif

    for(TI run_i = 0; run_i < NUM_RUNS; run_i++){
        auto start = std::chrono::high_resolution_clock::now();
        std::cout << "Run: " << run_i << std::endl;
        backprop_tools::rl::algorithms::sac::TrainingState<TrainingConfig> ts;
        TI seed = run_i;
        training_init(ts, seed);
        ts.off_policy_runner.parameters.exploration_noise = 0;
#ifdef BACKPROP_TOOLS_ENABLE_HDF5
        auto run_group = data_file.createGroup(std::to_string(run_i));
#endif
        for(TI step_i=0; step_i < TrainingConfig::STEP_LIMIT; step_i++){
            training_step(ts);
//            if(step_i % 1000 == 0){
////                std::cout << "alpha: " << bpt::math::exp(DEVICE::SPEC::MATH{}, bpt::get(ts.actor_critic.log_alpha.parameters, 0, 0)) << std::endl;
//            }
        }
        std::vector<size_t> dims{decltype(ts)::N_EVALUATIONS};
        std::vector<T> mean_returns;
        for(TI i=0; i < decltype(ts)::N_EVALUATIONS; i++){
            mean_returns.push_back(ts.evaluation_results[i].returns_mean);
        }

        run_group.createDataSet("episode_returns", mean_returns);

        training_destroy(ts);
        auto end = std::chrono::high_resolution_clock::now();
        std::cout << "Run time: " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() / 1000.0 << "s" << std::endl;
    }
    return 0;
}