cmake_minimum_required(VERSION 3.30)

project(fa
    VERSION 0.1.0
    LANGUAGES C
)

set(CMAKE_C_STANDARD 17)

option(SET_DEBUG "Enable set debugging features" ON)
if(SET_DEBUG)
    add_definitions(-DSET_DEBUG_ENABLE)
    add_definitions(-DHASH_TABLE_DEBUG)
endif()


add_executable(maintool
    main.c
    src/set/set.c
)