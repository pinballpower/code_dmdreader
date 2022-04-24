cmake_minimum_required (VERSION 3.16)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(VCPKG_TARGET_TRIPLET=x64-windows-static)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

set(DISABLE_RAYLIB 1)

add_definitions(-D_SILENCE_CXX17_ITERATOR_BASE_CLASS_DEPRECATION_WARNING)
set(BUILD_SHARED_LIBS OFF)
set(Boost_USE_STATIC_LIBS ON)

project ("DMDReader")

message(STATUS "$ENV{CMAKE_INCLUDE_PATH}")

# detect Raspberry Pi
include(CheckIncludeFile)
CHECK_INCLUDE_FILE(bcm_host.h RPI)
if (RPI)
message(STATUS "compiling on Raspberry Pi")
endif()

if(WIN32)
message(STATUS "compiling on Windows")
set(EXTRA_LIBS winmm.lib)
endif()

#
# STB 
#
find_path(STB_INCLUDE_DIRS "stb.h")
include_directories(${STB_INCLUDE_DIRS})

#
# Raylib (optional)
#
if (NOT(DISABLE_RAYLIB))
find_package(raylib CONFIG QUIET)
if (raylib_FOUND)
add_definitions(-DUSE_RAYLIB)
message(STATUS "enabling raylib output module")
set(raylib_sources "render/raylibrenderer.h" "render/raylibrenderer.cpp")
else()
message(STATUS "not enabling raylib output module")
endif()
endif()

#
# OpenGL Pi4
#
message(STATUS "Checking for Raspberry Pi")
if (RPI)
message(STATUS "compiling OpenGL ES for Raspberry Pi")
find_path(DRM_INCLUDE_DIR "drm.h")
message(STATUS "drm.h found in ${DRM_INCLUDE_DIR}")
add_definitions(-DUSE_OPENGLPI4)
# set (PI_INCLUDE_DIRS "/usr/include:/usr/include/stb:/usr/include/libdrm")
set (PI_LIBS EGL drm glfw gbm GLESv2)
set (opengl_sources "render/pi4renderer.h" "render/pi4renderer.cpp" "render/openglrenderer.h" "render/openglrenderer.cpp" "render/openglshader.h" "render/openglshader.cpp")
endif()


#
# OpenGL/GLAD
#
if (NOT(RPI))
message(STATUS "Checking for OpenGL")
set(OPENGL 1)
set(OPENGL_GLAD 1)
find_package(OpenGL QUIET)
if (NOT OpenGL_FOUND)
message(STATUS "Package OpenGL not found, disabling OpenGL")
endif()

find_package(glbinding CONFIG QUIET)
if (NOT glbinding_FOUND)
message(STATUS "Package glbinding not found, disabling OpenGL")
set(OPENGL 0)
endif()

find_package(glfw3 CONFIG QUIET)
if (NOT glfw3_FOUND)
message(STATUS "Package glfw3 not found, disabling OpenGL")
set(OPENGL 0)
endif()

find_package(glm QUIET)
if (NOT glm_FOUND)
message(STATUS "Package glm not found, disabling OpenGL")
set(OPENGL 0)
endif()

find_package(fmt CONFIG QUIET)
if (NOT fmt_FOUND)
message(STATUS "Package fmt not found, disabling OpenGL")
set(OPENGL 0)
endif()

find_package(GLEW CONFIG QUIET)
if (NOT GLEW_FOUND)
message(STATUS "Package GLEW not found, disabling OpenGL")
set(OPENGL 0)
endif()

find_package(glad CONFIG QUIET)
if (NOT glad_FOUND)
message(STATUS "Package glad not found, disabling OpenGL")
set(OPENGL 0)
set(OPENGL_GLAD 0)
endif()

if (OPENGL)
add_definitions(-DUSE_OPENGLGLAD)
set (opengl_sources "render/openglrenderer.h" "render/openglrenderer.cpp" "render/openglshader.h" "render/openglshader.cpp" "render/gladopenglrenderer.h" "render/gladopenglrenderer.cpp")
set(OPENGL_LIBRARIES fmt::fmt
    fmt::fmt-header-only
    glfw
    glbinding::glbinding
    glbinding::glbinding-aux
    glad::glad
    GLEW::GLEW
    ${OPENGL_LIBRARIES}
    )
endif()

if (OPENGL_GLAD) 
add_definitions(-DUSE_GLAD)
message(STATUS "enabling OpenGL/GLAD output module")
endif()
endif()

#
# Boost
#
SET(Boost_USE_STATIC_LIBS ON)
find_package(Boost COMPONENTS log REQUIRED)

#add_library(Boost STATIC IMPORTED)

# also use colorisation files if they exists
get_filename_component(COLOR_CMAKE "colorize/CMakeLists.txt" ABSOLUTE)
if(EXISTS ${COLOR_CMAKE})
message(STATUS "enabling colorisation module")
add_subdirectory("colorize")
else()
message(STATUS "not enabling colorisation module")
endif()


add_executable (${PROJECT_NAME} "dmdreader.cpp" "util/crc32.cpp" "util/crc32.h" "render/framerenderer.h"  "dmd/dmdframe.cpp" "dmd/dmdframe.h" "render/framerenderer.cpp" "util/bmp.h" "util/bmp.cpp"  "dmd/color.cpp" "util/endian.h" "util/endian.cpp" "processor/pubcapture.cpp" "processor/frameprocessor.h" "processor/frameprocessor.cpp" "util/image.h" "util/image.cpp" "dmdsource/dmdsource.h" "dmdsource/dmdsource.cpp" "util/objectfactory.h" "util/objectfactory.cpp" "util/endian.cpp"  "dmdsource/datdmdsource.h" "dmdsource/datdmdsource.cpp" "dmdsource/txtdmdsource.h" "dmdsource/txtdmdsource.cpp" "processor/frameinfologger.h" "processor/frameinfologger.cpp"    "dmd/palette.h"     "dmd/maskeddmdframe.h" "dmd/maskedmdframe.cpp" "dmdsource/nullsource.cpp" "dmd/palette.cpp" "util/bithelper.cpp" "processor/palettecolorizer.h" "processor/palettecolorizer.cpp" "processor/framestore.h" "processor/framestore.cpp" ${color_sources} ${raylib_sources} ${angle_sources} ${opengl_sources})

target_include_directories(${PROJECT_NAME} PRIVATE ${raylib_INCLUDE_DIRS} ${PI_INCLUDE_DIRS} ${DRM_INCLUDE_DIR})
target_link_libraries(${PROJECT_NAME} PUBLIC ${raylib_LIBRARIES} ${Boost_LIBRARIES} ${angle_LIBRARIES} ${EXTRA_LIBS} ${OPENGL_LIBRARIES} ${PI_LIBS})
