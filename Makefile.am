noinst_LIBRARIES=libsdl_item.a
libsdl_item_a_SOURCES=sdl.c item.c anim.c

AV_LIB=-lavcodec -lavformat -lswscale
SDL_LIB=`sdl2-config --libs`

libsdl_item_a_LIBADD=$(ADD_LIB)
libsdl_item_a_CFLAGS=$(AM_CFLAGS)

dist_noinst_HEADERS = sdl.h item.h anim.h
