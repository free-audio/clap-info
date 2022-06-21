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

add_library(jsoncpp
        libs/jsoncpp/src/lib_json/json_reader.cpp
        libs/jsoncpp/src/lib_json/json_value.cpp
        libs/jsoncpp/src/lib_json/json_writer.cpp
        )
target_include_directories(jsoncpp PUBLIC libs/jsoncpp/include)
target_compile_options(jsoncpp PRIVATE $<$<CXX_COMPILER_ID:GNU>:-Wno-deprecated-declarations>)


# use asan as an option (currently mac only)
option(USE_USE_SANITIZER "Build and link with ASAN" FALSE)

add_executable(${PROJECT_NAME}
        src/main.cpp
        src/clap-info-host.cpp
        src/resolve_entrypoint.cpp

        src/info-params.cpp
        src/info-pathscan.cpp
        src/info-ports.cpp
        )


add_custom_target(clap-info-zip)
add_dependencies(clap-info-zip clap-info)
set(INSTALLER_DIR ${CMAKE_BINARY_DIR}/installer)
set(INFO_DIR ${INSTALLER_DIR}/clap-info)

add_custom_command(TARGET clap-info-zip
        POST_BUILD
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${INFO_DIR}
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/README.md ${INFO_DIR}
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/LICENSE.md ${INFO_DIR}
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:clap-info> ${INFO_DIR}
        COMMAND git log -1 > ${INFO_DIR}/version.txt
        COMMAND git log --pretty=oneline -10 >> ${INFO_DIR}/version.txt
        )

if (UNIX)
    add_custom_command(TARGET clap-info-zip
            POST_BUILD
            WORKING_DIRECTORY ${INSTALLER_DIR}
            COMMAND tar cvzf clap-info.tar.gz clap-info )
else()
    add_custom_command(TARGET clap-info-zip
            POST_BUILD
            WORKING_DIRECTORY ${INSTALLER_DIR}
            COMMAND  7z a -r clap-info.zip clap-info)
endif()

if (APPLE)
    target_compile_definitions(${PROJECT_NAME} PRIVATE MAC=1)
    target_sources(${PROJECT_NAME} PRIVATE src/resolve_macosLocations.mm)
    target_link_libraries(${PROJECT_NAME} PRIVATE dl "-framework CoreFoundation" "-framework Foundation")
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
target_link_libraries(${PROJECT_NAME} PRIVATE clap-core CLI11 jsoncpp)
