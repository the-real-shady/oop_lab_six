cmake_minimum_required(VERSION 3.10)

project(BalagurFateLab6)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Строгие флаги
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic")
endif()

# GoogleTest через FetchContent
include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# Включаем include/ для заголовков
include_directories(
    ${PROJECT_SOURCE_DIR}/objects/npc
    ${PROJECT_SOURCE_DIR}/objects/dragon
    ${PROJECT_SOURCE_DIR}/objects/princess
    ${PROJECT_SOURCE_DIR}/objects/knight
)

# Главная программа
add_executable(balagur_lab6
    main.cpp
)

# ---- GoogleTest ----
enable_testing()

add_executable(gtests
    tests.cpp
)

target_link_libraries(gtests
    GTest::gtest
    GTest::gtest_main
)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    target_link_libraries(gtests pthread)
endif()

add_test(NAME unit_tests COMMAND gtests)
