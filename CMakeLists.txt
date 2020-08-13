cmake_minimum_required(VERSION 2.8)
enable_language(CXX)
enable_language(C)

project(sdl_item)
set(VERSION 0.0.0)

find_package(PkgConfig REQUIRED)

pkg_search_module(SDL2 REQUIRED sdl2)
pkg_search_module(SDL2TTF REQUIRED SDL2_ttf>=2.0.12)
include_directories(
        ${SDL2_INCLUDE_DIRS}
        ${SDL2TTF_INCLUDE_DIRS}
)

pkg_search_module(LIBAVFORMAT REQUIRED libavformat>=53.21.1)
pkg_search_module(LIBAVCODEC REQUIRED libavcodec>=53.35.0)
pkg_search_module(LIBAVUTIL REQUIRED libavutil)
pkg_search_module(LIBSWSCALE REQUIRED libswscale >= 2.1.0)
pkg_search_module(LIBPNG REQUIRED libpng)
pkg_search_module(LIBZIP REQUIRED libzip)

include_directories(
        ${LIBAVFORMAT_INCLUDE_DIRS}
        ${LIBAVCODEC_INCLUDE_DIRS}
        ${LIBAVUTIL_INCLUDE_DIRS}
        ${LIBSWSCALE_INCLUDE_DIRS}
        ${LIBPNG_INCLUDE_DIRS}
        ${LIBZIP_INCLUDE_DIRS}
)

include_directories(.)
include_directories(include)
include_directories(media)

add_library(
	${PROJECT_NAME}
	SHARED
	SiAnim.cpp
	sdl.cpp
	SdlItem.cpp
	SiKeyCallback.cpp
	SiMouseEvent.cpp
	SiTexture.cpp
	media/reader.cpp
	media/si_gif.cpp
	media/si_libav.cpp
	media/si_png.cpp
	media/si_zip.cpp
)

target_link_libraries (${PROJECT_NAME}
        ${SDL2_LIBRARIES}
        ${SDL2TTF_LIBRARIES}
        ${LIBAVFORMAT_LIBRARIES}
        ${LIBAVCODEC_LIBRARIES}
        ${LIBAVUTIL_LIBRARIES}
        ${LIBSWSCALE_LIBRARIES}
        ${LIBPNG_LIBRARIES}
        ${LIBZIP_LIBRARIES}
	-lgif
)

