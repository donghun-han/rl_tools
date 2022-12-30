#include "td3.h"

#include <random>
#include <layer_in_c/rl/components/replay_buffer/replay_buffer.h>
#include <layer_in_c/nn/nn.h>
#include <layer_in_c/nn_models/operations_generic.h>
#include <layer_in_c/utils/generic/polyak.h>

namespace layer_in_c{
    template<typename SPEC>
    void update_target_layer(lic::nn::layers::dense::Layer<lic::devices::Generic, SPEC>& target, const lic::nn::layers::dense::Layer<lic::devices::Generic, SPEC>& source, typename SPEC::T polyak) {
        lic::utils::polyak::update_matrix<typename SPEC::T, SPEC::OUTPUT_DIM, SPEC::INPUT_DIM>(lic::devices::Generic(), target.weights, source.weights, polyak);
        lic::utils::polyak::update       <typename SPEC::T, SPEC::OUTPUT_DIM                 >(lic::devices::Generic(), target.biases , source.biases , polyak);
    }
    template<typename T, typename TARGET_NETWORK_TYPE, typename SOURCE_NETWORK_TYPE>
    void update_target_network(TARGET_NETWORK_TYPE& target, const SOURCE_NETWORK_TYPE& source, T polyak) {
        update_target_layer(target.layer_1, source.layer_1, polyak);
        update_target_layer(target.layer_2, source.layer_2, polyak);
        update_target_layer(target.output_layer, source.output_layer, polyak);
    }

    template <typename DEVICE, typename SPEC>
    void update_targets(lic::rl::algorithms::td3::ActorCritic<DEVICE, SPEC>& actor_critic) {
        update_target_network(actor_critic.actor_target   , actor_critic.   actor, SPEC::PARAMETERS::ACTOR_POLYAK);
        update_target_network(actor_critic.critic_target_1, actor_critic.critic_1, SPEC::PARAMETERS::CRITIC_POLYAK);
        update_target_network(actor_critic.critic_target_2, actor_critic.critic_2, SPEC::PARAMETERS::CRITIC_POLYAK);

    }


    template <typename DEVICE, typename SPEC, auto RANDOM_UNIFORM, typename RNG>
    void init(lic::rl::algorithms::td3::ActorCritic<DEVICE, SPEC>& actor_critic, RNG& rng){
        layer_in_c::init_weights<typename lic::rl::algorithms::td3::ActorCritic<DEVICE, SPEC>:: ACTOR_NETWORK_SPEC, RANDOM_UNIFORM, RNG>(actor_critic.actor, rng);
        layer_in_c::init_weights<typename lic::rl::algorithms::td3::ActorCritic<DEVICE, SPEC>::CRITIC_NETWORK_SPEC, RANDOM_UNIFORM, RNG>(actor_critic.critic_1, rng);
        layer_in_c::init_weights<typename lic::rl::algorithms::td3::ActorCritic<DEVICE, SPEC>::CRITIC_NETWORK_SPEC, RANDOM_UNIFORM, RNG>(actor_critic.critic_2, rng);
        layer_in_c::reset_optimizer_state(actor_critic.actor);
        layer_in_c::reset_optimizer_state(actor_critic.critic_1);
        layer_in_c::reset_optimizer_state(actor_critic.critic_2);

        actor_critic.actor_target = actor_critic.actor;
        actor_critic.critic_target_1 = actor_critic.critic_1;
        actor_critic.critic_target_2 = actor_critic.critic_2;
    }

    template <typename DEVICE, typename SPEC, typename CRITIC_TYPE, int CAPACITY, typename RNG, bool DETERMINISTIC=false>
    typename SPEC::T train_critic(lic::rl::algorithms::td3::ActorCritic<DEVICE, SPEC>& actor_critic, CRITIC_TYPE& critic, lic::rl::algorithms::td3::ReplayBuffer<typename SPEC::T, SPEC::ENVIRONMENT::OBSERVATION_DIM, SPEC::ENVIRONMENT::ACTION_DIM, CAPACITY>& replay_buffer, typename SPEC::T target_next_action_noise[SPEC::PARAMETERS::CRITIC_BATCH_SIZE][SPEC::ENVIRONMENT::ACTION_DIM], RNG& rng) {
        typedef typename SPEC::T T;
        assert(replay_buffer.full || replay_buffer.position >= SPEC::PARAMETERS::CRITIC_BATCH_SIZE);
        T loss = 0;
        lic::zero_gradient(critic);
        std::uniform_int_distribution<uint32_t> sample_distribution(0, (replay_buffer.full ? CAPACITY : replay_buffer.position) - 1);
        for (int batch_step_i=0; batch_step_i < SPEC::PARAMETERS::CRITIC_BATCH_SIZE; batch_step_i++){
            uint32_t sample_index = DETERMINISTIC ? batch_step_i : sample_distribution(rng);
            T next_state_action_value_input[SPEC::ENVIRONMENT::OBSERVATION_DIM + SPEC::ENVIRONMENT::ACTION_DIM];
            memcpy(next_state_action_value_input, replay_buffer.next_observations[sample_index], sizeof(T) * SPEC::ENVIRONMENT::OBSERVATION_DIM); // setting the first part with next observations
            lic::evaluate(actor_critic.actor_target, next_state_action_value_input, &next_state_action_value_input[SPEC::ENVIRONMENT::OBSERVATION_DIM]); // setting the second part with next actions
            for(int action_i=0; action_i < SPEC::ENVIRONMENT::ACTION_DIM; action_i++){
                T noisy_next_action = next_state_action_value_input[SPEC::ENVIRONMENT::OBSERVATION_DIM + action_i] + target_next_action_noise[batch_step_i][action_i];
                noisy_next_action = std::clamp<T>(noisy_next_action, -1, 1);
                next_state_action_value_input[SPEC::ENVIRONMENT::OBSERVATION_DIM + action_i] = noisy_next_action;
            }
            T next_state_action_value_critic_1 = lic::evaluate(actor_critic.critic_target_1, next_state_action_value_input);
            T next_state_action_value_critic_2 = lic::evaluate(actor_critic.critic_target_2, next_state_action_value_input);

            T min_next_state_action_value = std::min(
                    next_state_action_value_critic_1,
                    next_state_action_value_critic_2
            );
            T state_action_value_input[SPEC::ENVIRONMENT::OBSERVATION_DIM + SPEC::ENVIRONMENT::ACTION_DIM];
            memcpy(state_action_value_input, replay_buffer.observations[sample_index], sizeof(T) * SPEC::ENVIRONMENT::OBSERVATION_DIM); // setting the first part with the current observation
            memcpy(&state_action_value_input[SPEC::ENVIRONMENT::OBSERVATION_DIM], replay_buffer.actions[sample_index], sizeof(T) * SPEC::ENVIRONMENT::ACTION_DIM); // setting the first part with the current action
//        standardise<T,  OBSERVATION_DIM>(X_train[batch_i * batch_size + sample_i].data(), X_mean.data(), X_std.data(), input);
//        standardise<T, ACTION_DIM>(Y_train[batch_i * batch_size + sample_i].data(), Y_mean.data(), Y_std.data(), output);
            T target_action_value[1] = {replay_buffer.rewards[sample_index] + SPEC::PARAMETERS::GAMMA * min_next_state_action_value * (!replay_buffer.terminated[sample_index])};

            lic::forward_backward_mse<typename CRITIC_TYPE::SPEC, SPEC::PARAMETERS::CRITIC_BATCH_SIZE>(critic, state_action_value_input, target_action_value);
            static_assert(lic::rl::algorithms::td3::ActorCritic<DEVICE, SPEC>::CRITIC_NETWORK_TYPE::SPEC::OUTPUT_LAYER::SPEC::ACTIVATION_FUNCTION == lic::nn::activation_functions::IDENTITY); // Ensuring the critic output activation is identity so that we can just use the pre_activations to get the loss value
            T loss_sample = lic::nn::loss_functions::mse<T, 1, SPEC::PARAMETERS::CRITIC_BATCH_SIZE>(critic.output_layer.pre_activations, target_action_value);
            loss += loss_sample;
        }
        lic::update(critic);
        return loss;
    }
    template <typename DEVICE, typename SPEC, typename CRITIC_TYPE, int CAPACITY, typename RNG>
    typename SPEC::T train_critic_deterministic(lic::rl::algorithms::td3::ActorCritic<DEVICE, SPEC>& actor_critic, CRITIC_TYPE& critic, lic::rl::algorithms::td3::ReplayBuffer<typename SPEC::T, SPEC::ENVIRONMENT::OBSERVATION_DIM, SPEC::ENVIRONMENT::ACTION_DIM, CAPACITY>& replay_buffer, typename SPEC::T target_next_action_noise[SPEC::PARAMETERS::CRITIC_BATCH_SIZE][SPEC::ENVIRONMENT::ACTION_DIM], RNG& rng) {
        return train_critic<DEVICE, SPEC, CRITIC_TYPE, CAPACITY, RNG, true>(actor_critic, critic, replay_buffer, target_next_action_noise, rng);
    }
    template <typename DEVICE, typename SPEC, typename CRITIC_TYPE, int CAPACITY, typename RNG>
    typename SPEC::T train_critic(lic::rl::algorithms::td3::ActorCritic<DEVICE, SPEC>& actor_critic, CRITIC_TYPE& critic, lic::rl::algorithms::td3::ReplayBuffer<typename SPEC::T, SPEC::ENVIRONMENT::OBSERVATION_DIM, SPEC::ENVIRONMENT::ACTION_DIM, CAPACITY>& replay_buffer, RNG& rng) {
        typedef typename SPEC::T T;
        std::normal_distribution<T> target_next_action_noise_distribution(0, SPEC::PARAMETERS::TARGET_NEXT_ACTION_NOISE_STD);
        T action_noise[SPEC::PARAMETERS::CRITIC_BATCH_SIZE][SPEC::ENVIRONMENT::ACTION_DIM];
        for(int batch_sample_i=0; batch_sample_i < SPEC::PARAMETERS::CRITIC_BATCH_SIZE; batch_sample_i++){
            for(int action_i=0; action_i < SPEC::ENVIRONMENT::ACTION_DIM; action_i++){
                action_noise[batch_sample_i][action_i] = std::clamp(
                        target_next_action_noise_distribution(rng),
                        -SPEC::PARAMETERS::TARGET_NEXT_ACTION_NOISE_CLIP,
                        SPEC::PARAMETERS::TARGET_NEXT_ACTION_NOISE_CLIP
                );
            }
        }
        return train_critic(actor_critic, critic, replay_buffer, action_noise, rng);
    }

    template <typename DEVICE, typename SPEC, int CAPACITY, typename RNG, bool DETERMINISTIC = false>
    typename SPEC::T train_actor(lic::rl::algorithms::td3::ActorCritic<DEVICE, SPEC>& actor_critic, lic::rl::algorithms::td3::ReplayBuffer<typename SPEC::T, SPEC::ENVIRONMENT::OBSERVATION_DIM, SPEC::ENVIRONMENT::ACTION_DIM, CAPACITY>& replay_buffer, RNG& rng) {
        typedef typename SPEC::T T;
        typedef typename SPEC::PARAMETERS PARAMETERS;
        typedef typename SPEC::ENVIRONMENT ENVIRONMENT;
        T actor_value = 0;
        lic::zero_gradient(actor_critic.actor);
        std::uniform_int_distribution<uint32_t> sample_distribution(0, (replay_buffer.full ? CAPACITY : replay_buffer.position) - 1);
        for (int sample_i=0; sample_i < PARAMETERS::ACTOR_BATCH_SIZE; sample_i++){
            uint32_t sample_index = DETERMINISTIC ? sample_i : sample_distribution(rng);
            T state_action_value_input[ENVIRONMENT::OBSERVATION_DIM + ENVIRONMENT::ACTION_DIM];
            memcpy(state_action_value_input, replay_buffer.observations[sample_index], sizeof(T) * ENVIRONMENT::OBSERVATION_DIM); // setting the first part with next observations
            lic::forward(actor_critic.actor, state_action_value_input, &state_action_value_input[ENVIRONMENT::OBSERVATION_DIM]);

//            typename lic::rl::algorithms::td3::ActorCritic<DEVICE, SPEC>::CRITIC_TARGET_NETWORK_TYPE& critic = actor_critic.critic_target_1;
            auto& critic = actor_critic.critic_1;
            T critic_output = lic::forward_univariate(critic, state_action_value_input);
            actor_value += critic_output/SPEC::PARAMETERS::ACTOR_BATCH_SIZE;
            T d_output[1] = {-(T)1/SPEC::PARAMETERS::ACTOR_BATCH_SIZE}; // we want to maximise the critic output using gradient descent
            T d_critic_input[ENVIRONMENT::OBSERVATION_DIM + ENVIRONMENT::ACTION_DIM];
            lic::backward(critic, state_action_value_input, d_output, d_critic_input);
            T d_actor_input[ENVIRONMENT::OBSERVATION_DIM];
            lic::backward(actor_critic.actor, state_action_value_input, &d_critic_input[ENVIRONMENT::OBSERVATION_DIM], d_actor_input);
        }
        lic::update(actor_critic.actor);
        return actor_value;
    }

}