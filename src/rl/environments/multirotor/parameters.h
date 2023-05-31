#include <backprop_tools/rl/environments/multirotor/parameters/reward_functions/abs_exp.h>
#include <backprop_tools/rl/environments/multirotor/parameters/reward_functions/squared.h>
#include <backprop_tools/rl/environments/multirotor/parameters/reward_functions/default.h>
#include <backprop_tools/rl/environments/multirotor/parameters/dynamics/crazy_flie.h>
#include <backprop_tools/rl/environments/multirotor/parameters/init/default.h>
#include <backprop_tools/rl/environments/multirotor/parameters/termination/default.h>

namespace parameters{
    namespace bpt = backprop_tools;
    template<typename T, typename TI>
    struct environment{
        static constexpr auto reward_function = backprop_tools::rl::environments::multirotor::parameters::reward_functions::reward_old_but_gold<T>;
//        static constexpr auto reward_function = backprop_tools::rl::environments::multirotor::parameters::reward_functions::reward_mm<T, TI>;
//        static constexpr auto reward_function = backprop_tools::rl::environments::multirotor::parameters::reward_functions::reward_squared_2<T>;
//        static constexpr auto reward_function = backprop_tools::rl::environments::multirotor::parameters::reward_functions::reward_squared_4<T>;
        using REWARD_FUNCTION_CONST = typename backprop_tools::utils::typing::remove_cv_t<decltype(reward_function)>;
        using REWARD_FUNCTION = typename backprop_tools::utils::typing::remove_cv<REWARD_FUNCTION_CONST>::type;

        static constexpr backprop_tools::rl::environments::multirotor::ParametersBase<T, TI, 4, REWARD_FUNCTION> parameters = {
                backprop_tools::rl::environments::multirotor::parameters::dynamics::crazy_flie_medium_inertia<T, TI, REWARD_FUNCTION>,
                {0.01}, // integration dt
                {
                        backprop_tools::rl::environments::multirotor::parameters::init::all_around<T, TI, 4, REWARD_FUNCTION>,
                        reward_function,
                        {   // Observation noise
                            0, // position
                            0, // orientation
                            0, // linear_velocity
                            0, // angular_velocity
                        },
//                        backprop_tools::rl::environments::multirotor::parameters::init::all_around_simplified<T, TI, 4, REWARD_FUNCTION>,
//                        backprop_tools::rl::environments::multirotor::parameters::init::simple<T, TI, 4, REWARD_FUNCTION>,
                        backprop_tools::rl::environments::multirotor::parameters::termination::classic<T, TI, 4, REWARD_FUNCTION>
                }
        };

        using PARAMETERS = typename backprop_tools::utils::typing::remove_cv_t<decltype(parameters)>;

        struct ENVIRONMENT_STATIC_PARAMETERS: bpt::rl::environments::multirotor::StaticParametersDefault{
            static constexpr bool ENFORCE_POSITIVE_QUATERNION = false;
            static constexpr bool RANDOMIZE_QUATERNION_SIGN = false;
            static constexpr bpt::rl::environments::multirotor::StateType STATE_TYPE = bpt::rl::environments::multirotor::StateType::Base;
            static constexpr bpt::rl::environments::multirotor::ObservationType OBSERVATION_TYPE = bpt::rl::environments::multirotor::ObservationType::RotationMatrix;
        };

        using ENVIRONMENT_SPEC = bpt::rl::environments::multirotor::Specification<T, TI, PARAMETERS, ENVIRONMENT_STATIC_PARAMETERS>;
        using ENVIRONMENT = bpt::rl::environments::Multirotor<ENVIRONMENT_SPEC>;
    };


}