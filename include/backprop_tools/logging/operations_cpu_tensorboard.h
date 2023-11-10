#include "../version.h"
#if (defined(BACKPROP_TOOLS_DISABLE_INCLUDE_GUARDS) || !defined(BACKPROP_TOOLS_LOGGING_OPERATIONS_CPU_TENSORBOARD_H)) && (BACKPROP_TOOLS_USE_THIS_VERSION == 1)
#pragma once
#define BACKPROP_TOOLS_LOGGING_OPERATIONS_CPU_TENSORBOARD_H
#include <backprop_tools/containers.h>

#include <filesystem>
#include <cassert>
#include "operations_cpu.h"
BACKPROP_TOOLS_NAMESPACE_WRAPPER_START

#ifdef BACKPROP_TOOLS_ENABLE_LIBATTOPNG
#include <libattopng.h>
#endif
namespace backprop_tools{
    namespace logging::tensorboard{
        std::string sanitize_file_name(const std::string &input) {
            std::string output = input;

            const std::string invalid_chars = R"(<>:\"/\|?*)";

            std::replace_if(output.begin(), output.end(), [&invalid_chars](const char &c) {
                return invalid_chars.find(c) != std::string::npos;
            }, '_');

            return output;
        }
        template <typename DEVICE, typename KEY_TYPE>
        void count_topic(DEVICE& device, devices::logging::CPU_TENSORBOARD<>& logger, KEY_TYPE key){ }
        template <typename DEVICE, typename KEY_TYPE>
        void count_topic(DEVICE& device, devices::logging::CPU_TENSORBOARD_FREQUENCY_EXTENSION& logger, KEY_TYPE key){
            if(!logger.topic_frequency_dict.count(key)){
                logger.topic_frequency_dict.insert({key, 0});
            }
            logger.topic_frequency_dict[key] += 1;
        }
    }
    template <typename DEVICE, typename SPEC>
    void construct(DEVICE& device, devices::logging::CPU_TENSORBOARD<SPEC>& logger, std::string logs_dir, std::string name){
        if (!std::filesystem::is_directory(logs_dir.c_str()) || !std::filesystem::exists(logs_dir.c_str())) {
            std::filesystem::create_directory(logs_dir.c_str());
        }
        std::filesystem::path log_dir = std::filesystem::path(logs_dir) / name;
        if (!std::filesystem::is_directory(log_dir.c_str()) || !std::filesystem::exists(log_dir.c_str())) {
            std::filesystem::create_directory(log_dir.c_str());
        }

        auto log_file = log_dir / std::string("data.tfevents");
        std::cout << "Logging to " << log_file.string() << std::endl;
        TensorBoardLoggerOptions opts;
        opts.flush_period_s(1);
        logger.tb = new TensorBoardLogger(log_file.string(), opts);
    }
    template <typename DEVICE, typename SPEC>
    void construct(DEVICE& device, devices::logging::CPU_TENSORBOARD<SPEC>& logger){
        time_t now;
        time(&now);
        char buf[sizeof "0000-00-00T00:00:00Z"];
        strftime(buf, sizeof buf, "%FT%TZ", localtime(&now));

        std::string run_name = std::string(buf);

        construct(device, logger, std::string("logs"), logging::tensorboard::sanitize_file_name(run_name));
    }
    template <typename DEVICE, typename SPEC>
    void destruct(DEVICE& device, devices::logging::CPU_TENSORBOARD<SPEC>& logger){
        delete logger.tb;
    }
    template <typename DEVICE, typename SPEC>
    void set_step(DEVICE& device, devices::logging::CPU_TENSORBOARD<SPEC>& logger, typename DEVICE::index_t step){
        logger.step = step;
    }
    template <typename DEVICE, typename KEY_TYPE, typename VALUE_TYPE, typename CADANCE_TYPE, typename SPEC>
    void add_scalar(DEVICE& device, devices::logging::CPU_TENSORBOARD<SPEC>& logger, const KEY_TYPE key, const VALUE_TYPE value, const CADANCE_TYPE cadence){
        std::lock_guard<std::mutex> lock(logger.mutex);
        if(logger.step % cadence == 0){
            logger.tb->add_scalar(key, logger.step, (float)value);
            logging::tensorboard::count_topic(device, logger, key);
        }
    }
    template <typename DEVICE, typename KEY_TYPE, typename VALUE_TYPE, typename SPEC>
    void add_scalar(DEVICE& device, devices::logging::CPU_TENSORBOARD<SPEC>& logger, const KEY_TYPE key, const VALUE_TYPE value){
        add_scalar(device, logger, key, value, (typename DEVICE::index_t)1);
    }
    template <typename DEVICE, typename KEY_TYPE, typename T, typename TI, typename CADANCE_TYPE, typename SPEC>
    void add_histogram(DEVICE& device, devices::logging::CPU_TENSORBOARD<SPEC>& logger, const KEY_TYPE key, const T* values, const TI n_values, const CADANCE_TYPE cadence = (typename DEVICE::index_t)1){
        if(logger.tb == nullptr){
            return;
        }
        std::lock_guard<std::mutex> lock(logger.mutex);
        if(logger.step % cadence == 0){
            logger.tb->add_histogram(key, logger.step, values, n_values);
            logging::tensorboard::count_topic(device, logger, key);
        }
    }
    template <typename DEVICE, typename KEY_TYPE, typename T, typename TI, typename SPEC>
    void add_histogram(DEVICE& device, devices::logging::CPU_TENSORBOARD<SPEC>& logger, const KEY_TYPE key, const T* values, const TI n_values){
        add_histogram(device, logger, key, values, n_values, (typename DEVICE::index_t)1);
    }
#ifdef BACKPROP_TOOLS_ENABLE_LIBATTOPNG
    template <typename DEVICE, typename KEY_TYPE, typename LOGGING_SPEC, typename SPEC>
    void add_image(DEVICE& device, devices::logging::CPU_TENSORBOARD<LOGGING_SPEC>& logger, const KEY_TYPE key, backprop_tools::Matrix<SPEC> values){
        using T = typename SPEC::T;
        using TI = typename DEVICE::index_t;
        libattopng_t* png = libattopng_new(SPEC::COLS, SPEC::ROWS, PNG_RGBA);

        for (TI y = 0; y < SPEC::ROWS; y++) {
            for (TI x = 0; x < SPEC::COLS; x++) {
                uint8_t r = math::clamp<T>(device.math, get(values, x, y) * 255.0, 0, 255);
                uint8_t g = r;
                uint8_t b = r;
                uint8_t a = 255;
                uint32_t color = r | ((g) << 8) | ((b) << 16) | ((a) << 24);
                libattopng_set_pixel(png, x, y, color);
            }
        }
        size_t len;
        char *data = libattopng_get_data(png, &len);
        std::string image_data = std::string(data, len);
        std::cout << "adding image at step: " << logger.step << std::endl;
        logger.tb->add_image(key, logger.step, image_data, SPEC::ROWS, SPEC::COLS, 4, "Test", "Image");
        libattopng_destroy(png);
        logging::tensorboard::count_topic(device, logger, key);
    }
#else
    template <typename DEVICE, typename LOGGER_SPEC, typename SPEC>
    void add_image(DEVICE& device, devices::logging::CPU_TENSORBOARD<LOGGER_SPEC>& logger, backprop_tools::Matrix<SPEC> values){ }
#endif
}
BACKPROP_TOOLS_NAMESPACE_WRAPPER_END
#endif
