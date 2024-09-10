cmake_minimum_required(VERSION 3.20)
cmake_policy(SET CMP0091 NEW)

# CLAP supports standard C; but this validator uses std::filesystem so requires C++ 17
# and macOS 10.15
set(CMAKE_OSX_DEPLOYMENT_TARGET 10.15 CACHE STRING "Minimum macOS version")
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

project(clap-info VERSION 1.0.0 LANGUAGES C CXX)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_CXX_STANDARD 17)

# Build-time options
# Use a different version of clap
if (NOT DEFINED ${CLAP_INFO_CLAP_DIR})
    set(CLAP_INFO_CLAP_DIR "libs/clap" CACHE STRING "Location of the CLAP cmake project")
endif()
# Use the Address Sanitizer (currently macOS only)
option(USE_USE_SANITIZER "Build and link with ASAN" FALSE)


add_subdirectory(libs)
message(STATUS "Using CLAP installed from ${CLAP_INFO_CLAP_DIR}")
add_subdirectory(${CLAP_INFO_CLAP_DIR})

add_library(clap-scanner
        src/clap-scanner/resolve_entrypoint.cpp
        src/clap-scanner/scanner.cpp
        )

target_include_directories(clap-scanner PUBLIC include)
target_link_libraries(clap-scanner PUBLIC clap-core)

add_executable(${PROJECT_NAME}
        src/clap-info/main.cpp
        src/clap-info/clap-info-host.cpp

        src/clap-info/info-params.cpp
        src/clap-info/info-ports.cpp
        src/clap-info/info-other-extensions.cpp
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
    target_compile_definitions(clap-scanner PUBLIC MAC=1)
    target_sources(clap-scanner PRIVATE src/clap-scanner/resolve_macosLocations.mm)
    target_link_libraries(clap-scanner PUBLIC dl "-framework CoreFoundation" "-framework Foundation")

    target_compile_definitions(${PROJECT_NAME} PRIVATE MAC=1)
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
    target_compile_definitions(clap-scanner PUBLIC LIN=1)
    target_link_libraries(clap-scanner PUBLIC -ldl)
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Werror)
else()
    target_compile_definitions(clap-scanner PUBLIC WIN=1)
endif()

target_include_directories(${PROJECT_NAME} PRIVATE src)
target_link_libraries(${PROJECT_NAME} PRIVATE CLI11 jsoncpp clap-scanner)
