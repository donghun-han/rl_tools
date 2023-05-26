
#include "../../multirotor.h"

namespace backprop_tools::rl::environments::multirotor::parameters::dynamics{
    template<typename T, typename TI, typename REWARD_FUNCTION>
    typename Parameters <T, TI, TI(4), REWARD_FUNCTION>::Dynamics mrs = {
            // Rotor positions
            {
                    {
                            0.1202081528017130834795622718047525268048,
                            -0.1202081528017130834795622718047525268048,
                            0.0000000000000000000000000000000000000000
                    },
                    {
                            0.1202081528017130834795622718047525268048,
                            0.1202081528017130834795622718047525268048,
                            0.0000000000000000000000000000000000000000
                    },
                    {

                            -0.1202081528017130834795622718047525268048,
                            0.1202081528017130834795622718047525268048,
                            0.0000000000000000000000000000000000000000
                    },
                    {
                            -0.1202081528017130834795622718047525268048,
                            -0.1202081528017130834795622718047525268048,
                            0.0000000000000000000000000000000000000000
                    }
            },
            // Rotor thrust directions
            {
                    {0, 0, -1},
                    {0, 0, -1},
                    {0, 0, -1},
                    {0, 0, -1},
            },
            // Rotor torque directions
            {
                    {0, 0, -1},
                    {0, 0, 1},
                    {0, 0, -1},
                    {0, 0, 1},
            },
            // thrust constants
            {
                    -1.7689986848125325291647413905593566596508,
                    0.0038360810526746032603218061751704226481,
                    0.0000013298253500372891536393180067499031
            },
            // torque constant
            0.016,
            // mass vehicle
            0.73,
            // gravity
            {0, 0, 9.81},
            // J
            {
                    {
                            0.0079113750000000000045519144009631418157,
                            0.0000000000000000000000000000000000000000,
                            0.0000000000000000000000000000000000000000
                    },
                    {
                            0.0000000000000000000000000000000000000000,
                            0.0079113750000000000045519144009631418157,
                            0.0000000000000000000000000000000000000000
                    },
                    {
                            0.0000000000000000000000000000000000000000,
                            0.0000000000000000000000000000000000000000,
                            0.0123065833333333343041493534997243841644
                    }
            },
            // J_inv
            {
                    {
                            126.4002780806117840484148473478853702545166,
                            0.0000000000000000000000000000000000000000,
                            0.0000000000000000000000000000000000000000
                    },
                    {
                            0.0000000000000000000000000000000000000000,
                            126.4002780806117840484148473478853702545166,
                            0.0000000000000000000000000000000000000000
                    },
                    {
                            0.0000000000000000000000000000000000000000,
                            0.0000000000000000000000000000000000000000,
                            81.2573216232504194067587377503514289855957
                    }
            },
            // action limit
            0.04, // T RPM time constant
            {0, 2000},
    };

}

