# vim: syntax=cmake
if(NOT CMAKE_BUILD_TYPE)
    # default to Release build for GCC builds
    set(CMAKE_BUILD_TYPE Release CACHE STRING
        "Choose the type of build, options are: None(CMAKE_CXX_FLAGS or CMAKE_C_FLAGS used) Debug Release RelWithDebInfo MinSizeRel."
        FORCE)
endif()

project(glfwDemo)

set(CMAKE_CXX_STANDARD 11)

cmake_minimum_required(VERSION 2.8)

include(platform/${BUILD_PLATFORM}.cmake)

include_directories("${PROJECT_SOURCE_DIR}/glad/include")
include_directories("${PROJECT_SOURCE_DIR}/deps/${BUILD_PLATFORM}/GLFW/include")
link_directories("${PROJECT_SOURCE_DIR}/deps/${BUILD_PLATFORM}/GLFW/lib")

set(GLAD_SRC
    "${PROJECT_SOURCE_DIR}/glad/include/glad/glad.h"
    "${PROJECT_SOURCE_DIR}/glad/include/KHR/khrplatform.h"
    "${PROJECT_SOURCE_DIR}/glad/src/glad.c"
)

set(DEMO_SRC
    "${PROJECT_SOURCE_DIR}/GLDemo.cc"
)

source_group(TREE ${CMAKE_SOURCE_DIR} FILES ${GLAD_SRC} ${DEMO_SRC})

add_executable(main ${GLAD_SRC} ${DEMO_SRC})
target_link_libraries(main glfw3 ${PLATFORM_LIB})
