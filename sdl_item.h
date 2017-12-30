/*
 sdl_item is a graphical library based on SDL.
 Copyright (C) 2013-2017 carabobz@gmail.com

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

#ifndef SIITEM_H
#define SIITEM_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "anim.h"
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL.h>

enum class SiLayout
{
	TOP_LEFT, CENTER
};

typedef struct anim_array
{
	anim_t ** list;
	int num; // Number of *anim in list
} anim_array_t;

typedef struct item
{
	SDL_Rect rect; // Current coordinate/size in pixels
	Uint32 anim_start_tick;	// Tick from when animation will be calculated
	int flip;
	double angle;
	double zoom_x;
	double zoom_y;
	int user1;	// User defined
	int user2;	// User defined
	int overlay;
	anim_array_t anim;		//default sprite
	anim_array_t anim_over;	//is set to default_anim_over, when needed (i.e. mouse over this item)
	anim_array_t default_anim_over;
	anim_array_t anim_click;//is set to default_anim_click, when needed (i.e. click on this item)
	anim_array_t default_anim_click;
	SiLayout layout;	// How to display array of anim (default is top-left)
	int anim_loop;
	int clicked;
	void (*click_left)(void * arg); //callback on left click on this item
	void * click_left_arg;
	void (*click_left_free)(void * arg); //if not NULL, used to free memory pointed by arf
	void (*click_right)(void * arg); //callback on right click on this item
	void * click_right_arg;
	void (*click_right_free)(void * arg); //if not NULL, used to free memory pointed by arf
	void (*double_click_left)(void * arg); //callback on double left click on this item
	void * double_click_left_arg;
	void (*double_click_left_free)(void * arg); //if not NULL, used to free memory pointed by arf
	void (*double_click_right)(void * arg); //callback on double right click on this item
	void * double_click_right_arg;
	void (*double_click_right_free)(void * arg); //if not NULL, used to free memory pointed by arf
	void (*wheel_up)(void * arg); //callback on mouse wheel up
	void * wheel_up_arg;
	void (*wheel_up_free)(void * arg); //if not NULL, used to free memory pointed by arf
	void (*wheel_down)(void * arg); //callback on mouse wheel down
	void * wheel_down_arg;
	void (*wheel_down_free)(void * arg); //if not NULL, used to free memory pointed by arf
	void (*over)(void * arg, int x, int y); //callback on mouse over this item
	void * over_arg;
	void (*over_free)(void * arg); //if not NULL, used to free memory pointed by arf
	char * string;		// string centered on item
	char * m_Buffer;    // Keyboard input buffer
	size_t m_BufferSize; // Buffer size
	Uint32 string_bg;	// Background color RGBA
	TTF_Font * font;
	SDL_Texture * str_tex;
	int editable;
	void (*edit_cb)(void * arg);
	const void * user_ptr;
	const void * user1_ptr;
	struct item * next;	// next element in a list of item
	struct item * last;	// last element in a list of item
} item_t;

item_t * item_list_add(item_t ** item_list);
void item_list_free(item_t * item_list);
void item_init(item_t * item);
void item_set_pos(item_t * item, int x, int y);
void item_set_anim(item_t * item, anim_t * anim, int anim_index);
void item_set_anim_array(item_t * item, anim_t ** anim_array);
void item_set_anim_over(item_t * item, anim_t * anim, int anim_index);
void item_set_anim_over_array(item_t * item, anim_t ** anim);
void item_set_anim_click(item_t * item, anim_t * anim, int anim_index);
void item_set_anim_click_array(item_t * item, anim_t ** anim);

void item_set_anim_shape(item_t * item, int x, int y, int w, int h);
void item_set_user(item_t * item, int user1, int user2);
void item_set_angle(item_t * item, double a);
void item_set_zoom_x(item_t * item, double a);
void item_set_zoom_y(item_t * item, double a);
void item_set_flip(item_t * item, int a);
void item_set_overlay(item_t * item, int overlay);
void item_set_click_left(item_t * item, void (*click_left)(void * arg),
		void * click_left_arg, void (*free_func)(void *ptr));
void item_set_click_right(item_t * item, void (*click_right)(void * arg),
		void * click_right_arg, void (*free_func)(void *ptr));
void item_set_double_click_left(item_t * item, void (*click_left)(void * arg),
		void * click_left_arg, void (*free_func)(void *ptr));
void item_set_double_click_right(item_t * item, void (*click_right)(void * arg),
		void * click_right_arg, void (*free_func)(void *ptr));
void item_set_wheel_up(item_t * item, void (*cb_wheel_up)(void * arg),
		void * wheel_up_arg, void (*free_func)(void *ptr));
void item_set_wheel_down(item_t * item, void (*cb_wheel_down)(void * arg),
		void * wheel_down_arg, void (*free_func)(void *ptr));
/* x,y is the mouse pointer position relative to the item itself.
 i.e. 0,0 is the mouse pointer is in the upper-left corner of the item */
void item_set_over(item_t * item, void (*over)(void * arg, int x, int y),
		void * over_arg, void (*free_func)(void *ptr));
void item_set_string(item_t * item, const char * buf);
void item_set_buffer(item_t * item, char * buf,const size_t p_BufferSize);
void item_set_string_bg(item_t * item, Uint32 color);
void item_set_editable(item_t * item, int is_editable);
void item_set_edit_cb(item_t * item, void (*cb_edit)(void * arg));
void item_set_geometry(item_t * item, int x, int y, int w, int h);
void item_set_layout(item_t * item, SiLayout layout);
void item_set_anim_loop(item_t * item, int loop);
void item_set_font(item_t * item, TTF_Font * font);
void item_set_anim_start_tick(item_t * item, Uint32 tick);

#ifdef __cplusplus
}
#endif

#endif
