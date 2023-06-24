cmake_minimum_required (VERSION 3.16)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(VCPKG_TARGET_TRIPLET=x64-windows-static)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

add_definitions(-D_SILENCE_CXX17_ITERATOR_BASE_CLASS_DEPRECATION_WARNING)

option(FORCE_OPENGLES "force nable OpenGL ES" 0)
option(FORCE_SPI "force enable SPI" 0)

project ("dmdreader")

# message(STATUS "$ENV{CMAKE_INCLUDE_PATH}")

# detect Raspberry Pi
message(STATUS "Target architecture: " ${CMAKE_SYSTEM_PROCESSOR})
if(${CMAKE_SYSTEM_PROCESSOR} STREQUAL "armv7l")
    message(STATUS "Target architecture: ARM v7l, assuming Raspberry Pi")
    set (RPI 1)
endif()
if(${CMAKE_SYSTEM_PROCESSOR} STREQUAL "armv8l")
    message(STATUS "Target architecture: ARM v8l, assuming Raspberry Pi")
    set (RPI 1)
endif()

if (RPI)
message(STATUS "compiling for Raspberry Pi")
else()
message(STATUS "not compiling for Raspberry Pi")
endif()

if (UNIX)
message(STATUS "compiling for Unix")
set (CONFIGDIR "/etc/dmdreader")
endif()

if(WIN32)
message(STATUS "compiling on Windows")
set(EXTRA_LIBS winmm.lib)
set(ENABLE_BOOST_STATIC_LIBS ON)
set (CONFIGDIR "/temp")
endif()

if (MSVC)
    # see https://learn.microsoft.com/en-us/cpp/error-messages/compiler-warnings/compiler-warning-level-1-c4530
    add_compile_options(/EHsc)
endif()

#
# OpenGL ES
#
if (UNIX)
message(STATUS "checking for OpenGL ES")
set (OGL 1)

find_path(OPENGLES3_INCLUDE_DIR GLES3/gl3.h)
if (OPENGLES3_INCLUDE_DIR)
message(STATUS "GLES3/gl3.h found in ${OPENGLES3_INCLUDE_DIR}")
include_directories(${OPENGLES3_INCLUDE_DIR})
else()
message(STATUS "GLES3/gl3.h not found")
set(OGL 0)
endif()

find_path(DRM_INCLUDE_DIR libdrm/drm.h)
if (DRM_INCLUDE_DIR)
message(STATUS "drm.h found in ${DRM_INCLUDE_DIR}")
include_directories(${DRM_INCLUDE_DIR}/libdrm)
else()
message(STATUS "drm.h not found")
set(OGL 0)
endif()

find_path(OPENGLES3_INCLUDE_DIR GLES3/gl3.h)
message(STATUS ${OPENGLES3_INCLUDE_DIR})

find_path(EGL_INCLUDE_DIR "EGL/egl.h")
if (EGL_INCLUDE_DIR)
message(STATUS "egl.h found in ${EGL_INCLUDE_DIR}")
include_directories(${EGL_INCLUDE_DIR})
else()
message(STATUS "egl.h not found")
set(OGL 0)
endif()

find_path(XF86_INCLUDE_DIR "xf86drm.h")
if (XF86_INCLUDE_DIR)
message(STATUS "xf86drm.h found in ${XF86_INCLUDE_DIR}")
include_directories(${XF86_INCLUDE_DIR})
else()
message(STATUS "xf86drm.h not found")
set(OGL 0)
endif()

find_path(GL2_INCLUDE_DIR "gl2.h" /usr/include/GLES2)
if (GL2_INCLUDE_DIR)
message(STATUS "gl2.h found in ${GL2_INCLUDE_DIR}")
include_directories(${GL2_INCLUDE_DIR})
else()
message(STATUS "gl2.h not found")
set(OGL 0)
endif()

if (OGL)
message(STATUS "Enabling  OpenGL ES")
add_definitions(-DUSE_OPENGLPI4)
set (PI_LIBRARIES EGL drm gbm GLESv2 gpiodcxx)
set (OPENGL_SRC "render/pi4renderer.hpp" "render/pi4renderer.cpp" "render/openglrenderer.hpp" "render/openglrenderer.cpp" "render/openglshader.hpp" "render/openglshader.cpp" "drm/drmhelper.hpp" "drm/drmhelper.cpp")
else()
message(STATUS "Disabling OpenGL ES")
if (FORCE_OPENGLES)
message(FATAL_ERROR "couldn't enable OpenGL ES, aborting")
endif()
endif()

endif()

if (UNIX)
find_path(GPIOD_INCLUDE_DIR "gpiod.hpp")
if (GPIOD_INCLUDE_DIR)
message(STATUS "gpiod.hpp found in ${GPIOD_INCLUDE_DIR}")
include_directories(${GPIOD_INCLUDE_DIR})
set (SPI_SRC "dmdsource/spisource.hpp" "dmdsource/spisource.cpp" "rpi/spi.hpp" "rpi/spi.cpp" "rpi/gpio.hpp" "rpi/gpio.cpp")
add_definitions(-DUSE_SPI)
message(STATUS "Enabling SPI")
else()
message(STATUS "Disabling SPI")
if (FORCE_SPI)
message(FATAL_ERROR "couldn't enable SPI, aborting")
endif()
endif()
endif()


# PUPPlayer
if (UNIX)

set (VIDEO 1)
if (DRM_INCLUDE_DIR)
message(STATUS "drm.h found")
include_directories(${DRM_INCLUDE_DIR})
else()
message(STATUS "drm.h not found, disabling video")
set(VIDEO 0)
endif()

find_path(LIBAV_INCLUDE_DIR "libavutil/hwcontext_drm.h")
if (LIBAV_INCLUDE_DIR)
message(STATUS "libavutil/hwcontext_drm.h found in ${LIBAV_INCLUDE_DIR}")
else()
message(STATUS "libavutil/hwcontext_drm.h not found, disabling video")
set(VIDEO 0)
endif()

find_path(LIBAVCODEC_INCLUDE_DIR "libavcodec/avcodec.h")
if (LIBAVCODEC_INCLUDE_DIR)
include_directories(${LIBAVCODEC_INCLUDE_DIR})
message(STATUS "libavcodec/avcodec.h found in ${LIBAVCODEC_INCLUDE_DIR}")
else()
message(STATUS "libavcodec/avcodec.h not found, disabling video")
set (VIDEO 0)
endif()

if (VIDEO)
message(STATUS "Enabling PUP player")
add_definitions(-DUSE_VIDEO)
include_directories(${LIBAV_INCLUDE_DIR})
set (VIDEO_SRC "drm/drmprimeout.cpp" "drm/drmprimeout.hpp" "drm/videoplayer.cpp" "drm/videoplayer.hpp" "drm/drmframebuffer.hpp" "drm/drmframebuffer.cpp" )
set (PUPPLAYER_SRC "pupplayer/pupplayer.hpp" "pupplayer/pupplayer.cpp" "drm/videofile.hpp" "drm/videofile.cpp" "drm/videofileloader.cpp" "drm/videofileloader.hpp" "pupplayer/trigger.hpp" "pupplayer/trigger.cpp" "pupplayer/screen.hpp" "pupplayer/screen.cpp" "pupplayer/playlist.hpp" "pupplayer/playlist.cpp" "pupplayer/pivid.cpp" "pupplayer/pivid.hpp" "pupplayer/internalpupplayer.hpp" "pupplayer/internalpupplayer.cpp" "pupplayer/pividpupplayer.hpp" "pupplayer/pividpupplayer.cpp")
set (VIDEO_LIBRARIES avcodec avfilter avutil avformat drm)
else()
message(status "Disabling PUP Player")
endif()

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
set (OPENGL_SRC "render/openglrenderer.hpp" "render/openglrenderer.cpp" "render/openglshader.hpp" "render/openglshader.cpp" "render/gladopenglrenderer.hpp" "render/gladopenglrenderer.cpp")
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
message(STATUS "Enabling OpenGL/GLAD output")
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
message(STATUS "Enabling VNI/PAL colorisation module")
add_subdirectory("colorize")
else()
message(STATUS "Disabling VNI/PAL colorisation module")
endif()

# use libserum if it exists
get_filename_component(SERUM_CMAKE "external/libserum/CMakeLists.txt" ABSOLUTE)
if(EXISTS ${SERUM_CMAKE})
message(STATUS "Enabling SERUM colorisation module")
add_subdirectory("external/libserum")
add_definitions(-DUSE_SERUM)
set (SERUM_SRC "processor/serumcolorizer.cpp" "processor/serumcolorizer.hpp")
set (SERUM_LIBRARIES Serum)
add_library(libSerum STATIC IMPORTED)
else()
message(STATUS "Disabling SERUM colorisation module")
endif()

# use rpi-rgb-led-matrix
if (RPI)
get_filename_component(RGB_MAKEFILE "external/rpi-rgb-led-matrix/Makefile" ABSOLUTE)
if(EXISTS ${RGB_MAKEFILE})
message(STATUS "Enabling RGB LED matrix module")
file(GLOB_RECURSE RGB_SRC_FILES external/rpi-rgb-led-matrix/lib/*.c*)
file(GLOB_RECURSE RGB_HEADER_FILES external/rpi-rgb-led-matrix/include/*.h)
include_directories(external/rpi-rgb-led-matrix/lib)
include_directories(external/rpi-rgb-led-matrix/include)
add_definitions(-DUSE_LEDMATRIX)
set (RGB_SRC ${RGB_SRC_FILES} ${RGB_HEADER_FILES} render/ledmatrixrenderer.hpp render/ledmatrixrenderer.cpp)
else()
message(STATUS "Disabling RGB LED matrix module")
endif()
endif()

# XBR
get_filename_component(XBR_INCLUDE "external/xbr/internal_hqx_common.h" ABSOLUTE)
if(EXISTS ${XBR_INCLUDE})
message(STATUS "Enabling XBR upsampling")
include_directories(external/xbr)
add_definitions(-DUSE_XBR)
set (XBR_SRC external/xbr/xbr.c external/xbr/hq2x.c external/xbr/hq3x.c external/xbr/hq4x.c external/xbr/filters.h external/xbr/internal_hqx_common.h)
else()
message(STATUS "Disabling XBR upsampling")
endif()

set (UTIL_SRC "util/data.hpp" "util/data.cpp" "util/crc32.cpp" "util/crc32.hpp" "dmd/color.cpp" "util/image.hpp" "util/image.cpp" "util/objectfactory.hpp" "util/objectfactory.cpp" "util/bithelper.cpp" "util/bithelper.hpp" "stb/stb_image.h" "stb/stb_image_write.h" "util/glob.hpp" "util/glob.cpp")
set (RENDER_SRC "render/framerenderer.hpp"  "render/framerenderer.cpp" )
set (DMDSOURCE_SRC  "dmdsource/pngsource.hpp" "dmdsource/pngsource.cpp" "dmdsource/dmdsource.hpp" "dmdsource/dmdsource.cpp" "dmdsource/datdmdsource.hpp" "dmdsource/datdmdsource.cpp" "dmdsource/nullsource.cpp" "dmdsource/txtdmdsource.hpp" "dmdsource/txtdmdsource.cpp")
set (DMD_SRC "dmd/palette.hpp" "dmd/maskeddmdframe.hpp" "dmd/maskeddmdframe.cpp" "dmd/dmdframe.cpp" "dmd/dmdframe.hpp"  "dmd/palette.cpp"  )
set (PROCESSOR_SRC "processor/pupcapture.cpp" "processor/frameprocessor.hpp" "processor/frameprocessor.cpp" "processor/frameinfologger.hpp" "processor/frameinfologger.cpp" "processor/palettecolorizer.hpp" "processor/palettecolorizer.cpp" "processor/txtwriter.hpp" "processor/txtwriter.cpp" "processor/genericwriter.hpp" "processor/genericwriter.cpp" "processor/pngwriter.hpp" "processor/pngwriter.cpp" "processor/upscale.hpp" "processor/upscale.cpp" "processor/fulllevel.hpp" "processor/fulllevel.cpp")
set (SCORE_SRC "processor/statedetector.hpp" "processor/statedetector.cpp" "processor/patternmatcher.cpp" "processor/patternmatcher.hpp" "processor/patterndetector.hpp" "processor/patterndetector.cpp")
set (SERVICES_SRC "services/service.cpp" "services/service.hpp" "services/serviceregistry.hpp" "services/serviceregistry.cpp" "services/pupeventplayback.hpp" "services/pupeventplayback.cpp")
set (TEST_SRC "test/drmtest.hpp"  "test/drmtest.cpp" "test/scoredetector.cpp" "test/scoredetector.hpp")

set (THIRDPARTY_SRC ${SERUM_SRC} ${RGB_SRC} ${HQX_SRC} ${XBR_SRC})

set (ALL_SOURCES ${UTIL_SRC} ${RENDER_SRC} ${DMD_SRC} ${DMDSOURCE_SRC} ${PROCESSOR_SRC} ${COLOR_SRC} ${OPENGL_SRC} ${SPI_SRC} ${RPI_SRC} ${VIDEO_SRC} ${PUPPLAYER_SRC} ${SERVICES_SRC} ${TEST_SRC} ${SCORE_SRC} ${THIRDPARTY_SRC} ${SPI_SRC})

add_executable (${PROJECT_NAME} "dmdreader.cpp" ${ALL_SOURCES} )
target_include_directories(${PROJECT_NAME} PRIVATE ${PI_INCLUDE_DIRS} ${DRM_INCLUDE_DIR})
target_link_libraries(${PROJECT_NAME} PUBLIC ${Boost_LIBRARIES} ${EXTRA_LIBS} ${OPENGL_LIBRARIES} ${PI_LIBRARIES} ${ZLIB_LIBRARIES} ${VIDEO_LIBRARIES} ${SERUM_LIBRARIES})

if (COMPILE_TEST)
add_executable (test.bin "test.cpp" ${ALL_SOURCES} )
target_include_directories(test.bin PRIVATE ${PI_INCLUDE_DIRS} ${DRM_INCLUDE_DIR})
target_link_libraries(test.bin PUBLIC ${Boost_LIBRARIES} ${EXTRA_LIBS} ${OPENGL_LIBRARIES} ${PI_LIBRARIES} ${ZLIB_LIBRARIES} ${VIDEO_LIBRARIES} ${SERUM_LIBRARIES})
endif()


#
# Install
#
install(TARGETS ${PROJECT_NAME} DESTINATION bin)
install(DIRECTORY shaders DESTINATION ${CONFIGDIR} FILES_MATCHING PATTERN "*.?s")
install(DIRECTORY sample-configs DESTINATION ${CONFIGDIR} FILES_MATCHING PATTERN "*.json")
install(DIRECTORY img DESTINATION ${CONFIGDIR} FILES_MATCHING PATTERN "*.png")
install(FILES samples/dmd-to-led.json DESTINATION ${CONFIGDIR} RENAME dmdreader.json) # Default config
install(DIRECTORY external/libserum DESTINATION lib FILES_MATCHING PATTERN "*.so")
