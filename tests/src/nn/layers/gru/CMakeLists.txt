
add_executable(
        test_nn_layers_gru_compile_time
        gru_compile.cpp
)
target_link_libraries(
        test_nn_layers_gru_compile_time
        PRIVATE
        RLtools::RLtools
)

if(RL_TOOLS_ENABLE_HDF5)
add_executable(
        test_nn_layers_gru
        gru.cpp
)
target_link_libraries(
        test_nn_layers_gru
        PRIVATE
        RLtools::RLtools
        RLtools::Test
        GTest::gtest_main
)

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    target_compile_options(test_nn_layers_gru PRIVATE -fsanitize=address -fno-omit-frame-pointer)
    target_link_options(test_nn_layers_gru PRIVATE -fsanitize=address)
endif()

gtest_discover_tests(test_nn_layers_gru)
RL_TOOLS_WARNINGS_ARE_ERRORS(test_nn_layers_gru)
target_compile_definitions(test_nn_layers_gru PRIVATE -DRL_TOOLS_DEBUG_CONTAINER_CHECK_BOUNDS)

add_executable(
        test_nn_layers_gru_sequential
        sequential.cpp
)
target_link_libraries(
        test_nn_layers_gru_sequential
        PRIVATE
        RLtools::RLtools
        RLtools::Test
        GTest::gtest_main
)

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    target_compile_options(test_nn_layers_gru_sequential PRIVATE -fsanitize=address -fno-omit-frame-pointer)
    target_link_options(test_nn_layers_gru_sequential PRIVATE -fsanitize=address)
endif()

gtest_discover_tests(test_nn_layers_gru_sequential)
RL_TOOLS_WARNINGS_ARE_ERRORS(test_nn_layers_gru_sequential)
target_compile_definitions(test_nn_layers_gru_sequential PRIVATE -DRL_TOOLS_DEBUG_CONTAINER_CHECK_BOUNDS)
endif()

if(RL_TOOLS_ENABLE_ZLIB AND RL_TOOLS_ENABLE_CJSON AND RL_TOOLS_ENABLE_HDF5)
add_executable(
        test_nn_layers_gru_training
        gru_training.cpp
)
target_link_libraries(
        test_nn_layers_gru_training
        PRIVATE
        RLtools::RLtools
        ZLIB::ZLIB
        cjson
)
if(NOT CMAKE_BUILD_TYPE MATCHES Debug)
    target_compile_options(test_nn_layers_gru_training PRIVATE -Ofast -march=native)
endif()
#if(RL_TOOLS_ENABLE_TRACY)
#    target_link_libraries(test_nn_layers_gru_training PRIVATE Tracy::TracyClient)
#    target_include_directories(test_nn_layers_gru_training PRIVATE ${PROJECT_SOURCE_DIR}/external/tracy/public/tracy)
#    target_compile_definitions(test_nn_layers_gru_training PRIVATE RL_TOOLS_ENABLE_TRACY)
#endif()
endif()

add_executable(
        test_nn_layers_gru_blas
        gru_blas.cpp
)
target_link_libraries(
        test_nn_layers_gru_blas
        PRIVATE
        RLtools::RLtools
        RLtools::Test
        GTest::gtest_main
)
gtest_discover_tests(test_nn_layers_gru_blas)

if(RL_TOOLS_ENABLE_HDF5)
add_executable(
        test_nn_layers_gru_persist
        gru_persist.cpp
)
target_link_libraries(
        test_nn_layers_gru_persist
        PRIVATE
        RLtools::RLtools
        RLtools::Test
        GTest::gtest_main
)
gtest_discover_tests(test_nn_layers_gru_persist)
endif()

if(RL_TOOLS_ENABLE_HDF5)
    add_executable(
            test_nn_layers_gru_inference
            gru_inference.cpp
    )
    target_link_libraries(
            test_nn_layers_gru_inference
            PRIVATE
            RLtools::RLtools
            RLtools::Test
    )
endif()
