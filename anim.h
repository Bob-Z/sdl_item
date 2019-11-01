/*
 sdl_item is a graphical library based on SDL.
 Copyright (C) 2013-2019 carabobz@gmail.com

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

#ifndef ANIM_H
#define ANIM_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <SDL2/SDL.h>

typedef struct
{
	int num_frame;
	SDL_Texture ** tex;
	int w; // width
	int h; // height
	Uint32 * delay; //delay between each frame in millisecond
	Uint32 total_duration;
} anim_t;

anim_t * anim_load(const char * filename);
anim_t * anim_create_color(Uint32 width, Uint32 height, Uint32 color);
void si_anim_free(anim_t * anim);

#ifdef __cplusplus
}
#endif

#endif

