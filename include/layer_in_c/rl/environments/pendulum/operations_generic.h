#ifndef LAYER_IN_C_RL_ENVIRONMENTS_PENDULUM_OPERATIONS_GENERIC
#define LAYER_IN_C_RL_ENVIRONMENTS_PENDULUM_OPERATIONS_GENERIC
#include "pendulum.h"
namespace layer_in_c::rl::environments::pendulum {
    template <typename T>
    inline T clip(T x, T min, T max){
        x = x < min ? min : (x > max ? max : x);
        return x;
    }
    template <typename T>
    T f_mod_python(T a, T b){
        return a - b * floor(a / b);
    }

    template <typename T>
    T angle_normalize(T x){
//        T temp = std::fmod(std::abs(x) + M_PI, 2*M_PI) - M_PI;
//        temp = x > 0 ? temp : -temp;
//        return temp;
        return f_mod_python((x + M_PI), (2 * M_PI)) - M_PI;
    }
}
namespace layer_in_c{
    template<typename SPEC, typename RNG>
    static void sample_initial_state(const rl::environments::Pendulum<devices::Generic, SPEC>& env, rl::environments::pendulum::State<typename SPEC::T>& state, RNG& rng){
        state.theta     = std::uniform_real_distribution<typename SPEC::T>(SPEC::PARAMETERS::initial_state_min_angle, SPEC::PARAMETERS::initial_state_max_angle)(rng);
        state.theta_dot = std::uniform_real_distribution<typename SPEC::T>(SPEC::PARAMETERS::initial_state_min_speed, SPEC::PARAMETERS::initial_state_max_speed)(rng);
    }
    template<typename SPEC>
    static typename SPEC::T step(const rl::environments::Pendulum<devices::Generic, SPEC>& env, const rl::environments::pendulum::State<typename SPEC::T>& state, const typename SPEC::T action[1], rl::environments::pendulum::State<typename SPEC::T>& next_state) {
        using namespace rl::environments::pendulum;
        typedef typename SPEC::T T;
        typedef typename SPEC::PARAMETERS PARAMS;
        T u_normalised = action[0];
        T u = PARAMS::max_torque * u_normalised;
        T g = PARAMS::g;
        T m = PARAMS::m;
        T l = PARAMS::l;
        T dt = PARAMS::dt;

        u = clip(u, -PARAMS::max_torque, PARAMS::max_torque);

        T newthdot = state.theta_dot + (3 * g / (2 * l) * std::sin(state.theta) + 3.0 / (m * l * l) * u) * dt;
        newthdot = clip(newthdot, -PARAMS::max_speed, PARAMS::max_speed);
        T newth = state.theta + newthdot * dt;

//        while (newth < -M_PI) {
//            newth += M_PI * 2;
//        }
//        while (newth >= M_PI) {
//            newth -= M_PI * 2;
//        }
//
        next_state.theta = newth;
        next_state.theta_dot = newthdot;
        return SPEC::PARAMETERS::dt;
    }
    template<typename SPEC>
    static typename SPEC::T reward(const rl::environments::Pendulum<devices::Generic, SPEC>& env, const rl::environments::pendulum::State<typename SPEC::T>& state, const typename SPEC::T action[1], const rl::environments::pendulum::State<typename SPEC::T>& next_state){
        using namespace rl::environments::pendulum;
        typedef typename SPEC::T T;
        T angle_norm = angle_normalize(state.theta);
        T u_normalised = action[0];
        T u = SPEC::PARAMETERS::max_torque * u_normalised;
        T costs = angle_norm * angle_norm + 0.1 * state.theta_dot * state.theta_dot + 0.001 * (u * u);
        return -costs;
    }

    template<typename SPEC>
    static void observe(const rl::environments::Pendulum<devices::Generic, SPEC>& env, const rl::environments::pendulum::State<typename SPEC::T>& state, typename SPEC::T observation[3]){
        typedef typename SPEC::T T;
        observation[0] = std::cos(state.theta);
        observation[1] = std::sin(state.theta);
        observation[2] = state.theta_dot;
    }
    template<typename SPEC>
    static bool terminated(const rl::environments::Pendulum<devices::Generic, SPEC>& env, const typename rl::environments::pendulum::State<typename SPEC::T> state){
        return false;
    }
}
#endif