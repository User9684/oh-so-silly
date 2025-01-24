cmake_minimum_required(VERSION 3.16)
project(hehe)

set(CMAKE_VERBOSE_MAKEFILE ON)

# Source files
set(SOURCES
    main.cpp
)

# Resources file
set(RESOURCE_FILE ./resources/resources.rc)

# Add executable
add_executable(${PROJECT_NAME} ${SOURCES} ${RESOURCE_FILE})

# Link required libraries
target_link_libraries(hehe PRIVATE user32 winmm)

set_target_properties(${PROJECT_NAME} PROPERTIES
    WIN32_EXECUTABLE true
)