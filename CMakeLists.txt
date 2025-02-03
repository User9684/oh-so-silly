cmake_minimum_required(VERSION 3.16)
project(hehe)

set(CMAKE_VERBOSE_MAKEFILE ON)

# Source files
set(SOURCES
    main.cpp
)

set(RC_DIRECTORY "${CMAKE_SOURCE_DIR}/resources")

# Generate resource file
set(RESOURCE_FILE "${CMAKE_BINARY_DIR}/resources.rc")
file(WRITE ${RESOURCE_FILE} "")

# Add constant meow sfx
file(APPEND ${RESOURCE_FILE} "IDR_SOUND1 RC_RCDATA \"${RC_DIRECTORY}/meow.wav\"\n")

# Add all cats from resources/cats directory
file(GLOB CAT_FILES "${RC_DIRECTORY}/cats/*")

set(CAT_INDEX 1)
foreach(CAT_FILE ${CAT_FILES})
    get_filename_component(CAT_NAME ${CAT_FILE} NAME_WE)

    file(APPEND ${RESOURCE_FILE} "CAT${CAT_INDEX} CAT \"${CAT_FILE}\"\n")
    math(EXPR CAT_INDEX "${CAT_INDEX} + 1")
endforeach()

# Add executable
add_executable(${PROJECT_NAME} ${SOURCES} ${RESOURCE_FILE})

# Link required libraries
target_link_libraries(hehe PRIVATE user32 winmm)

set_target_properties(${PROJECT_NAME} PROPERTIES
    WIN32_EXECUTABLE true
)