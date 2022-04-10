cmake_minimum_required (VERSION 3.16)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(VCPKG_TARGET_TRIPLET=x64-windows-static)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")


add_definitions(-D_SILENCE_CXX17_ITERATOR_BASE_CLASS_DEPRECATION_WARNING)
set(BUILD_SHARED_LIBS OFF)
set(Boost_USE_STATIC_LIBS ON)

project ("DMDReader")

# detect Raspberry Pi
include(CheckIncludeFile)
CHECK_INCLUDE_FILE(/opt/vc/include/bcm_host.h RPI)
if (RPI)
message(STATUS "compiling on Raspberry Pi")
endif()

if(WIN32)
message(STATUS "compiling on Windows")
set(EXTRA_LIBS winmm.lib)
endif()

#
# Raylib (optional)
#
find_package(raylib CONFIG QUIET)
if (raylib_FOUND)
add_definitions(-DUSE_RAYLIB)
message(STATUS "enabling raylib output module")
set(raylib_sources "render/raylibrenderer.h" "render/raylibrenderer.cpp")
else()
message(STATUS "not enabling raylib output module")
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


add_executable (${PROJECT_NAME} "dmdreader.cpp" "util/crc32.cpp" "util/crc32.h" "render/framerenderer.h"  "dmd/dmdframe.cpp" "dmd/dmdframe.h" "render/framerenderer.cpp" "util/bmp.h" "util/bmp.cpp"  "dmd/color.cpp" "util/endian.h" "util/endian.cpp" "dmd/pubcapture.cpp" "dmd/frameprocessor.h" "dmd/frameprocessor.cpp" "util/image.h" "util/image.cpp" "dmdsource/dmdsource.h" "dmdsource/dmdsource.cpp" "util/objectfactory.h" "util/objectfactory.cpp" "util/endian.cpp"  "dmdsource/datdmdsource.h" "dmdsource/datdmdsource.cpp" "dmdsource/txtdmdsource.h" "dmdsource/txtdmdsource.cpp" "dmd/frameinfologger.h" "dmd/frameinfologger.cpp" "dmd/palette_colorizer.h" "dmd/palette_colorizer.cpp"  "dmd/palette.h"     "dmd/maskeddmdframe.h" "dmd/maskedmdframe.cpp" "dmdsource/nullsource.cpp" "dmd/palette.cpp" "util/bithelper.cpp" ${color_sources} ${raylib_sources})

target_include_directories(${PROJECT_NAME} PRIVATE ${raylib_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PUBLIC ${raylib_LIBRARIES} ${Boost_LIBRARIES} ${EXTRA_LIBS})
