#include "../../../../../../src/rl/environments/pendulum/sac/cpu/training.h"

#include <gtest/gtest.h>

#include <vector>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>


TEST(RL_TOOLS_RL_ALGORITHMS_SAC, FULL_TRAINING_BLAS){
    static constexpr TI NUM_EVALUATION_EPISODES = 10;
    static constexpr TI NUM_ITERATIONS = 100;
    DEVICE device;
    std::vector<std::vector<T>> evaluation_returns;
    std::vector<std::chrono::duration<T>> training_durations;
    for(TI iteration_i=0; iteration_i < NUM_ITERATIONS; iteration_i++){
        LOOP_STATE ts;
        rlt::malloc(device, ts);
        rlt::init(device, ts, iteration_i);
        std::chrono::time_point<std::chrono::high_resolution_clock> start_time = std::chrono::high_resolution_clock::now();
        std::vector<T> current_evaluation_returns;
        while(!rlt::step(device, ts)){
            if(ts.step % 1000 == 0){
                auto result = evaluate(device, ts.env_eval, ts.ui, get_actor(ts), rlt::rl::utils::evaluation::Specification<NUM_EVALUATION_EPISODES, LOOP_CONFIG::EVALUATION_PARAMETERS::EPISODE_STEP_LIMIT>(), ts.observations_mean, ts.observations_std, ts.actor_deterministic_evaluation_buffers, ts.rng_eval, false);
                current_evaluation_returns.push_back(result.returns_mean);
                std::sort(current_evaluation_returns.begin(), current_evaluation_returns.end());
                std::cout << "perc60: " << current_evaluation_returns[evaluation_returns.size()*0.6] << std::endl;
                for(auto rr: evaluation_returns){
                    for(auto r: rr){
                        std::cout << r << " ";
                    }
                }
                std::cout << std::endl;
            }
        }
        std::chrono::time_point<std::chrono::high_resolution_clock> end_time = std::chrono::high_resolution_clock::now();
        std::chrono::duration<T> elapsed_seconds = end_time - start_time;
        std::cout << "Elapsed time: " << elapsed_seconds.count() << "s\n";
        evaluation_returns.push_back(current_evaluation_returns);
        training_durations.push_back(elapsed_seconds);
        // sort returns
        rlt::free(device, ts);
    }
    std::vector<T> all_evaluation_returns;
    T worst_mean_return = 0;
    bool worst_mean_return_set = false;
    T worst_median_return = 0;
    bool worst_median_return_set = false;
    T worst_perc_60_return = 0;
    bool worst_perc_60_return_set = false;
    for(auto returns: evaluation_returns){
        all_evaluation_returns.insert(all_evaluation_returns.end(), returns.begin(), returns.end());
        T mean_return = 0;
        for(auto r: returns){
            mean_return += r;
        }
        mean_return /= returns.size();
        if(!worst_mean_return_set || mean_return < worst_mean_return){
            worst_mean_return = mean_return;
            worst_mean_return_set = true;
        }
        std::sort(returns.begin(), returns.end());
        T median_return = returns[(TI)(returns.size()*0.5)];
        if(!worst_median_return_set || median_return < worst_median_return){
            worst_median_return = median_return;
            worst_median_return_set = true;
        }
        T perc_60_return = returns[(TI)(returns.size()*0.6)];
        if(!worst_perc_60_return_set || perc_60_return < worst_perc_60_return){
            worst_perc_60_return = perc_60_return;
            worst_perc_60_return_set = true;
        }
    }
    std::sort(training_durations.begin(), training_durations.end());
    T median_time = training_durations[(TI)(training_durations.size()*0.5)].count();
    T min_time = training_durations[0].count();
    T max_time = training_durations[training_durations.size()-1].count();
    std::sort(all_evaluation_returns.begin(), all_evaluation_returns.end());
    T perc_60 = all_evaluation_returns[(TI)(all_evaluation_returns.size()*0.6)];
    std::cout << "worst_mean_return: " << worst_mean_return << std::endl;
    std::cout << "worst_median_return: " << worst_median_return << std::endl;
    std::cout << "worst_perc_60_return: " << worst_perc_60_return << std::endl;
    std::cout << "perc_60: " << perc_60 << " (should be around -162)" << std::endl;
    std::cout << "median_time: " << median_time << std::endl;
    std::cout << "min_time: " << min_time << std::endl;
    std::cout << "max_time: " << max_time << std::endl;

    std::filesystem::path results_path = "test_results/rl/algorithms/sac/full_training_blas/";
    std::filesystem::create_directories(results_path);
    std::ofstream results_file(results_path / "results.json");
    nlohmann::json j;
    j["evaluation_returns"] = evaluation_returns;
    j["training_durations"] = training_durations;
    results_file << j.dump(4);
    results_file.close();

    EXPECT_GT(perc_60, -200);
}

