cmake_minimum_required(VERSION 3.14)
if(POLICY CMP0135)
    cmake_policy(SET CMP0135 NEW)
endif()
project(MiniGit VERSION 1.0 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(FetchContent)

find_package(ZLIB REQUIRED)

# Fetch BLAKE3
FetchContent_Declare(
  blake3
  GIT_REPOSITORY https://github.com/BLAKE3-team/BLAKE3.git
  GIT_TAG 1.3.3
  GIT_SHALLOW TRUE
  SOURCE_SUBDIR c
)
FetchContent_MakeAvailable(blake3)

# Create blake3 library manually since it has no CMakeLists in 1.3.3
add_library(blake3 STATIC
    ${blake3_SOURCE_DIR}/c/blake3.c
    ${blake3_SOURCE_DIR}/c/blake3_dispatch.c
    ${blake3_SOURCE_DIR}/c/blake3_portable.c
)
if(CMAKE_SYSTEM_PROCESSOR MATCHES "arm64|aarch64")
    target_sources(blake3 PRIVATE ${blake3_SOURCE_DIR}/c/blake3_neon.c)
endif()
target_include_directories(blake3 PUBLIC ${blake3_SOURCE_DIR}/c)

# Fetch GoogleTest
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/5376968f6948923e2411081fd9372e71a59d8e77.zip
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_library(minigit_core
    src/core/Repository.cpp
    src/core/GitObject.cpp
    src/core/Blob.cpp
    src/core/Tree.cpp
    src/core/Commit.cpp
    src/core/Index.cpp
    src/core/Diff.cpp
    src/core/Chunker.cpp
    src/core/Utils.cpp
)
target_link_libraries(minigit_core PUBLIC ZLIB::ZLIB blake3)
target_include_directories(minigit_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src ${blake3_SOURCE_DIR}/c)

add_executable(minigit 
    src/main.cpp
    src/cli/InitCommand.cpp
    src/cli/AddCommand.cpp
    src/cli/CommitCommand.cpp
    src/cli/DiffCommand.cpp
    src/cli/GcCommand.cpp
    src/cli/CatFileCommand.cpp
    src/cli/LogCommand.cpp
    src/cli/StatusCommand.cpp
)
target_link_libraries(minigit PRIVATE minigit_core)

enable_testing()
add_executable(minigit_tests tests/test_basic.cpp)
target_link_libraries(minigit_tests PRIVATE minigit_core GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(minigit_tests)
