cmake_minimum_required(VERSION 3.10)
project(SchemeCompiler C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Add include directories
include_directories(
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/include/scanner
    ${PROJECT_SOURCE_DIR}/include/utils
    ${PROJECT_SOURCE_DIR}/include/parser
)

# Create utils library
add_library(utils_lib
    src/utils/buffer.c
    src/utils/error.c
)

# Create scanner library
add_library(scanner_lib
    src/scanner/scanner.c
    src/scanner/token.c
)

# Create parser library
add_library(parser_lib
    src/parser/parser.c
)

target_link_libraries(scanner_lib utils_lib)
target_link_libraries(parser_lib scanner_lib utils_lib)

# Create executable
add_executable(scheme_compiler src/main.c)
target_link_libraries(scheme_compiler parser_lib scanner_lib utils_lib)

# Add install targets
install(TARGETS scheme_compiler DESTINATION bin)
install(TARGETS scanner_lib utils_lib DESTINATION lib)
install(DIRECTORY include/ DESTINATION include)

# # Add testing
# enable_testing()
# add_subdirectory(tests)
