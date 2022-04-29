cmake_minimum_required (VERSION 3.16)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(VCPKG_TARGET_TRIPLET=x64-windows-static)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

add_definitions(-D_SILENCE_CXX17_ITERATOR_BASE_CLASS_DEPRECATION_WARNING)

project ("dmdeader")

message(STATUS "$ENV{CMAKE_INCLUDE_PATH}")

# detect Raspberry Pi
if (RPI) 
message(STATUS "RPI defined")
else()
find_path(RPI bcm_host.h)
endif()
if (RPI)
message(STATUS "compiling for Raspberry Pi")
else()
message(STATUS "not compiling for Raspberry Pi")
endif()

if(WIN32)
message(STATUS "compiling on Windows")
set(EXTRA_LIBS winmm.lib)
endif()

#
# Pi4
#
if (RPI)
message(STATUS "configuring OpenGL ES for Raspberry Pi")

find_path(DRM_INCLUDE_DIR "drm.h" /usr/include/libdrm)
if (DRM_INCLUDE_DIR)
message(STATUS "drm.h found in ${DRM_INCLUDE_DIR}")
include_directories(${DRM_INCLUDE_DIR})
else()
message(FATAL_ERROR "drm.h not found")
endif()

find_path(EGL_INCLUDE_DIR "egl.h" /usr/include/EGL)
if (EGL_INCLUDE_DIR)
message(STATUS "egl.h found in ${EGL_INCLUDE_DIR}")
include_directories(${EGL_INCLUDE_DIR})
else()
message(FATAL_ERROR "egl.h not found")
endif()

find_path(XF86_INCLUDE_DIR "xf86drm.h")
if (XF86_INCLUDE_DIR)
message(STATUS "xf86drm.h found in ${XF86_INCLUDE_DIR}")
include_directories(${XF86_INCLUDE_DIR})
else()
message(FATAL_ERROR "xf86drm.h not found")
endif()

find_path(GL2_INCLUDE_DIR "gl2.h" /usr/include/GLES2)
if (GL2_INCLUDE_DIR)
message(STATUS "gl2.h found in ${GL2_INCLUDE_DIR}")
include_directories(${GL2_INCLUDE_DIR})
else()
message(FATAL_ERROR "gl2.h not found")
endif()

add_definitions(-DUSE_OPENGLPI4)
add_definitions(-DUSE_SPI)
# set (PI_INCLUDE_DIRS "/usr/include:/usr/include/stb:/usr/include/libdrm")
set (PI_LIBS EGL drm gbm GLESv2)
set (opengl_sources "render/pi4renderer.h" "render/pi4renderer.cpp" "render/openglrenderer.h" "render/openglrenderer.cpp" "render/openglshader.h" "render/openglshader.cpp")
set (spi_sources "dmdsource/spisource.h" "dmdsource/spisource.cpp" "rpi/spi.h" "rpi/spi.cpp" "rpi/gpio.h" "rpi/gpio.cpp")
endif()


#
# OpenGL/GLAD (only tested under Windows)
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
if (ENABLE_BOOST_STATIC_LIBS)
set(Boost_USE_STATIC_LIBS ON)
else()
set(Boost_USE_STATIC_LIBS OFF)
add_definitions (-DBOOST_LOG_DYN_LINK=1)
endif()
find_package(Boost COMPONENTS log system iostreams REQUIRED)
include_directories( ${Boost_INCLUDE_DIR} )
link_directories(${Boost_LIBRARY_DIR})
message(STATUS "Boost libs: ${Boost_LIBRARIES}")
find_package(ZLIB REQUIRED)


# also use colorisation files if they exists
get_filename_component(COLOR_CMAKE "colorize/CMakeLists.txt" ABSOLUTE)
if(EXISTS ${COLOR_CMAKE})
message(STATUS "enabling colorisation module")
add_subdirectory("colorize")
else()
message(STATUS "not enabling colorisation module")
endif()


add_executable (${PROJECT_NAME} "dmdreader.cpp" "util/crc32.cpp" "util/crc32.h" "render/framerenderer.h"  "dmd/dmdframe.cpp" "dmd/dmdframe.h" "render/framerenderer.cpp" "util/bmp.h" "util/bmp.cpp"  "dmd/color.cpp" "util/endian.h" "util/endian.cpp" "processor/pubcapture.cpp" "processor/frameprocessor.h" "processor/frameprocessor.cpp" "util/image.h" "util/image.cpp" "dmdsource/dmdsource.h" "dmdsource/dmdsource.cpp" "util/objectfactory.h" "util/objectfactory.cpp" "util/endian.cpp"  "dmdsource/datdmdsource.h" "dmdsource/datdmdsource.cpp" "dmdsource/txtdmdsource.h" "dmdsource/txtdmdsource.cpp" "processor/frameinfologger.h" "processor/frameinfologger.cpp"    "dmd/palette.h"     "dmd/maskeddmdframe.h" "dmd/maskedmdframe.cpp" "dmdsource/nullsource.cpp" "dmd/palette.cpp" "util/bithelper.cpp" "processor/palettecolorizer.h" "processor/palettecolorizer.cpp" "processor/framestore.h" "processor/framestore.cpp" "stb/stb_image.h" ${color_sources} ${opengl_sources} ${spi_sources})

target_include_directories(${PROJECT_NAME} PRIVATE ${PI_INCLUDE_DIRS} ${DRM_INCLUDE_DIR})
target_link_libraries(${PROJECT_NAME} PUBLIC ${Boost_LIBRARIES} ${EXTRA_LIBS} ${OPENGL_LIBRARIES} ${PI_LIBS} ${ZLIB_LIBRARIES})

install(TARGETS ${PROJECT_NAME} DESTINATION bin)