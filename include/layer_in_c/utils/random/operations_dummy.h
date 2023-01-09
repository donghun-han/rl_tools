#ifndef LAYER_IN_C_UTILS_RANDOM_OPERATIONS_DUMMY_H
#define LAYER_IN_C_UTILS_RANDOM_OPERATIONS_DUMMY_H

#include <layer_in_c/devices.h>
#include <layer_in_c/utils/generic/typing.h>

namespace layer_in_c::utils::random{
    template<typename DEVICE>
    using default_engine = utils::typing::enable_if_t<utils::typing::is_same_v<DEVICE, devices::random::Dummy>, index_t>;

    template<typename T, typename RNG>
    T uniform_int_distribution(const devices::random::Dummy& dev, T low, T high, RNG& rng){
//        static_assert(utils::typing::is_same_v<T, index_t>);
//        static_assert(utils::typing::is_same_v<RNG, index_t>);
//        // https://en.wikipedia.org/wiki/Xorshift
//        rng ^= (rng << 21);
//        rng ^= (rng >> 35);
//        rng ^= (rng << 4);
//        return rng % (high-low) + low;
        return low;
    }
    template<typename T, typename RNG>
    T uniform_real_distribution(const devices::random::Dummy& dev, T low, T high, RNG& rng){
//        static_assert(utils::typing::is_same_v<RNG, index_t>);
//        T r = (T)uniform_int_distribution<index_t, index_t>((index_t)0, (index_t)1<<31, rng) / (T)(1<<31) - 1;
//        return r * (high-low) + low;
        return low;
    }
    template<typename T, typename RNG>
    T normal_distribution(const devices::random::Dummy& dev, T mean, T std, RNG& rng){
//        static_assert(utils::typing::is_same_v<RNG, index_t>);
//        //
//        T x, y, r, res;
//        x = 0;
//        y = 0;
//        r = 0;
//        res = 0;
//
//        do {
//            x = (T)2.0 * (T)uniform_int_distribution<index_t, index_t>(0, 1<<31, rng) / (T)(1<<31) - 1;
//            y = (T)2.0 * (T)uniform_int_distribution<index_t, index_t>(0, 1<<31, rng) / (T)(1<<31) - 1;
//            r = x * x + y * y;
//        } while (r == 0.0 || r > 1.0);
//
//        T d = math::sqrt(-2.0 * math::log(r) / r);
//
//        T n1 = x * d;
//        T n2 = y * d;
//
//        res = n1 * std + mean;
//
//        return res;
        return mean;
    }
}

#endif