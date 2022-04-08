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

add_executable (${PROJECT_NAME} "dmdreader.cpp" "dmdreader.h" "util/crc32.cpp" "util/crc32.h" "render/framerenderer.h" "render/raylibrenderer.h" "render/raylibrenderer.cpp"  "dmd/dmdframe.cpp" "dmd/dmdframe.h" "render/framerenderer.cpp" "util/bmp.h" "util/bmp.cpp"    "dmd/color.cpp" "util/endian.h" "util/endian.cpp" "dmd/pubcapture.cpp" "dmd/frameprocessor.h" "dmd/frameprocessor.cpp" "util/image.h" "util/image.cpp" "dmdsource/dmdsource.h" "dmdsource/dmdsource.cpp" "util/objectfactory.h" "util/objectfactory.cpp" "util/endian.cpp"  "colorize/palcoloring.h"  "colorize/palpalette.h" "colorize/palcoloring.cpp" "colorize/palmapping.h" "colorize/switchmode.h" "colorize/palmapping.cpp" "colorize/streamhelper.h" "colorize/streamhelper.cpp" "colorize/palpalette.cpp" "colorize/animationframe.h" "colorize/animationframe.cpp" "colorize/animationplane.h" "colorize/animationset.h" "colorize/animation.h" "colorize/animation.cpp" "colorize/vnianimation.h" "colorize/vnianimation.cpp" "colorize/vnianimationframe.h" "colorize/vnianimationframe.cpp" "util/bithelper.h" "util/bithelper.cpp" "colorize/vnianimationplane.h"  "colorize/vnianimationplane.cpp" "colorize/vnianimationset.h" "colorize/vnianimationset.cpp" "colorize/animationset.cpp"  "colorize/heatshrink_config.h" "colorize/heatshrink_decoder.h" "colorize/heatshrink_decoder.cpp" "colorize/pin2dmdcolorisation.h" "colorize/pin2dmdcolorisation.cpp" "colorize/config.h" "dmdsource/datdmdsource.h" "dmdsource/datdmdsource.cpp" "dmdsource/txtdmdsource.h" "dmdsource/txtdmdsource.cpp" "dmd/frameinfologger.h" "dmd/frameinfologger.cpp" "dmd/palette_colorizer.h" "dmd/palette_colorizer.cpp"  "dmd/palette.h"     "dmd/maskeddmdframe.h" "dmd/maskedmdframe.cpp" "dmdsource/nullsource.cpp" "dmd/palette.cpp")

target_include_directories(${PROJECT_NAME} PRIVATE ${raylib_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PUBLIC ${raylib_LIBRARIES} ${Boost_LIBRARIES} winmm.lib)

# this needs to be improved
#add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
#    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${VCPKG_INSTALLED_DIR}\\${VCPKG_TARGET_TRIPLET}\\bin\\raylib.dll $<TARGET_FILE_DIR:${PROJECT_NAME}>
#    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${VCPKG_INSTALLED_DIR}\\${VCPKG_TARGET_TRIPLET}\\bin\\boost_log_vc143-mt-x64-1_78.dll $<TARGET_FILE_DIR:${PROJECT_NAME}>
#    )