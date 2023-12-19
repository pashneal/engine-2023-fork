cmake_minimum_required(VERSION 3.2)

# Replace "cpp-pokerbot" with your bot's name
project(cpp-pokerbot)

set(CMAKE_CXX_STANDARD 17)

add_subdirectory(libs)

#enable_language(Rust)

file(GLOB_RECURSE BOT_SRC ${PROJECT_SOURCE_DIR}/src/*.cpp)
add_executable(pokerbot ${BOT_SRC})
target_include_directories(pokerbot PUBLIC ${PROJECT_SOURCE_DIR}/include)

set(CMAKE_C_COMPILER "clang")
set(CMAKE_CXX_COMPILER "clang++")
target_link_libraries(pokerbot skeleton ${PROJECT_SOURCE_DIR}/librust_skeleton.so)
