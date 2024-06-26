#include "../version.h"
#if (defined(RL_TOOLS_DISABLE_INCLUDE_GUARDS) || !defined(RL_TOOLS_UTILS_RANDOM_OPERATIONS_GENERIC_H)) && (RL_TOOLS_USE_THIS_VERSION == 1)
#pragma once
#define RL_TOOLS_UTILS_RANDOM_OPERATIONS_GENERIC_H


#include "../utils/generic/typing.h"

RL_TOOLS_NAMESPACE_WRAPPER_START
namespace rl_tools::random{
   template <typename MATH_DEV>
   typename MATH_DEV::index_t default_engine(const devices::random::Generic<MATH_DEV>& dev, typename MATH_DEV::index_t seed = 1){
       return 0b10101010101010101010101010101010 + seed;
   };
    template <typename MATH_DEV>
    constexpr typename MATH_DEV::index_t next_max(const devices::random::Generic<MATH_DEV>& dev){
       return MATH_DEV::MAX_INDEX;
   }
   template<typename MATH_DEV, typename RNG>
   void next(const devices::random::Generic<MATH_DEV>& dev, RNG& rng){
       static_assert(utils::typing::is_same_v<RNG, typename MATH_DEV::index_t>);
       rng ^= (rng << 13);
       rng ^= (rng >> 17);
       rng ^= (rng << 5);
   }
    template <typename MATH_DEV, typename TI, typename RNG>
    auto split(const devices::random::Generic<MATH_DEV>& dev, TI split_id, RNG& rng){
        // this operation should not alter the state of rng
        RNG rng_copy = rng;
        auto next_value = next(dev, rng_copy);
        return default_engine(dev, next_value + split_id);
    }

   template<typename MATH_DEV, typename T, typename RNG>
   T uniform_int_distribution(const devices::random::Generic<MATH_DEV>& dev, T low, T high, RNG& rng){
       static_assert(utils::typing::is_same_v<RNG, typename MATH_DEV::index_t>);
       using TI = typename MATH_DEV::index_t;
       TI range = static_cast<typename MATH_DEV::index_t>(high - low) + 1;
       next(dev, rng);
       TI r = rng % range;
       return static_cast<T>(r) + low;
   }
   template<typename MATH_DEV, typename T, typename RNG>
   T uniform_real_distribution(const devices::random::Generic<MATH_DEV>& dev, T low, T high, RNG& rng){
       static_assert(utils::typing::is_same_v<RNG, typename MATH_DEV::index_t>);
       static_assert(utils::typing::is_same_v<T, double> || utils::typing::is_same_v<T, float>);
       next(dev, rng);
       return (rng / static_cast<T>(next_max(dev))) * (high - low) + low;
   }
    namespace normal_distribution{
        template<typename MATH_DEV, typename T, typename RNG>
        RL_TOOLS_FUNCTION_PLACEMENT T sample(const devices::random::Generic<MATH_DEV>& dev, T mean, T std, RNG& rng){
            static_assert(utils::typing::is_same_v<RNG, typename MATH_DEV::index_t>);
            static_assert(utils::typing::is_same_v<T, double> || utils::typing::is_same_v<T, float>);
            next(dev, rng);
            T u1 = rng / static_cast<T>(next_max(dev));
            next(dev, rng);
            T u2 = rng / static_cast<T>(next_max(dev));
            T x = math::sqrt(MATH_DEV{}, -2.0 * math::log(MATH_DEV{}, u1));
            T y = 2.0 * math::PI<T> * u2;
            T z = x * math::cos(MATH_DEV{}, y);
            return z * std + mean;
        }
        template<typename RANDOM_DEVICE, typename T>
        RL_TOOLS_FUNCTION_PLACEMENT T log_prob(const RANDOM_DEVICE& dev, T mean, T log_std, T value){
            static_assert(utils::typing::is_same_v<T, float> || utils::typing::is_same_v<T, double>);
            using MATH_DEV = typename RANDOM_DEVICE::MATH_DEVICE;
            T neg_log_sqrt_pi = -0.5 * math::log(MATH_DEV{}, 2 * math::PI<T>);
            T diff = (value - mean);
            T std = math::exp(MATH_DEV{}, log_std);
            T pre_square = diff/std;
            return neg_log_sqrt_pi - log_std - 0.5 * pre_square * pre_square;
        }
        template<typename RANDOM_DEVICE, typename T>
        RL_TOOLS_FUNCTION_PLACEMENT T d_log_prob_d_mean(RANDOM_DEVICE& dev, T mean, T log_std, T value){
            using MATH_DEV = typename RANDOM_DEVICE::MATH_DEVICE;
            T diff = (value - mean);
            T std = math::exp(MATH_DEV{}, log_std);
            T pre_square = diff/std;
            return pre_square / std;
        }
        template<typename RANDOM_DEVICE, typename T>
        RL_TOOLS_FUNCTION_PLACEMENT T d_log_prob_d_log_std(RANDOM_DEVICE& dev, T mean, T log_std, T value){
            using MATH_DEV = typename RANDOM_DEVICE::MATH_DEVICE;
            T diff = (value - mean);
            T std = math::exp(MATH_DEV{}, log_std);
            T pre_square = diff/std;
            return - 1 + pre_square * pre_square;
        }
        template<typename RANDOM_DEVICE, typename T>
        RL_TOOLS_FUNCTION_PLACEMENT T d_log_prob_d_sample(RANDOM_DEVICE& dev, T mean, T log_std, T value){
            using MATH_DEV = typename RANDOM_DEVICE::MATH_DEVICE;
            T diff = (value - mean);
            T std = math::exp(MATH_DEV{}, log_std);
            T pre_square = diff/std;
            return - pre_square / std;
        }
    }
}
RL_TOOLS_NAMESPACE_WRAPPER_END

#endif
