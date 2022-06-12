cmake_minimum_required(VERSION 3.20)
cmake_policy(SET CMP0091 NEW)

# This only needs macos 15 due to std::filesystem, not clap
set(CMAKE_OSX_DEPLOYMENT_TARGET 10.15 CACHE STRING "Minimum macOS version")

project(clap-info VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)
# CLAP supports standard C; but this validator uses std::filesystem so requires C++ 17
set(CMAKE_CXX_STANDARD 17)

add_subdirectory(libs/clap)
add_subdirectory(libs/CLI11)

# use asan as an option (currently mac only)
option(USE_USE_SANITIZER "Build and link with ASAN" FALSE)

add_executable(${PROJECT_NAME}
        src/main.cpp
        src/clap-info-host.cpp
        src/resolve_entrypoint.cpp)

if (APPLE)
    target_compile_definitions(${PROJECT_NAME} PRIVATE MAC=1)
    target_link_libraries(${PROJECT_NAME} PRIVATE dl "-framework CoreFoundation")
    target_compile_options(${PROJECT_NAME} PRIVATE
            $<$<BOOL:${USE_SANITIZER}>:-fUSE_SANITIZER=address>
            $<$<BOOL:${USE_SANITIZER}>:-fUSE_SANITIZER=undefined>
    )
    target_link_options(${PROJECT_NAME} PRIVATE
            $<$<BOOL:${USE_SANITIZER}>:-fUSE_SANITIZER=address>
            $<$<BOOL:${USE_SANITIZER}>:-fUSE_SANITIZER=undefined>
    )
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Werror)
elseif(UNIX)
    target_compile_definitions(${PROJECT_NAME} PRIVATE LIN=1)
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Werror)
    target_link_libraries(${PROJECT_NAME} PRIVATE -ldl)
else()
    target_compile_definitions(${PROJECT_NAME} PRIVATE WIN=1)
endif()

target_include_directories(${PROJECT_NAME} PRIVATE src)
target_link_libraries(${PROJECT_NAME} PRIVATE clap-core CLI11)
