add_executable(tinyrefl_test tinyrefl_test.cpp tinyrefl_test.hpp)
target_link_libraries(tinyrefl_test PRIVATE project_options project_warnings CONAN_PKG::tinyrefl)
target_include_directories(tinyrefl_test PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
target_compile_options(tinyrefl_test PRIVATE "-Wno-attributes")

set(CMAKE_CXX_STANDARD 17)

find_package(tinyrefl_tool REQUIRED)

tinyrefl_tool(
    TARGET
    tinyrefl_test
    HEADERS
    tinyrefl_test.hpp)
