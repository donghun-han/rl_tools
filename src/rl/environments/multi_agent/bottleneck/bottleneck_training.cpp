#include <rl_tools/operations/cpu.h>
#include <rl_tools/rl/environments/multi_agent/bottleneck/operations_cpu.h>
#include <rl_tools/ui_server/client/operations_websocket.h>

namespace rlt = rl_tools;


using DEVICE = rlt::devices::DefaultCPU;
using TI = typename DEVICE::index_t;
using T = float;

struct ENVIRONMENT_PARAMETERS: rlt::rl::environments::multi_agent::bottleneck::DefaultParameters<T, TI>{
    static constexpr T DT = 1/60.0;
    static constexpr TI N_AGENTS = 10;
};
using ENVIRONMENT_SPEC = rlt::rl::environments::multi_agent::bottleneck::Specification<T, TI, ENVIRONMENT_PARAMETERS>;
using ENVIRONMENT = rlt::rl::environments::multi_agent::Bottleneck<ENVIRONMENT_SPEC>;
using OBSERVATION = ENVIRONMENT::Observation;


struct LOOP_CORE_PARAMETERS: rlt::rl::algorithms::ppo::loop::core::DefaultParameters<T, TI, ENVIRONMENT>{
    struct PPO_PARAMETERS: rl::algorithms::ppo::DefaultParameters<T, TI>{
        static constexpr T GAMMA = 0.95;
        static constexpr T ACTION_ENTROPY_COEFFICIENT = 1.0;
        static constexpr TI N_EPOCHS = 2;
    };
    static constexpr TI STEP_LIMIT = 74; // 1024 * 4 * 74 ~ 300k steps

    static constexpr TI ACTOR_HIDDEN_DIM = 64;
    static constexpr TI CRITIC_HIDDEN_DIM = 64;
    static constexpr TI EPISODE_STEP_LIMIT = ENVIRONMENT::EPISODE_STEP_LIMIT;
    static constexpr TI N_ENVIRONMENTS = 4;
    static constexpr TI ON_POLICY_RUNNER_STEPS_PER_ENV = 1024;
    static constexpr TI BATCH_SIZE = 256;
};
using LOOP_CORE_CONFIG = rlt::rl::algorithms::ppo::loop::core::Config<T, TI, RNG, ENVIRONMENT, LOOP_CORE_PARAMETERS, rlt::rl::algorithms::ppo::loop::core::ConfigApproximatorsSequential>;


using ENV_UI = rlt::ui_server::client::UIWebSocket<ENVIRONMENT>;

int main(){
    DEVICE device;
    TI seed = 1;
    auto rng = rlt::random::default_engine(device.random, seed);
    ENVIRONMENT env;
    typename ENVIRONMENT::Parameters env_parameters;
    ENV_UI ui;
    ui.address = "127.0.0.1";
    ui.port = 13337;
    rlt::init(device, env, env_parameters, ui);
    typename ENVIRONMENT::State state, next_state;
    ENVIRONMENT::Parameters parameters;
    rlt::sample_initial_parameters(device, env, parameters, rng);
    rlt::sample_initial_state(device, env, parameters, state, rng);
    rlt::MatrixStatic<rlt::matrix::Specification<T, TI, ENVIRONMENT_PARAMETERS::N_AGENTS, ENVIRONMENT::ACTION_DIM>> action;
    rlt::MatrixStatic<rlt::matrix::Specification<T, TI, ENVIRONMENT_PARAMETERS::N_AGENTS, ENVIRONMENT::Observation::DIM>> observation;
//    rlt::randn(device, action, rng);
    rlt::set_all(device, action, 1);
    rlt::clamp(device, action, -1, 1);
    TI step = 0;
    while(true){
        rlt::randn(device, action, rng);
        rlt::clamp(device, action, -1, 1);
//        if(step > 50){
//            rlt::set_all(device, action, 0);
//        }
        rlt::set_state(device, env, parameters, ui, state, action);
        rlt::observe(device, env, parameters, state, OBSERVATION{}, observation, rng);
        T dt = rlt::step(device, env, parameters, state, action, next_state, rng);
        std::this_thread::sleep_for(std::chrono::duration<T>(dt));
        state = next_state;
        step++;
    }
    return 0;
}
