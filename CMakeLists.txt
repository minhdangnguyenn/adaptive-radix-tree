cmake_minimum_required (VERSION 3.3)
set(CMAKE_C_COMPILER "gcc")
set(CMAKE_CXX_COMPILER "g++")
project(art CXX)


set (CMAKE_CXX_STANDARD 20)
set (CMAKE_CXX_STANDARD_REQUIRED ON)

# local run: remove mavx512
 set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mavx -mavx2 -mpopcnt -mno-avx512f -pthread -Wall -Wextra")

# run in remote (CI)
# set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mavx -mavx512f -mavx512dq -mavx512cd -mavx512bw -mavx512vl -mavx2 -pthread -Wall -Wextra")


if(CMAKE_BUILD_TYPE MATCHES DEBUG)
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror")
endif(CMAKE_BUILD_TYPE MATCHES DEBUG)


if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif(NOT CMAKE_BUILD_TYPE)


# Prepare "Catch" library for other executables
add_library(catch INTERFACE)
target_include_directories(catch INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/catch)


find_package(Threads)

set(LIBS "pthread")

file(GLOB SRC_FILES ./src/*.cpp)
add_library(${PROJECT_NAME}_lib ${SRC_FILES})
target_include_directories(${PROJECT_NAME}_lib PRIVATE ./include)
target_link_libraries(${PROJECT_NAME}_lib ${LIBS})


file(GLOB TEST_FILES ./test/*.cpp)
foreach(file ${TEST_FILES})
    get_filename_component(name ${file} NAME_WE)
    message("Found test: ${name}")
    set(target "${PROJECT_NAME}_test_${name}")

    add_executable(${target} ./catch/main.cpp ${file})
    target_link_libraries(${target} ${PROJECT_NAME}_lib ${LIBS})
    target_include_directories(${target} PRIVATE ./catch ./include)
endforeach()

# I add this section to run perf
# benchmark target (standalone main)
set(BENCH_TARGET "art_bench")

add_executable(${BENCH_TARGET} bench.cpp)
target_link_libraries(${BENCH_TARGET} ${PROJECT_NAME}_lib ${LIBS})
target_include_directories(${BENCH_TARGET} PRIVATE ./include)
