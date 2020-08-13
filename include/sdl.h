/*
 sdl_item is a graphical library based on SDL.
 Copyright (C) 2013-2020 carabobz@gmail.com

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

#include "reader.h"
#include "SdlItem.h"
#include <functional>
#include <SDL2/SDL.h>
#include <SiAnim.h>
#include <string>
#include <vector>

#define SDL_OPAQUE 0xff
#define SDL_TRANSPARENT 0x00

#define RMASK 0x00ff0000
#define GMASK 0x0000ff00
#define BMASK 0x000000ff
#define AMASK 0xff000000

#define FRAME_DELAY 20

#define VIRTUAL_ANIM_DURATION 150
#define VIRTUAL_CAMERA_ANIM_DURATION 150

//#define PAL_TO_RGB(x) x.r<<2,x.g<<2,x.b<<2,SDL_OPAQUE
#define PAL_TO_RGB(x) x.r,x.g,x.b,SDL_OPAQUE

#define MOUSE_MOTION		0
#define MOUSE_BUTTON_UP		1
#define MOUSE_BUTTON_DOWN	2
#define MOUSE_WHEEL_UP		3
#define MOUSE_WHEEL_DOWN	4

void sdl_init(const std::string & title, const bool vsync);
void sdl_cleanup(void);
SDL_Renderer * sdl_get_renderer();

void sdl_set_pixel(SDL_Surface *surface, int x, int y, Uint32 R, Uint32 G, Uint32 B, Uint32 A);
Uint32 sdl_get_pixel(SDL_Surface *surface, int x, int y);

// Return true if a mouse event has been detected
bool sdl_mouse_manager(SDL_Event * event, std::vector<SdlItem*> & itemArray);

void sdl_mouse_position_manager(std::vector<SdlItem *> & itemArray);
int sdl_screen_manager(SDL_Event * event);
void sdl_loop_manager();
void sdl_blit_tex(SDL_Texture * tex, SDL_Rect * rect, double angle, double zoomX, double zoomY, int flip, int overlay);
int sdl_blit_anim(const SiAnim & anim, SDL_Rect * rect, const double angle, const double zoomX, const double zoomY, const bool isFlip, const bool isLoop,
		const bool isOverlay, const Uint32 animStartTick);
void sdl_get_string_size(TTF_Font * font, const std::string & text, int * w, int *h);
void sdl_print_item(SdlItem & item);
int sdl_blit_item(SdlItem & item);
void sdl_blit_item_list(std::vector<SdlItem> & itemArray);
void sdl_keyboard_text_init(std::string * buf, const std::function<void(std::string)>& editCb);
void sdl_init_screen();
const std::string & sdl_keyboard_text_get_buf();

// Return true is a key event has been detected
bool sdl_keyboard_manager(SDL_Event * event);

void sdl_blit_to_screen();
void sdl_set_virtual_x(int x);
void sdl_set_virtual_y(int y);
void sdl_set_virtual_z(double z);
int sdl_get_virtual_x();
int sdl_get_virtual_y();
double sdl_get_virtual_z();
void sdl_force_virtual_x(int x);
void sdl_force_virtual_y(int y);
void sdl_force_virtual_z(double z);
void sdl_add_down_key_cb(const SDL_Scancode code, const std::function<void()> & downCb);
void sdl_add_up_key_cb(const SDL_Scancode code, const std::function<void()> & upCb);
void sdl_clean_key_cb();
void sdl_add_mousecb(Uint32 event_type, std::function<void()> callBack);
void sdl_free_mousecb();
Uint32 sdl_get_global_time();
SiAnim * sdl_get_minimal_anim();
void sdl_set_background_color(int R, int G, int B, int A);
void sdl_get_output_size(int * width, int * height);
void sdl_clear();
