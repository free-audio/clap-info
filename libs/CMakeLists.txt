project(clap-info-libs VERSION 1.0.0 LANGUAGES C CXX)

add_subdirectory(CLI11)
add_library(jsoncpp
        jsoncpp/src/lib_json/json_reader.cpp
        jsoncpp/src/lib_json/json_value.cpp
        jsoncpp/src/lib_json/json_writer.cpp
        )
target_include_directories(jsoncpp PUBLIC jsoncpp/include)
target_compile_options(jsoncpp PRIVATE $<$<CXX_COMPILER_ID:GNU>:-Wno-deprecated-declarations>)

