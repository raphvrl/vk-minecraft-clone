cmake_minimum_required(VERSION 3.10)
project(vulkan-minecraft)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(BUILD_SHARED_LIBS "Build shared libraries" OFF)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING 
            "Build type (Debug/Release/RelWithDebInfo/MinSizeRel)" FORCE)
endif()

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set(STRICT_FLAGS -Wall -Wextra -Werror -pedantic)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O3 -g")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -march=native -ftree-vectorize")
elseif(MSVC)
    set(STRICT_FLAGS /W4 /WX /permissive- /analyze)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /O2")
endif()

find_package(Vulkan REQUIRED)

if(CMAKE_BUILD_TYPE MATCHES "Debug")
        add_compile_definitions(DEBUG)
else()
        add_compile_definitions(NDEBUG)
endif()

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(lib/glfw)

add_compile_definitions(GLM_ENABLE_EXPERIMENTAL)

set(ALSOFT_UTILS OFF CACHE BOOL "" FORCE)
set(ALSOFT_EXAMPLES OFF CACHE BOOL "" FORCE)
set(ALSOFT_TESTS OFF CACHE BOOL "" FORCE)
set(ALSOFT_CONFIG OFF CACHE BOOL "" FORCE)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -w")
endif()

add_subdirectory(lib/openal)

file(GLOB_RECURSE SRC_FILES 
        "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp"
)

add_executable(vulkan-minecraft ${SRC_FILES})

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(vulkan-minecraft PRIVATE ${STRICT_FLAGS})
endif()

target_include_directories(vulkan-minecraft PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/stb
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/glm
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/fastnoise/Cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/tomplusplus/include
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/vma/include
)

target_link_libraries(vulkan-minecraft PRIVATE
        glfw
        Vulkan::Vulkan
        OpenAL
)

if(Vulkan_FOUND)
        find_program(GLSLC_EXECUTABLE glslc HINTS 
                $ENV{VULKAN_SDK}/bin 
                ${Vulkan_GLSLC_EXECUTABLE}
        )
        
        if(GLSLC_EXECUTABLE)
                message(STATUS "Found glslc: ${GLSLC_EXECUTABLE}")

                file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/assets/shaders)

                file(GLOB_RECURSE SHADER_SOURCES 
                        "${CMAKE_CURRENT_SOURCE_DIR}/src/shaders/*.vert"
                        "${CMAKE_CURRENT_SOURCE_DIR}/src/shaders/*.frag"
                        "${CMAKE_CURRENT_SOURCE_DIR}/src/shaders/*.comp"
                        "${CMAKE_CURRENT_SOURCE_DIR}/src/shaders/*.geom"
                        "${CMAKE_CURRENT_SOURCE_DIR}/src/shaders/*.tesc"
                        "${CMAKE_CURRENT_SOURCE_DIR}/src/shaders/*.tese"
                )

                set(SHADER_OUTPUTS)

                foreach(SHADER ${SHADER_SOURCES})
                        get_filename_component(SHADER_NAME ${SHADER} NAME)

                        set(SHADER_OUTPUT 
                                "${CMAKE_CURRENT_SOURCE_DIR}/assets/shaders/${SHADER_NAME}.spv")

                        add_custom_command(
                                OUTPUT ${SHADER_OUTPUT}
                                COMMAND ${GLSLC_EXECUTABLE} 
                                        -I${CMAKE_CURRENT_SOURCE_DIR}/src/shaders/include
                                        ${SHADER} 
                                        -o ${SHADER_OUTPUT}
                                DEPENDS ${SHADER}
                                COMMENT "Compiling shader ${SHADER_NAME}"
                                VERBATIM
                        )

                        list(APPEND SHADER_OUTPUTS ${SHADER_OUTPUT})
                endforeach()

                add_custom_target(shaders ALL DEPENDS ${SHADER_OUTPUTS})

                add_dependencies(vulkan-minecraft shaders)
        else()
                message(WARNING 
                        "Could not find glslc executable. Shaders will not be compiled.")
        endif()
endif()

add_custom_command(TARGET vulkan-minecraft POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/assets
    ${CMAKE_BINARY_DIR}/assets)