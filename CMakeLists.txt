cmake_minimum_required(VERSION 3.9.4)
project(learn_algo)

set(CMAKE_CXX_STANDARD 17)

find_package(GTest      REQUIRED)
find_package(Threads    REQUIRED)

include_directories(${PROJECT_SOURCE_DIR})


file(GLOB   test_cxx_source_files            ${PROJECT_SOURCE_DIR}/test/*.cc)
add_executable(test_learn_algo               ${test_cxx_source_files})
target_include_directories(test_learn_algo   PUBLIC ${GTEST_INCLUDE_DIRS})
target_link_libraries(test_learn_algo        ${GTEST_BOTH_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
add_test(test_learn_algo     test_learn_algo)

enable_testing()

file(GLOB_RECURSE all_cxx_source_files ${PROJECT_SOURCE_DIR}/*.cc ${PROJECT_SOURCE_DIR}/*.h)

find_program(clang_format "clang-format")
if(clang_format)
    add_custom_target(clang-format COMMAND ${clang_format} -i -style=file ${all_cxx_source_files})
endif()

find_program(clang_tidy "clang-tidy")
if(clang_tidy)
    add_custom_target(clang-tidy COMMAND ${clang_tidy} ${all_cxx_source_files} -checks=* -- -std=c++1z -I${include_paths})
endif()