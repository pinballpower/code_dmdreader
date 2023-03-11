cmake_minimum_required (VERSION 3.16)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(VCPKG_TARGET_TRIPLET=x64-windows-static)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

add_definitions(-D_SILENCE_CXX17_ITERATOR_BASE_CLASS_DEPRECATION_WARNING)

project ("dmdreader")

message(STATUS "$ENV{CMAKE_INCLUDE_PATH}")

# detect Raspberry Pi
if (RPI) 
message(STATUS "RPI defined")
else()
find_path(RPI bcm_host.h)
endif()
if (RPI)
message(STATUS "compiling for Raspberry Pi")
set (CONFIGDIR "/etc/dmdreader")
else()
message(STATUS "not compiling for Raspberry Pi")
endif()

if(WIN32)
message(STATUS "compiling on Windows")
set(EXTRA_LIBS winmm.lib)
set(ENABLE_BOOST_STATIC_LIBS ON)
set (CONFIGDIR "/temp")
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

find_path(GPIOD_INCLUDE_DIR "gpiod.hpp")
if (GPIOD_INCLUDE_DIR)
message(STATUS "gpiod.hpp found in ${GPIOD_INCLUDE_DIR}")
include_directories(${GPIOD_INCLUDE_DIR})
else()
message(FATAL_ERROR "gpiod.hpp not found")
endif()

add_definitions(-DUSE_OPENGLPI4)
add_definitions(-DUSE_SPI)
# set (PI_INCLUDE_DIRS "/usr/include:/usr/include/stb:/usr/include/libdrm")
set (PI_LIBRARIES EGL drm gbm GLESv2 gpiodcxx)
set (OPENGL_SRC "render/pi4renderer.hpp" "render/pi4renderer.cpp" "render/openglrenderer.hpp" "render/openglrenderer.cpp" "render/openglshader.hpp" "render/openglshader.cpp")
set (SPI_SRC "dmdsource/spisource.hpp" "dmdsource/spisource.cpp" )
set (RPI_SRC "drm/drmhelper.hpp" "drm/drmhelper.cpp"   "rpi/spi.hpp" "rpi/spi.cpp" "rpi/gpio.hpp" "rpi/gpio.cpp")

# libavutil
find_path(LIBAV_INCLUDE_DIR "libavutil/hwcontext_drm.h")
if (LIBAV_INCLUDE_DIR)
message(STATUS "libavutil/hwcontext_drm.h found in ${LIBAV_INCLUDE_DIR}")
add_definitions(-DUSE_VIDEO)
include_directories(${LIBAV_INCLUDE_DIR})
set (VIDEO_SRC "drm/drmprimeout.cpp" "drm/drmprimeout.hpp" "drm/videoplayer.cpp" "drm/videoplayer.hpp" "drm/drmframebuffer.hpp" "drm/drmframebuffer.cpp" )
set (PUPPLAYER_SRC "pupplayer/pupplayer.hpp" "pupplayer/pupplayer.cpp" "drm/videofile.hpp" "drm/videofile.cpp" "drm/videofileloader.cpp" "drm/videofileloader.hpp" "pupplayer/trigger.hpp" "pupplayer/trigger.cpp" "pupplayer/screen.hpp" "pupplayer/screen.cpp" "pupplayer/playlist.hpp" "pupplayer/playlist.cpp" "pupplayer/pivid.cpp" "pupplayer/pivid.hpp" "pupplayer/internalpupplayer.hpp" "pupplayer/internalpupplayer.cpp" "pupplayer/pividpupplayer.hpp" "pupplayer/pividpupplayer.cpp")
set (VIDEO_LIBRARIES avcodec avfilter avutil avformat drm)
else()
message(FATAL_ERROR "libavutil/hwcontext_drm.h not found, you might need to install libavutil-dev ")
endif()

find_path(LIBAVCODEC_INCLUDE_DIR "libavcodec/avcodec.h")
if (LIBAVCODEC_INCLUDE_DIR)
include_directories(${LIBAVCODEC_INCLUDE_DIR})
message(STATUS "libavcodec/avcodec.h found in ${LIBAVCODEC_INCLUDE_DIR}")
else()
message(FATAL_ERROR "libavcodec/avcodec.h not found, you might need to install libavcodec-dev")
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

# use libserum if it exists
get_filename_component(SERUM_CMAKE "external/libserum/CMakeLists.txt" ABSOLUTE)
if(EXISTS ${SERUM_CMAKE})
message(STATUS "enabling SERUM colorisation module")
add_subdirectory("external/libserum")
add_definitions(-DUSE_SERUM)
set (SERUM_SRC "processor/serumcolorizer.cpp" "processor/serumcolorizer.hpp")

set (SERUM_LIBRARIES Serum)

message(STATUS ${LIBRARY_SOURCES})
else()
message(STATUS "libserum source not found in external/libserum, not enabling it")
endif()


set (UTIL_SRC "util/data.hpp" "util/data.cpp" "util/crc32.cpp" "util/crc32.hpp" "dmd/color.cpp" "util/image.hpp" "util/image.cpp" "util/objectfactory.hpp" "util/objectfactory.cpp" "util/bithelper.cpp" "util/bithelper.hpp" "stb/stb_image.h" "stb/stb_image_write.h" "util/glob.hpp" "util/glob.cpp")
set (RENDER_SRC "render/framerenderer.hpp"  "render/framerenderer.cpp" )
set (DMDSOURCE_SRC  "dmdsource/pngsource.hpp" "dmdsource/pngsource.cpp" "dmdsource/dmdsource.hpp" "dmdsource/dmdsource.cpp" "dmdsource/datdmdsource.hpp" "dmdsource/datdmdsource.cpp" "dmdsource/nullsource.cpp" "dmdsource/txtdmdsource.hpp" "dmdsource/txtdmdsource.cpp")
set (DMD_SRC "dmd/palette.hpp" "dmd/maskeddmdframe.hpp" "dmd/maskeddmdframe.cpp" "dmd/dmdframe.cpp" "dmd/dmdframe.hpp"  "dmd/palette.cpp"  )
set (PROCESSOR_SRC "processor/pupcapture.cpp" "processor/frameprocessor.hpp" "processor/frameprocessor.cpp" "processor/frameinfologger.hpp" "processor/frameinfologger.cpp" "processor/palettecolorizer.hpp" "processor/palettecolorizer.cpp" "processor/txtwriter.hpp" "processor/txtwriter.cpp" "processor/genericwriter.hpp" "processor/genericwriter.cpp" "processor/pngwriter.hpp" "processor/pngwriter.cpp")
set (SCORE_SRC "processor/statedetector.hpp" "processor/statedetector.cpp" "processor/patternmatcher.cpp" "processor/patternmatcher.hpp" "processor/patterndetector.hpp" "processor/patterndetector.cpp")
set (SERVICES_SRC "services/service.cpp" "services/service.hpp" "services/serviceregistry.hpp" "services/serviceregistry.cpp" "services/pupeventplayback.hpp" "services/pupeventplayback.cpp")
set (TEST_SRC "test/drmtest.hpp"  "test/drmtest.cpp" "test/scoredetector.cpp" "test/scoredetector.hpp")

set (ALL_SOURCES ${UTIL_SRC} ${RENDER_SRC} ${DMD_SRC} ${DMDSOURCE_SRC} ${PROCESSOR_SRC} ${COLOR_SRC} ${OPENGL_SRC} ${SPI_SRC} ${RPI_SRC} ${VIDEO_SRC} ${PUPPLAYER_SRC} ${SERVICES_SRC} ${TEST_SRC} ${SCORE_SRC} ${SERUM_SRC})
message(STATUS ${ALL_SOURCES})

add_executable (${PROJECT_NAME} "dmdreader.cpp" ${ALL_SOURCES} )
target_include_directories(${PROJECT_NAME} PRIVATE ${PI_INCLUDE_DIRS} ${DRM_INCLUDE_DIR})
target_link_libraries(${PROJECT_NAME} PUBLIC ${Boost_LIBRARIES} ${EXTRA_LIBS} ${OPENGL_LIBRARIES} ${PI_LIBRARIES} ${ZLIB_LIBRARIES} ${VIDEO_LIBRARIES} ${SERUM_LIBRARIES})

add_executable (test.bin "test.cpp" ${ALL_SOURCES} )
target_include_directories(test.bin PRIVATE ${PI_INCLUDE_DIRS} ${DRM_INCLUDE_DIR})
target_link_libraries(test.bin PUBLIC ${Boost_LIBRARIES} ${EXTRA_LIBS} ${OPENGL_LIBRARIES} ${PI_LIBRARIES} ${ZLIB_LIBRARIES} ${VIDEO_LIBRARIES} ${SERUM_LIBRARIES})


#
# Install
#
install(TARGETS ${PROJECT_NAME} DESTINATION bin)
install(DIRECTORY shaders DESTINATION ${CONFIGDIR} FILES_MATCHING PATTERN "*.?s")
install(DIRECTORY sample-configs DESTINATION ${CONFIGDIR} FILES_MATCHING PATTERN "*.json")
install(DIRECTORY img DESTINATION ${CONFIGDIR} FILES_MATCHING PATTERN "*.png")
install(FILES sample-configs/pispi.json DESTINATION ${CONFIGDIR} RENAME dmdreader.json) # Default config
