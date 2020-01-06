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

#include "const.h"
#include "sdl_item.h"
#include "sdl.h"
#include <SDL2/SDL.h>

/************************************************************************
 Return a pointer to the newly creates item_t
 ************************************************************************/
item_t * item_list_add(item_t ** item_list)
{
	item_t * item = nullptr;
	item_t * last_item = nullptr;

	item = (item_t*) malloc(sizeof(item_t));
	item_init(item);

	// add to item list
	if (*item_list != nullptr)
	{
		last_item = (*item_list)->last;
		last_item->next = item;
		(*item_list)->last = item;
	}
	else
	{
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
	if (item->anim.list)
	{
		free(item->anim.list);
	}
	if (item->default_anim_over.list)
	{
		free(item->default_anim_over.list);
	}
	if (item->default_anim_click.list)
	{
		free(item->default_anim_click.list);
	}
	if (item->str_tex)
	{
		SDL_DestroyTexture(item->str_tex);
	}
	if (item->click_left_free)
	{
		item->click_left_free(item->click_left_arg);
	}
	if (item->click_right_free)
	{
		item->click_right_free(item->click_right_arg);
	}
	if (item->double_click_left_free)
	{
		item->double_click_left_free(item->double_click_left_arg);
	}
	if (item->double_click_right_free)
	{
		item->double_click_right_free(item->double_click_right_arg);
	}
	if (item->wheel_up_free)
	{
		item->wheel_up_free(item->wheel_up_arg);
	}
	if (item->wheel_down_free)
	{
		item->wheel_down_free(item->wheel_down_arg);
	}
	if (item->over_free)
	{
		item->over_free(item->over_arg);
	}
	if (item->string)
	{
		free(item->string);
	}

	free(item);
}

/************************************************************************
 ************************************************************************/
void item_list_free(item_t * item_list)
{
	if (item_list == nullptr)
	{
		return;
	}

	item_list_free(item_list->next);

	item_free(item_list);
}

/************************************************************************
 ************************************************************************/
void item_init(item_t * item)
{
	item->rect.x = -1;
	item->rect.y = -1;
	item->rect.w = 0;
	item->rect.h = 0;
	item->angle = 0;
	item->zoom_x = 1.0;
	item->zoom_y = 1.0;
	item->flip = SDL_FLIP_NONE;
	item->anim_start_tick = 0;
	item->overlay = 0;
	item->anim.list = nullptr;
	item->anim.num = 0;
	item->anim_over.list = nullptr;
	item->anim_over.num = 0;
	item->default_anim_over.list = nullptr;
	item->default_anim_over.num = 0;
	item->anim_click.list = nullptr;
	item->anim_click.num = 0;
	item->default_anim_click.list = nullptr;
	item->default_anim_click.num = 0;
	item->layout = SiLayout::TOP_LEFT;
	item->anim_loop = TRUE;
	item->clicked = 0;
	item->click_left = nullptr;
	item->click_left_arg = nullptr;
	item->click_left_free = nullptr;
	item->click_right = nullptr;
	item->click_right_arg = nullptr;
	item->click_right_free = nullptr;
	item->double_click_left = nullptr;
	item->double_click_left_arg = nullptr;
	item->double_click_left_free = nullptr;
	item->double_click_right = nullptr;
	item->double_click_right_arg = nullptr;
	item->double_click_right_free = nullptr;
	item->wheel_up = nullptr;
	item->wheel_up_arg = nullptr;
	item->wheel_up_free = nullptr;
	item->wheel_down = nullptr;
	item->wheel_down_arg = nullptr;
	item->wheel_down_free = nullptr;
	item->over = nullptr;
	item->over_arg = nullptr;
	item->over_free = nullptr;
	item->string = nullptr;
	item->m_Buffer = nullptr;
	item->m_BufferSize = 0U;
	item->string_bg = 0; // Transparent black
	item->font = nullptr;
	item->str_tex = nullptr;
	item->editable = 0;
	item->edit_cb = nullptr;
	item->user_ptr = nullptr;
	item->user1_ptr = nullptr;
	item->next = nullptr;
	item->last = nullptr;
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
static void add_and_set_anim(anim_array_t * anim_array, anim_t * anim, int anim_index)
{
	int i;

	if (anim_array->num <= anim_index)
	{
		anim_array->list = (anim_t **) realloc(anim_array->list, (anim_index + 1) * sizeof(anim_t*));

		for (i = anim_array->num; i <= anim_index; i++)
		{
			anim_array->list[i] = nullptr;
		}
		anim_array->num = anim_index + 1;
	}

	anim_array->list[anim_index] = anim;
}

/************************************************************************
 ************************************************************************/
void item_set_anim(item_t * item, anim_t * anim, int anim_index)
{
	int w = 0;
	int h = 0;
	int max_w = 0;
	int max_h = 0;

	if (anim)
	{
		add_and_set_anim(&item->anim, anim, anim_index);
		max_w = anim->w;
		max_h = anim->h;
	}

	char * l_StringToDisplay = nullptr;

	if (item->string != nullptr)
	{
		l_StringToDisplay = item->string;
	}
	if (item->m_Buffer != nullptr)
	{
		l_StringToDisplay = item->m_Buffer;
	}

	if (l_StringToDisplay != nullptr)
	{
		sdl_get_string_size(item->font, l_StringToDisplay, &w, &h);
		if (w > max_w)
		{
			max_w = w;
		}
		if (h > max_h)
		{
			max_h = h;
		}
	}

	if (item->rect.w < max_w)
	{
		item->rect.w = max_w;
	}
	if (item->rect.h < max_h)
	{
		item->rect.h = max_h;
	}
}

/************************************************************************
 ************************************************************************/
void item_set_anim_shape(item_t * item, int x, int y, int w, int h)
{
	item_set_pos(item, x, y);
	item->rect.w = w;
	item->rect.h = h;
}

/************************************************************************
 anim_array is a nullptr terminated anim array
 ************************************************************************/
void item_set_anim_array(item_t * item, anim_t ** anim_array)
{
	int num_anim = 0;

	while (anim_array[num_anim])
	{
		item_set_anim(item, anim_array[num_anim], num_anim);
		num_anim++;
	}
}

/************************************************************************
 ************************************************************************/
void item_set_user(item_t * item, int user1, int user2)
{
	item->user1 = user1;
	item->user2 = user2;
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
void item_set_layout(item_t * item, SiLayout layout)
{
	item->layout = layout;
}

/************************************************************************
 ************************************************************************/
void item_set_anim_loop(item_t * item, int loop)
{
	item->anim_loop = loop;
}

/************************************************************************
 ************************************************************************/
void item_set_anim_over(item_t * item, anim_t * anim, int anim_index)
{
	add_and_set_anim(&item->default_anim_over, anim, anim_index);
}

/************************************************************************
 ************************************************************************/
void item_set_anim_over_array(item_t * item, anim_t ** anim_array)
{
	int num_anim = 0;

	if (anim_array == nullptr)
	{
		return;
	}

	while (anim_array[num_anim])
	{
		item_set_anim_over(item, anim_array[num_anim], num_anim);
		num_anim++;
	}
}

/************************************************************************
 ************************************************************************/
void item_set_anim_click(item_t * item, anim_t * anim, int anim_index)
{
	add_and_set_anim(&item->default_anim_click, anim, anim_index);
}

/************************************************************************
 ************************************************************************/
void item_set_anim_click_array(item_t * item, anim_t ** anim_array)
{
	int num_anim = 0;

	if (anim_array == nullptr)
	{
		return;
	}

	while (anim_array[num_anim])
	{
		item_set_anim_click(item, anim_array[num_anim], num_anim);
		num_anim++;
	}
}

/************************************************************************
 ************************************************************************/
void item_set_click_left(item_t * item, void (*click_left)(void * arg), void * click_left_arg, void (*free_func)(void *ptr))
{
	item->click_left = click_left;
	item->click_left_arg = click_left_arg;
	item->click_left_free = free_func;
}

/************************************************************************
 ************************************************************************/
void item_set_click_right(item_t * item, void (*click_right)(void * arg), void * click_right_arg, void (*free_func)(void *ptr))
{
	item->click_right = click_right;
	item->click_right_arg = click_right_arg;
	item->click_right_free = free_func;
}

/************************************************************************
 ************************************************************************/
void item_set_double_click_left(item_t * item, void (*double_click_left)(void * arg), void * double_click_left_arg, void (*free_func)(void *ptr))
{
	item->double_click_left = double_click_left;
	item->double_click_left_arg = double_click_left_arg;
	item->double_click_left_free = free_func;
}

/************************************************************************
 ************************************************************************/
void item_set_double_click_right(item_t * item, void (*double_click_right)(void * arg), void * double_click_right_arg, void (*free_func)(void *ptr))
{
	item->double_click_right = double_click_right;
	item->double_click_right_arg = double_click_right_arg;
	item->double_click_right_free = free_func;
}

/************************************************************************
 ************************************************************************/
void item_set_wheel_up(item_t * item, void (*wheel_up)(void * arg), void * wheel_up_arg, void (*free_func)(void *ptr))
{
	item->wheel_up = wheel_up;
	item->wheel_up_arg = wheel_up_arg;
	item->wheel_up_free = free_func;
}

/************************************************************************
 ************************************************************************/
void item_set_wheel_down(item_t * item, void (*wheel_down)(void * arg), void * wheel_down_arg, void (*free_func)(void *ptr))
{
	item->wheel_down = wheel_down;
	item->wheel_down_arg = wheel_down_arg;
	item->wheel_down_free = free_func;
}

/************************************************************************
 ************************************************************************/
void item_set_over(item_t * item, void (*over)(void * arg, int x, int y), void * over_arg, void (*free_func)(void *ptr))
{
	item->over = over;
	item->over_arg = over_arg;
	item->over_free = free_func;
}

/************************************************************************
 ************************************************************************/
void item_set_string(item_t * item, const char * buf)
{
	item->string = strdup(buf);

	if (item->str_tex)
	{
		SDL_DestroyTexture(item->str_tex);
		item->str_tex = nullptr;
	}
}

/************************************************************************
 ************************************************************************/
void item_set_buffer(item_t * item, char * buf, const size_t p_BufferSize)
{
	item->m_Buffer = buf;
	item->m_BufferSize = p_BufferSize;

	if (item->str_tex)
	{
		SDL_DestroyTexture(item->str_tex);
		item->str_tex = nullptr;
	}
}

/************************************************************************
 ************************************************************************/
void item_set_string_bg(item_t * item, Uint32 color)
{
	item->string_bg = color;
}

/************************************************************************
 ************************************************************************/
void item_set_editable(item_t * item, int is_editable)
{
	item->editable = is_editable;
}

/************************************************************************
 ************************************************************************/
void item_set_edit_cb(item_t * item, void (*cb_edit)(void * arg))
{
	item->edit_cb = cb_edit;
}

/************************************************************************
 ************************************************************************/
void item_set_geometry(item_t * item, int x, int y, int w, int h)
{
	item->rect.x = x;
	item->rect.y = y;
	item->rect.w = w;
	item->rect.h = h;
}

/************************************************************************
 ************************************************************************/
void item_set_font(item_t * item, TTF_Font * font)
{
	item->font = font;
	if (item->str_tex)
	{
		SDL_DestroyTexture(item->str_tex);
		item->str_tex = nullptr;
	}
}

/************************************************************************
 ************************************************************************/
void item_set_anim_start_tick(item_t * item, Uint32 tick)
{
	item->anim_start_tick = tick;
}
