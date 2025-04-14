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

    if(WIN32)
        set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} -static -static-libgcc -static-libstdc++ -mwindows")
    else()
        set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} -static-libgcc -static-libstdc++")
    endif()
elseif(MSVC)
    set(STRICT_FLAGS /W4 /WX /permissive- /analyze)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /O2")
endif()

option(GITHUB_ACTIONS "Build in GitHub Actions environment" OFF)

if(GITHUB_ACTIONS)
    message(STATUS "GitHub Actions detected, manual Vulkan configuration")
    
    set(Vulkan_INCLUDE_DIR "/usr/include")
    set(Vulkan_LIBRARY "/usr/lib/x86_64-linux-gnu/libvulkan.so")
    set(Vulkan_FOUND TRUE)

    add_library(vulkan INTERFACE)
    add_library(Vulkan::Vulkan ALIAS vulkan)
    target_include_directories(vulkan INTERFACE ${Vulkan_INCLUDE_DIR})
    target_link_libraries(vulkan INTERFACE ${Vulkan_LIBRARY})
else()
    find_package(Vulkan REQUIRED)
endif()

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
set(LIBTYPE "STATIC" CACHE STRING "Library type")

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -w")
endif()

add_subdirectory(lib/openal)

set(IMGUI_SOURCES
    lib/imgui/imgui.cpp
    lib/imgui/imgui_demo.cpp
    lib/imgui/imgui_draw.cpp
    lib/imgui/imgui_tables.cpp
    lib/imgui/imgui_widgets.cpp
    lib/imgui/backends/imgui_impl_glfw.cpp
    lib/imgui/backends/imgui_impl_vulkan.cpp
)

add_library(imgui STATIC ${IMGUI_SOURCES})

target_include_directories(imgui PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/imgui
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/imgui/backends
)

target_link_libraries(imgui PUBLIC
    glfw
    Vulkan::Vulkan
)

file(GLOB_RECURSE SRC_FILES 
        "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp"
)

add_executable(vulkan-minecraft ${SRC_FILES})

if(WIN32)
    set(APP_ICON_PATH "${CMAKE_CURRENT_SOURCE_DIR}/assets/icons/app.ico")
    set(APP_ICON_RESOURCE_WINDOWS "${CMAKE_CURRENT_SOURCE_DIR}/assets/icons/app_icon.rc")

    if(NOT EXISTS "${APP_ICON_PATH}")
        message(FATAL_ERROR "Windows icon file not found at ${APP_ICON_PATH}. Please ensure the file exists.")
    endif()

    if(NOT EXISTS "${APP_ICON_RESOURCE_WINDOWS}")
        file(WRITE "${APP_ICON_RESOURCE_WINDOWS}" "IDI_ICON1 ICON \"${APP_ICON_PATH}\"\n")
        message(STATUS "Created Windows icon resource file pointing to your existing app.ico")
    endif()

    target_sources(vulkan-minecraft PRIVATE "${APP_ICON_RESOURCE_WINDOWS}")

    add_custom_command(TARGET vulkan-minecraft POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${APP_ICON_PATH}"
            "$<TARGET_FILE_DIR:vulkan-minecraft>/app.ico"
        COMMENT "Copying Windows application icon"
        DEPENDS "${APP_ICON_PATH}"
    )
elseif(UNIX AND NOT APPLE)
    set(LINUX_ICON_PATH "${CMAKE_CURRENT_SOURCE_DIR}/assets/icons/app.png")

    if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/assets/icons")
        file(MAKE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/assets/icons")
    endif()

    if(NOT EXISTS "${LINUX_ICON_PATH}")
        message(STATUS "Please place your app.png in assets/icons folder for Linux icon support.")
    endif()

    set(DESKTOP_FILE_PATH "${CMAKE_CURRENT_BINARY_DIR}/vulkan-minecraft.desktop")
    file(WRITE ${DESKTOP_FILE_PATH}
        "[Desktop Entry]\n"
        "Type=Application\n"
        "Name=Vulkan Minecraft\n"
        "Comment=A Minecraft clone built with Vulkan\n"
        "Exec=${CMAKE_INSTALL_PREFIX}/bin/vulkan-minecraft\n"
        "Icon=${CMAKE_INSTALL_PREFIX}/share/icons/hicolor/256x256/apps/vulkan-minecraft.png\n"
        "Terminal=false\n"
        "Categories=Game;\n"
    )

    include(GNUInstallDirs)

    install(TARGETS vulkan-minecraft
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )

    install(DIRECTORY ${CMAKE_SOURCE_DIR}/assets/
        DESTINATION ${CMAKE_INSTALL_DATADIR}/vulkan-minecraft/assets
    )

    install(FILES ${LINUX_ICON_PATH}
        DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/256x256/apps
        RENAME vulkan-minecraft.png
    )

    install(FILES ${DESKTOP_FILE_PATH}
        DESTINATION ${CMAKE_INSTALL_DATADIR}/applications
    )
endif()

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(vulkan-minecraft PRIVATE ${STRICT_FLAGS})
endif()

target_include_directories(vulkan-minecraft PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/stb
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/stb-vorbis/source
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/glm
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/fastnoise/Cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/tomplusplus/include
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/vma/include
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/imgui
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/imgui/backends
)

add_library(stb_vorbis STATIC "${CMAKE_CURRENT_SOURCE_DIR}/lib/stb/stb_vorbis.c")
set_target_properties(stb_vorbis PROPERTIES LINKER_LANGUAGE C)
target_include_directories(stb_vorbis PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/lib/stb")

target_link_libraries(vulkan-minecraft PRIVATE
        glfw
        Vulkan::Vulkan
        OpenAL
        stb_vorbis
        imgui
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