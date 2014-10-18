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
#include "sdl.h"

/************************************************************************
Return a pointer to the newly creates item_t
************************************************************************/
item_t * item_list_add(item_t ** item_list)
{
	item_t * item;
	item_t * last_item;

	item = malloc(sizeof(item_t));
	item_init(item);

	/* add to item list */
	if(*item_list != NULL) {
		last_item = (*item_list)->last;
		last_item->next = item;
		(*item_list)->last = item;
	} else {
		*item_list = item;
		item->last = item;
	}

	return item;
}

/************************************************************************
Note that item->anim is not freed here !
************************************************************************/
static void item_free(item_t * item)
{
	if( item->string ) {
		free(item->string);
	}
	if( item->str_tex ) {
		SDL_DestroyTexture(item->str_tex);
	}
	if( item->click_left_free ) {
		item->click_left_free(item->click_left_arg);
	}
	if( item->click_right_free ) {
		item->click_right_free(item->click_right_arg);
	}
	if( item->double_click_left_free ) {
		item->double_click_left_free(item->double_click_left_arg);
	}
	if( item->double_click_right_free ) {
		item->double_click_right_free(item->double_click_right_arg);
	}
	if( item->wheel_up_free ) {
		item->wheel_up_free(item->wheel_up_arg);
	}
	if( item->wheel_down_free ) {
		item->wheel_down_free(item->wheel_down_arg);
	}
	if( item->over_free ) {
		item->over_free(item->over_arg);
	}

	free(item);
}

/************************************************************************
************************************************************************/
void item_list_free(item_t * item_list)
{
	if(item_list == NULL) {
		return;
	}

	item_list_free(item_list->next);

	item_free(item_list);
}

/************************************************************************
************************************************************************/
void item_init(item_t * item)
{
	item->rect.x=-1;
	item->rect.y=-1;
	item->rect.w=0;
	item->rect.h=0;
	item->x=-1;
	item->y=-1;
	item->angle=0;
	item->zoom_x=1.0;
	item->zoom_y=1.0;
	item->flip=SDL_FLIP_NONE;
	item->old_x=-1;
	item->old_y=-1;
	item->timer=0;
	item->overlay=0;
	item->anim=NULL;
	item->anim_start=0;
	item->anim_end=-1;
	item->current_frame=0;
	item->frame_normal=0;
	item->frame_over=0;
	item->anim_over=NULL;
	item->default_anim_over=NULL;
	item->frame_click=0;
	item->clicked=0;
	item->click_left=NULL;
	item->click_left_arg=NULL;
	item->click_left_free=NULL;
	item->click_right=NULL;
	item->click_right_arg=NULL;
	item->click_right_free=NULL;
	item->double_click_left=NULL;
	item->double_click_left_arg=NULL;
	item->double_click_left_free=NULL;
	item->double_click_right=NULL;
	item->double_click_right_arg=NULL;
	item->double_click_right_free=NULL;
	item->wheel_up=NULL;
	item->wheel_up_arg=NULL;
	item->wheel_up_free=NULL;
	item->wheel_down=NULL;
	item->wheel_down_arg=NULL;
	item->wheel_down_free=NULL;
	item->over=NULL;
	item->over_arg=NULL;
	item->over_free=NULL;
	item->string=NULL;
	item->string_bg=0; // Transparent black
	item->font=NULL;
	item->str_tex=NULL;
	item->editable=0;
	item->edit_cb=NULL;
	item->next=NULL;
	item->last=NULL;
}

/************************************************************************
************************************************************************/
void item_set_pos(item_t * item, int x, int y)
{
	item->rect.x = x;
	item->rect.y = y;
}

/************************************************************************
************************************************************************/
void item_set_frame(item_t * item, int x, int y,anim_t * anim)
{
	int w;
	int h;
	int max_w = 0;
	int max_h = 0;

	item_set_pos(item,x,y);

	if( anim ) {
		item->anim = anim;
		max_w = anim->w;
		max_h = anim->h;
	}
	if( item->string ) {
		sdl_get_string_size(item->font,item->string,&w,&h);
		if ( w > max_w ) {
			max_w = w;
		}
		if ( h > max_h ) {
			max_h = h;
		}
	}

	item->rect.w = max_w;
	item->rect.h = max_h;
}

/************************************************************************
************************************************************************/
void item_set_frame_shape(item_t * item, int x, int y,int w, int h)
{
	item_set_pos(item,x,y);
	item->rect.w = w;
	item->rect.h = h;
}

/************************************************************************
************************************************************************/
void item_set_anim(item_t * item, int x, int y,anim_t * anim)
{
	item_set_frame(item,x,y,anim);
	item->frame_normal = -1;
}

/************************************************************************
************************************************************************/
void item_set_smooth_anim(item_t * item, int x, int y,int old_x, int old_y, Uint32 timer, anim_t * anim)
{
	item->x = x;
	item->y = y;
	item->old_x = old_x;
	item->old_y = old_y;
	item->timer = timer;
	item_set_frame(item,x,y,anim);
	item->frame_normal = -1;
}

/************************************************************************
************************************************************************/
void item_set_tile(item_t * item, int x, int y)
{
	item->tile_x = x;
	item->tile_y = y;
}

/************************************************************************
************************************************************************/
void item_set_angle(item_t * item, double a)
{
	item->angle = a;
}

/************************************************************************
************************************************************************/
void item_set_zoom_x(item_t * item, double a)
{
	item->zoom_x = a;
}

/************************************************************************
************************************************************************/
void item_set_zoom_y(item_t * item, double a)
{
	item->zoom_y = a;
}

/************************************************************************
flip is one of SDL_FLIP_NONE, SDL_FLIP_HORIZONTAL, SDL_FLIP_VERTICAL
************************************************************************/
void item_set_flip(item_t * item, int a)
{
	item->flip = a;
}

/************************************************************************
************************************************************************/
void item_set_overlay(item_t * item, int overlay)
{
	item->overlay = overlay;
}

/************************************************************************
************************************************************************/
void item_set_frame_normal(item_t * item, int num_frame)
{
	item->frame_normal = num_frame;
}

/************************************************************************
************************************************************************/
void item_set_anim_start(item_t * item, int start_frame)
{
	item->anim_start = start_frame;
}

/************************************************************************
************************************************************************/
void item_set_anim_end(item_t * item, int end_frame)
{
	item->anim_end = end_frame;
}

/************************************************************************
************************************************************************/
void item_set_frame_over(item_t * item, int num_frame)
{
	item->frame_over = num_frame;
}

/************************************************************************
************************************************************************/
void item_set_anim_over(item_t * item, anim_t * anim)
{
	item->default_anim_over = anim;
}

/************************************************************************
************************************************************************/
void item_set_frame_click(item_t * item, int num_frame)
{
	item->frame_click = num_frame;
}

/************************************************************************
************************************************************************/
void item_set_click_left(item_t * item,void (*click_left)(void * arg),void * click_left_arg, void (*free_func)(void *ptr))
{
	item->click_left=click_left;
	item->click_left_arg=click_left_arg;
	item->click_left_free=free_func;
}

/************************************************************************
************************************************************************/
void item_set_click_right(item_t * item,void (*click_right)(void * arg),void * click_right_arg, void (*free_func)(void *ptr))
{
	item->click_right=click_right;
	item->click_right_arg=click_right_arg;
	item->click_right_free=free_func;
}

/************************************************************************
************************************************************************/
void item_set_double_click_left(item_t * item,void (*double_click_left)(void * arg),void * double_click_left_arg, void (*free_func)(void *ptr))
{
	item->double_click_left=double_click_left;
	item->double_click_left_arg=double_click_left_arg;
	item->double_click_left_free=free_func;
}

/************************************************************************
************************************************************************/
void item_set_double_click_right(item_t * item,void (*double_click_right)(void * arg),void * double_click_right_arg, void (*free_func)(void *ptr))
{
	item->double_click_right=double_click_right;
	item->double_click_right_arg=double_click_right_arg;
	item->double_click_right_free=free_func;
}

/************************************************************************
************************************************************************/
void item_set_wheel_up(item_t * item,void (*wheel_up)(void * arg),void * wheel_up_arg, void (*free_func)(void *ptr))
{
	item->wheel_up=wheel_up;
	item->wheel_up_arg=wheel_up_arg;
	item->wheel_up_free=free_func;
}

/************************************************************************
************************************************************************/
void item_set_wheel_down(item_t * item,void (*wheel_down)(void * arg),void * wheel_down_arg, void (*free_func)(void *ptr))
{
	item->wheel_down=wheel_down;
	item->wheel_down_arg=wheel_down_arg;
	item->wheel_down_free=free_func;
}

/************************************************************************
************************************************************************/
void item_set_over(item_t * item,void (*over)(void * arg),void * over_arg, void (*free_func)(void *ptr))
{
	item->over=over;
	item->over_arg=over_arg;
	item->over_free=free_func;
}

/************************************************************************
************************************************************************/
void item_set_string(item_t * item,char * buf)
{
	item->string = strdup(buf);

	if(item->str_tex) {
		SDL_DestroyTexture(item->str_tex);
		item->str_tex = NULL;
	}
}

/************************************************************************
************************************************************************/
void item_set_string_bg(item_t * item,Uint32 color)
{
	item->string_bg = color;
}

/************************************************************************
************************************************************************/
void item_set_editable(item_t * item,int is_editable)
{
	item->editable = is_editable;
}

/************************************************************************
************************************************************************/
void item_set_edit_cb(item_t * item,void (*cb_edit)(void * arg))
{
	item->edit_cb = cb_edit;
}

/************************************************************************
************************************************************************/
void item_set_geometry(item_t * item,int x, int y, int w, int h)
{
	item->rect.x=x;
	item->rect.y=y;
	item->rect.w=w;
	item->rect.h=h;
}

/************************************************************************
************************************************************************/
void item_set_font(item_t * item, TTF_Font * font)
{
	item->font = font;
	if(item->str_tex) {
		SDL_DestroyTexture(item->str_tex);
		item->str_tex = NULL;
	}
}

