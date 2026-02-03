cmake_minimum_required(VERSION 3.30)

project(fa
    VERSION 0.1.0
    LANGUAGES C
)

set(CMAKE_C_STANDARD 17)

# Option to build as shared library (default: OFF = static)
option(BUILD_SHARED_LIBS "Build shared libraries" OFF)

# Create the library (type depends on BUILD_SHARED_LIBS)
add_library(fa_lib
    src/common.c
    src/fa_error.c
    src/fa_memory.c
    src/fa_debug.c
    src/set/set.c
    src/hash/hash_table.c
    src/regex/regexpr.c
    src/io/fa_auto_io.c
    src/fa.c
)

# Set properties for the library
set_target_properties(fa_lib PROPERTIES
    OUTPUT_NAME fa
    VERSION ${PROJECT_VERSION}
    SOVERSION 1
    PUBLIC_HEADER include/fa.h  # If you have public headers
)

# Add include directories
target_include_directories(fa_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Create the executable
add_executable(fa main.c)
target_link_libraries(fa PRIVATE fa_lib)

# Install targets 
install(TARGETS fa_lib
    ARCHIVE DESTINATION lib      # Static library
    LIBRARY DESTINATION lib      # Shared library
    RUNTIME DESTINATION bin      # DLL on Windows
    PUBLIC_HEADER DESTINATION include
)

install(TARGETS fa DESTINATION bin)