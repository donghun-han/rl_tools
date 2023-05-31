#include "../parameters.h"


#include <backprop_tools/nn_models/models.h>
#include <backprop_tools/rl/algorithms/td3/td3.h>
#include <backprop_tools/rl/components/off_policy_runner/off_policy_runner.h>

#include <backprop_tools/utils/generic/typing.h>

namespace parameters{
    template<typename T, typename TI, typename ENVIRONMENT>
    struct rl{
        struct ACTOR_CRITIC_PARAMETERS: bpt::rl::algorithms::td3::DefaultParameters<T, TI>{
            static constexpr TI ACTOR_BATCH_SIZE = 256;
            static constexpr TI CRITIC_BATCH_SIZE = 256;
            static constexpr TI CRITIC_TRAINING_INTERVAL = 10;
            static constexpr TI ACTOR_TRAINING_INTERVAL = 20;
            static constexpr TI CRITIC_TARGET_UPDATE_INTERVAL = 10;
            static constexpr TI ACTOR_TARGET_UPDATE_INTERVAL = 20;
            static constexpr T TARGET_NEXT_ACTION_NOISE_CLIP = 1.0;
            static constexpr T TARGET_NEXT_ACTION_NOISE_STD = 0.5;
            static constexpr T GAMMA = 0.99;
            static constexpr bool IGNORE_TERMINATION = false;
        };


        using ACTOR_STRUCTURE_SPEC = bpt::nn_models::mlp::StructureSpecification<T, TI, ENVIRONMENT::OBSERVATION_DIM, ENVIRONMENT::ACTION_DIM, 3, 64, bpt::nn::activation_functions::RELU, bpt::nn::activation_functions::TANH, ACTOR_CRITIC_PARAMETERS::ACTOR_BATCH_SIZE>;
        using CRITIC_STRUCTURE_SPEC = bpt::nn_models::mlp::StructureSpecification<T, TI, ENVIRONMENT::OBSERVATION_DIM + ENVIRONMENT::ACTION_DIM, 1, 3, 64, bpt::nn::activation_functions::RELU, bpt::nn::activation_functions::IDENTITY, ACTOR_CRITIC_PARAMETERS::CRITIC_BATCH_SIZE>;

        using OPTIMIZER_PARAMETERS = typename bpt::nn::optimizers::adam::DefaultParametersTorch<T>;
        using OPTIMIZER = bpt::nn::optimizers::Adam<OPTIMIZER_PARAMETERS>;
        using ACTOR_SPEC = bpt::nn_models::mlp::AdamSpecification<ACTOR_STRUCTURE_SPEC>;
        using ACTOR_TYPE = bpt::nn_models::mlp::NeuralNetworkAdam<ACTOR_SPEC>;

        using ACTOR_TARGET_SPEC = bpt::nn_models::mlp::InferenceSpecification<ACTOR_STRUCTURE_SPEC>;
        using ACTOR_TARGET_TYPE = backprop_tools::nn_models::mlp::NeuralNetwork<ACTOR_TARGET_SPEC>;

        using CRITIC_SPEC = bpt::nn_models::mlp::AdamSpecification<CRITIC_STRUCTURE_SPEC>;
        using CRITIC_TYPE = backprop_tools::nn_models::mlp::NeuralNetworkAdam<CRITIC_SPEC>;

        using CRITIC_TARGET_SPEC = backprop_tools::nn_models::mlp::InferenceSpecification<CRITIC_STRUCTURE_SPEC>;
        using CRITIC_TARGET_TYPE = backprop_tools::nn_models::mlp::NeuralNetwork<CRITIC_TARGET_SPEC>;

        using ACTOR_CRITIC_SPEC = bpt::rl::algorithms::td3::Specification<T, TI, ENVIRONMENT, ACTOR_TYPE, ACTOR_TARGET_TYPE, CRITIC_TYPE, CRITIC_TARGET_TYPE, ACTOR_CRITIC_PARAMETERS>;
        using ActorCriticType = bpt::rl::algorithms::td3::ActorCritic<ACTOR_CRITIC_SPEC>;

        static constexpr TI N_ENVIRONMENTS = 1;
        static constexpr TI REPLAY_BUFFER_CAP = 10000;
        static constexpr TI ENVIRONMENT_STEP_LIMIT = 1000;
        using OFF_POLICY_RUNNER_SPEC = bpt::rl::components::off_policy_runner::Specification<T, TI, ENVIRONMENT, N_ENVIRONMENTS, REPLAY_BUFFER_CAP, ENVIRONMENT_STEP_LIMIT, bpt::rl::components::off_policy_runner::DefaultParameters<T>, true, 1000>;
        using OFF_POLICY_RUNNER_TYPE = bpt::rl::components::OffPolicyRunner<OFF_POLICY_RUNNER_SPEC>;
        static constexpr bpt::rl::components::off_policy_runner::DefaultParameters<T> off_policy_runner_parameters = {
            0.5
        };


        static constexpr TI N_WARMUP_STEPS_CRITIC = 15000;
        static constexpr TI N_WARMUP_STEPS_ACTOR = 30000;
    };
}

namespace parameters_0{
    using parameters::rl;
    using parameters::environment;
}