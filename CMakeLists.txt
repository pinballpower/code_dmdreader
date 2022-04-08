cmake_minimum_required (VERSION 3.16)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(VCPKG_TARGET_TRIPLET=x64-windows-static)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")


add_definitions(-D_SILENCE_CXX17_ITERATOR_BASE_CLASS_DEPRECATION_WARNING)
set(BUILD_SHARED_LIBS OFF)
set(Boost_USE_STATIC_LIBS ON)

project ("DMDReader")

find_package(raylib CONFIG REQUIRED) # Requires at least version 4.0
# SET(Boost_USE_STATIC_LIBS ON)
find_package(Boost COMPONENTS log REQUIRED)

#add_library(Boost STATIC IMPORTED)

# also use colorisation files if they exists
get_filename_component(COLOR_CMAKE "colorize/CMakeLists.txt" ABSOLUTE)
if(EXISTS ${COLOR_CMAKE})
add_subdirectory("colorize")
endif()
message(STATUS "color source files: ${color_sources}")
add_executable (${PROJECT_NAME} "dmdreader.cpp" "dmdreader.h" "util/crc32.cpp" "util/crc32.h" "render/framerenderer.h" "render/raylibrenderer.h" "render/raylibrenderer.cpp"  "dmd/dmdframe.cpp" "dmd/dmdframe.h" "render/framerenderer.cpp" "util/bmp.h" "util/bmp.cpp"    "dmd/color.cpp" "util/endian.h" "util/endian.cpp" "dmd/pubcapture.cpp" "dmd/frameprocessor.h" "dmd/frameprocessor.cpp" "util/image.h" "util/image.cpp" "dmdsource/dmdsource.h" "dmdsource/dmdsource.cpp" "util/objectfactory.h" "util/objectfactory.cpp" "util/endian.cpp"  "dmdsource/datdmdsource.h" "dmdsource/datdmdsource.cpp" "dmdsource/txtdmdsource.h" "dmdsource/txtdmdsource.cpp" "dmd/frameinfologger.h" "dmd/frameinfologger.cpp" "dmd/palette_colorizer.h" "dmd/palette_colorizer.cpp"  "dmd/palette.h"     "dmd/maskeddmdframe.h" "dmd/maskedmdframe.cpp" "dmdsource/nullsource.cpp" "dmd/palette.cpp" "colorize/animationplane.cpp" ${color_sources} )

target_include_directories(${PROJECT_NAME} PRIVATE ${raylib_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PUBLIC ${raylib_LIBRARIES} ${Boost_LIBRARIES} winmm.lib)
