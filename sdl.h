/*
   World of Gnome is a 2D multiplayer role playing game.
   Copyright (C) 2013-2014 carabobz@gmail.com

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software Foundation,
   Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
*/

#include <SDL2/SDL.h>
#include "item.h"

#define SDL_OPAQUE 0xff
#define SDL_TRANSPARENT 0x00

#define RMASK 0x00ff0000
#define GMASK 0x0000ff00
#define BMASK 0x000000ff
#define AMASK 0xff000000

#define DEFAULT_SCREEN_W 640
#define DEFAULT_SCREEN_H 480

#define FRAME_DELAY 20

#define VIRTUAL_ANIM_DURATION 150

//#define PAL_TO_RGB(x) x.r<<2,x.g<<2,x.b<<2,SDL_OPAQUE
#define PAL_TO_RGB(x) x.r,x.g,x.b,SDL_OPAQUE

typedef struct keycb {
        SDL_Scancode code;
        void (*cb)(void*);
        void (*cb_up)(void*);
		void * arg;
        struct keycb * next;
} keycb_t;

void sdl_init(const char * title, SDL_Renderer ** render,SDL_Window ** window, void (*screen_compose_cb)(void));
void sdl_cleanup(void);
void sdl_set_pixel(SDL_Surface *surface, int x, int y, Uint32 R, Uint32 G, Uint32 B, Uint32 A);
void sdl_mouse_manager(SDL_Renderer *,SDL_Event * event, item_t * item_list);
int sdl_screen_manager(SDL_Window * window,SDL_Renderer * render,SDL_Event * event);
void sdl_loop_manager();
void sdl_blit_tex(SDL_Renderer *,SDL_Texture * tex, SDL_Rect * rect, double angle, double zoom_x,double zoom_y, int flip, int overlay);
int sdl_blit_anim(SDL_Renderer *,anim_t * anim, SDL_Rect * rect, double angle, double zoom_x, double zoom_y, int flip, int start, int end,int overlay);
void sdl_get_string_size(TTF_Font * font,const char * string,int * w,int *h);
void sdl_print_item(SDL_Renderer *,item_t * item);
int sdl_blit_item(SDL_Renderer *,item_t * item);
void sdl_blit_item_list(SDL_Renderer *,item_t * item_list);
void sdl_keyboard_text_init(char * buf, void (*cb)(void*arg));
void sdl_keyboard_text_reset();
char * sdl_keyboard_text_get_buf();
void sdl_keyboard_manager(SDL_Event * event);
void sdl_blit_to_screen(SDL_Renderer *);
void sdl_set_virtual_x(int x);
void sdl_set_virtual_y(int y);
void sdl_set_virtual_z(double z);
int sdl_get_virtual_x();
int sdl_get_virtual_y();
double sdl_get_virtual_z();
void sdl_force_virtual_x(int x);
void sdl_force_virtual_y(int y);
void sdl_force_virtual_z(double z);
keycb_t * sdl_add_keycb(SDL_Scancode code,void (*cb)(void*),void (*cb_up)(void*),void * arg);
void sdl_free_keycb(keycb_t ** key);

