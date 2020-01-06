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
#include "sdl.h"
#include <assert.h>
#include <math.h>

static int fullscreen = 0;

static char *keyboard_text_buf = nullptr;
static unsigned int keyboard_text_index = 0U;
static unsigned int keyboard_text_index_max = 0U;
static void (*keyboard_text_cb)(void * arg) = nullptr;

static int virtual_x = 0;
static int virtual_y = 0;
static double virtual_z = 1.0;
static int old_vx = 0;
static int old_vy = 0;
static double old_vz = 1.0;
static int current_vx = 0;
static int current_vy = 0;
static double current_vz = 1.0;
static Uint32 virtual_tick = 0;
static int mouse_x = 0;
static int mouse_y = 0;
static int mouse_in = 1;
static Uint32 global_time;

static keycb_t * key_callback = nullptr;
static mousecb_t * mouse_callback = nullptr;

static SDL_Window * window = nullptr;
static SDL_Renderer * renderer = nullptr;

/*****************************************************************************/
SDL_Renderer * sdl_get_renderer()
{
	return renderer;
}

/************************************************************************
 ************************************************************************/
//You must SDL_LockSurface(surface); then SDL_UnlockSurface(surface); before calling this function
void sdl_set_pixel(SDL_Surface *surface, int x, int y, Uint32 R, Uint32 G, Uint32 B, Uint32 A)
{
	Uint32 color = (A << 24) + (R << 16) + (G << 8) + (B);
	Uint8 *target_pixel = (Uint8 *) surface->pixels + y * surface->pitch + x * surface->format->BytesPerPixel;
	*(Uint32 *) target_pixel = color;
}

/************************************************************************
 ************************************************************************/
//You must SDL_LockSurface(surface); then SDL_UnlockSurface(surface); before calling this function
Uint32 sdl_get_pixel(SDL_Surface *surface, int x, int y)
{
	Uint8 *target_pixel = (Uint8 *) surface->pixels + y * surface->pitch + x * surface->format->BytesPerPixel;
	return *(Uint32 *) target_pixel;
}

/************************************************************************
 ************************************************************************/
void sdl_cleanup()
{
	SDL_Quit();
}

/************************************************************************
 ************************************************************************/
void sdl_init(const std::string & title, int vsync)
{
	if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
	{
		exit(EXIT_FAILURE);
	}

	if (TTF_Init() == -1)
	{
		exit(EXIT_FAILURE);
	}

	atexit(sdl_cleanup);

	window = SDL_CreateWindow(title.c_str(),
	SDL_WINDOWPOS_UNDEFINED,
	SDL_WINDOWPOS_UNDEFINED,
	DEFAULT_SCREEN_W, DEFAULT_SCREEN_H, SDL_WINDOW_RESIZABLE);
	if (window == nullptr)
	{
		exit(EXIT_FAILURE);
	}

	renderer = SDL_CreateRenderer(window, -1, vsync ? SDL_RENDERER_PRESENTVSYNC : 0);
	if (renderer == nullptr)
	{
		exit(EXIT_FAILURE);
	}

	SDL_RenderSetLogicalSize(renderer, DEFAULT_SCREEN_W, DEFAULT_SCREEN_H);
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
}

/************************************************************************
 ************************************************************************/
static void get_virtual(SDL_Renderer * p_pRender, int & p_rVx, int & p_rVy, int & p_rWidth, int & p_rHeight)
{
	SDL_GetRendererOutputSize(p_pRender, &p_rWidth, &p_rHeight);

	p_rVx = (p_rWidth / current_vz / 2) - current_vx;
	p_rVy = (p_rHeight / current_vz / 2) - current_vy;
}

/************************************************************************
 ************************************************************************/
void sdl_mouse_position_manager(item_t * item_list)
{
	item_t * I = nullptr;
	int mx = 0;
	int my = 0;
	int vx = 0;
	int vy = 0;
	int l_Width = 0;
	int l_Height = 0;
	int zoomed_x = 0;
	int zoomed_y = 0;
	int zoomed_w = 0;
	int zoomed_h = 0;

	if (!mouse_in)
	{
		return;
	}

	I = item_list;
	while (I)
	{
		if (I->overlay)
		{
			mx = mouse_x;
			my = mouse_y;
			zoomed_x = I->rect.x;
			zoomed_y = I->rect.y;
			zoomed_w = I->rect.w;
			zoomed_h = I->rect.h;
		}
		else
		{
			get_virtual(renderer, vx, vy, l_Width, l_Height);
			mx = mouse_x - (vx * current_vz);
			my = mouse_y - (vy * current_vz);
			zoomed_x = I->rect.x * current_vz;
			zoomed_y = I->rect.y * current_vz;
			zoomed_w = I->rect.w * current_vz;
			zoomed_h = I->rect.h * current_vz;
		}

		I->anim_over.list = nullptr;
		I->anim_over.num = 0;
		I->anim_click.list = nullptr;
		I->anim_click.num = 0;

		if ((zoomed_x <= mx) && ((zoomed_x + zoomed_w) > mx) && (zoomed_y <= my) && ((zoomed_y + zoomed_h) > my))
		{
			I->anim_over.list = I->default_anim_over.list;
			I->anim_over.num = I->default_anim_over.num;
			/* Display clicked anim */
			if (SDL_GetMouseState(nullptr, nullptr))
			{
				I->anim_click.list = I->default_anim_click.list;
				I->anim_click.num = I->default_anim_click.num;
			}
		}

		I = I->next;
	}
}

/************************************************************************
 ************************************************************************/
bool sdl_mouse_manager(SDL_Event * event, item_t * item_list)
{
	int mx = 0;
	int my = 0;
	int vx = 0;
	int vy = 0;
	int l_Width = 0;
	int l_Height = 0;
	int zoomed_x = 0;
	int zoomed_y = 0;
	int zoomed_w = 0;
	int zoomed_h = 0;
	item_t * I = nullptr;
	int overlay_first = 1;
	int skip_non_overlay = FALSE;
	static Uint32 timestamp = 0;
	int action_done = FALSE;
	mousecb_t * mousecb = mouse_callback;

	if (event->type == SDL_WINDOWEVENT)
	{
		switch (event->window.event)
		{
		case SDL_WINDOWEVENT_ENTER:
			mouse_in = 1;
			break;
		case SDL_WINDOWEVENT_LEAVE:
			mouse_in = 0;
			break;
		default:
			break;
		}
	}

	if (mouse_in == 0)
	{
		return false;;
	}

	if (item_list == nullptr)
	{
		return false;
	}

	if (event->type == SDL_MOUSEMOTION)
	{
		mouse_x = event->motion.x;
		mouse_y = event->motion.y;
	}

	get_virtual(renderer, vx, vy, l_Width, l_Height);

	// First test overlay (UI) before background
	while (overlay_first != -1)
	{
		I = item_list;
		while (I)
		{
			if (I->overlay)
			{
				if (!overlay_first)
				{
					I = I->next;
					continue;
				}
				mx = mouse_x;
				my = mouse_y;
				zoomed_x = I->rect.x;
				zoomed_y = I->rect.y;
				zoomed_w = I->rect.w;
				zoomed_h = I->rect.h;
			}
			else
			{
				if (overlay_first)
				{
					I = I->next;
					continue;
				}
				mx = mouse_x - (vx * current_vz);
				my = mouse_y - (vy * current_vz);
				zoomed_x = I->rect.x * current_vz;
				zoomed_y = I->rect.y * current_vz;
				zoomed_w = I->rect.w * current_vz;
				zoomed_h = I->rect.h * current_vz;
			}

			// Manage event related to mouse position
			if ((zoomed_x <= mx) && ((zoomed_x + zoomed_w) > mx) && (zoomed_y <= my) && ((zoomed_y + zoomed_h) > my))
			{
				// We are on overlay item: skip, non-overlay item
				if (overlay_first)
				{
					skip_non_overlay = TRUE;
				}

				switch (event->type)
				{
				case SDL_MOUSEMOTION:
					if (I->over)
					{
						/* x,y is the mouse pointer position relative to the item itself.
						 i.e. 0,0 is the mouse pointer is in the upper-left corner of the item */
						I->over(I->over_arg, mx - I->rect.x, my - I->rect.y);
					}
					action_done = TRUE;
					break;
				case SDL_MOUSEBUTTONDOWN:
					sdl_keyboard_text_reset();
					if (I->editable)
					{
						sdl_keyboard_text_init(I->m_Buffer, I->m_BufferSize, I->edit_cb);
					}

					if (I->click_left && event->button.button == SDL_BUTTON_LEFT)
					{
						I->clicked = 1;
					}
					if (I->click_right && event->button.button == SDL_BUTTON_RIGHT)
					{
						I->clicked = 1;
					}
					action_done = TRUE;
					break;
				case SDL_MOUSEBUTTONUP:
					I->clicked = 0;
					if (I->click_left && event->button.button == SDL_BUTTON_LEFT && event->button.clicks == 1)
					{
						I->click_left(I->click_left_arg);
					}
					if (I->click_right && event->button.button == SDL_BUTTON_RIGHT && event->button.clicks == 1)
					{
						I->click_right(I->click_right_arg);
					}
					if (I->double_click_left && event->button.button == SDL_BUTTON_LEFT && event->button.clicks == 2)
					{
						I->double_click_left(I->double_click_left_arg);
					}
					if (I->double_click_right && event->button.button == SDL_BUTTON_RIGHT && event->button.clicks == 2)
					{
						I->double_click_right(I->double_click_right_arg);
					}
					action_done = TRUE;
					break;
				case SDL_MOUSEWHEEL:
					if (event->wheel.timestamp != timestamp)
					{
						if (event->wheel.y > 0 && I->wheel_up)
						{
							timestamp = event->wheel.timestamp;
							I->wheel_up(I->wheel_up_arg);
							action_done = TRUE;
						}
						if (event->wheel.y < 0 && I->wheel_down)
						{
							timestamp = event->wheel.timestamp;
							I->wheel_down(I->wheel_down_arg);
							action_done = TRUE;
						}
					}
					break;
				}
			}

			if (I->clicked)
			{
			}
			I = I->next;
		}
		overlay_first--;
		if (skip_non_overlay == TRUE)
		{
			break;
		}
	}

	if (action_done == TRUE)
	{
		return true;
	}

	while (mousecb)
	{
		switch (event->type)
		{
		case SDL_MOUSEMOTION:
			if (mousecb->event_type != MOUSE_MOTION)
			{
				mousecb = mousecb->next;
				continue;
			}
			mousecb->cb(mouse_x, mouse_y);
			break;
		case SDL_MOUSEBUTTONDOWN:
			if (mousecb->event_type != MOUSE_BUTTON_DOWN)
			{
				mousecb = mousecb->next;
				continue;
			}
			mousecb->cb(event->button.button, 0);
			break;
		case SDL_MOUSEBUTTONUP:
			if (mousecb->event_type != MOUSE_BUTTON_UP)
			{
				mousecb = mousecb->next;
				continue;
			}
			mousecb->cb(event->button.button, 0);
			break;
		case SDL_MOUSEWHEEL:
			if (mousecb->event_type != MOUSE_WHEEL_UP && mousecb->event_type != MOUSE_WHEEL_DOWN)
			{
				mousecb = mousecb->next;
				continue;
			}
			if (event->wheel.timestamp != timestamp)
			{
				if (event->wheel.y > 0 && mousecb->event_type == MOUSE_WHEEL_UP)
				{
					mousecb->cb(event->wheel.y, 0);
				}
				if (event->wheel.y < 0 && mousecb->event_type == MOUSE_WHEEL_DOWN)
				{
					mousecb->cb(event->wheel.y, 0);
				}
			}
			break;
		}
		mousecb = mousecb->next;
	}

	return false;
}

/************************************************************************
 Take care of system's windowing event
 return 1 if caller need to redraw it's screen
 ************************************************************************/
int sdl_screen_manager(SDL_Event * event)
{
	const Uint8 *keystate = nullptr;

	switch (event->type)
	{
	case SDL_WINDOWEVENT:
		switch (event->window.event)
		{
		case SDL_WINDOWEVENT_RESIZED:
			SDL_RenderSetLogicalSize(renderer, event->window.data1, event->window.data2);
			return 1;
		}
		break;
	case SDL_KEYDOWN:
		switch (event->key.keysym.sym)
		{
		case SDLK_RETURN:
			keystate = SDL_GetKeyboardState(nullptr);

			if (keystate[SDL_SCANCODE_RALT] || keystate[SDL_SCANCODE_LALT])
			{
				if (!fullscreen)
				{
					fullscreen = SDL_WINDOW_FULLSCREEN_DESKTOP;
				}
				else
				{
					fullscreen = 0;
				}
				SDL_SetWindowFullscreen(window, fullscreen);
				break;
			}
			break;
		default:
			break;
		}
		break;
	case SDL_QUIT:
		exit(EXIT_SUCCESS);
		break;
	default:
		break;
	}

	return 0;
}

/************************************************************************
 ************************************************************************/
void sdl_loop_manager()
{
	static Uint32 old_timer = 0;

	if (old_timer == 0)
	{
		old_timer = SDL_GetTicks();
	}

	global_time = SDL_GetTicks();
#if 0
	if( timer < old_timer + FRAME_DELAY )
	{
		SDL_Delay(old_timer + FRAME_DELAY - timer);
	}
#endif

	if (virtual_tick + VIRTUAL_CAMERA_ANIM_DURATION > global_time)
	{
		current_vx = (int) ((double) old_vx + (double) (virtual_x - old_vx) * (double) (global_time - virtual_tick) / (double) VIRTUAL_CAMERA_ANIM_DURATION);
		current_vy = (int) ((double) old_vy + (double) (virtual_y - old_vy) * (double) (global_time - virtual_tick) / (double) VIRTUAL_CAMERA_ANIM_DURATION);
		current_vz = (double) old_vz + (double) (virtual_z - old_vz) * (double) (global_time - virtual_tick) / (double) VIRTUAL_CAMERA_ANIM_DURATION;
	}
	else
	{
		old_vx = virtual_x;
		current_vx = virtual_x;

		old_vy = virtual_y;
		current_vy = virtual_y;

		old_vz = virtual_z;
		current_vz = virtual_z;
	}
}

/************************************************************************
 flip is one of SDL_FLIP_NONE, SDL_FLIP_HORIZONTAL, SDL_FLIP_VERTICAL
 ************************************************************************/
void sdl_blit_tex(SDL_Texture * tex, SDL_Rect * rect, double angle, double zoom_x, double zoom_y, int flip, int overlay)
{
	SDL_Rect r =
	{ 0, 0, 0, 0 };
	int vx = 0;
	int vy = 0;
	int l_Width = 0;
	int l_Height = 0;

	if (tex == nullptr)
	{
		return;
	}

	get_virtual(renderer, vx, vy, l_Width, l_Height);

	if (overlay == 1)
	{
		r.x = rect->x;
		r.y = rect->y;
	}
	else
	{
		r.x = rect->x + vx;
		r.y = rect->y + vy;
	}

	r.w = rect->w;
	r.h = rect->h;

	// Sprite zoom
	r.w *= zoom_x;
	r.h *= zoom_y;

	if (overlay == 0)
	{
		// Virtual zoom
		r.x = ceil((double) r.x * current_vz);
		r.y = ceil((double) r.y * current_vz);
		r.w = ceil((double) r.w * current_vz);
		r.h = ceil((double) r.h * current_vz);
	}

	// Crop
	if ((r.x > l_Width) || ((r.x + r.w) < 0) || (r.y > l_Height) || ((r.y + r.h) < 0))
	{
		return;
	}

	if (tex != nullptr)
	{
		if (SDL_RenderCopyEx(renderer, tex, nullptr, &r, angle, nullptr, (SDL_RendererFlip) flip) < 0)
		{
			//Error
		}
	}
}

/************************************************************************
 ************************************************************************/
int get_current_frame(anim_t * anim, int loop, Uint32 anim_start_tick)
{
	int current_frame = 0;

	if (anim->total_duration != 0)
	{
		if (loop == FALSE)
		{
			if (anim_start_tick + anim->total_duration < global_time)
			{
				current_frame = anim->num_frame - 1;
			}
			else
			{
				Uint32 tick = anim_start_tick;
				int i;
				for (i = 0; i < anim->num_frame; i++)
				{
					if (tick + anim->delay[i] > global_time)
					{
						current_frame = i;
						break;
					}
					tick += anim->delay[i];
				}
			}
		}
		// Loop animation
		else
		{
			Uint32 tick = (global_time - anim_start_tick) % anim->total_duration;
			Uint32 current_delay = 0;
			int i;
			for (i = 0; i < anim->num_frame; i++)
			{
				current_delay += anim->delay[i];
				if (tick < current_delay)
				{
					current_frame = i;
					break;
				}
			}
		}
	}

	return current_frame;
}
/*******************************
 return 0 if blit OK
 return -1 if blit NOK
 *******************************/
int sdl_blit_anim(anim_t * anim, SDL_Rect * rect, double angle, double zoom_x, double zoom_y, int flip, int loop, int overlay, Uint32 anim_start_tick)
{
	if (anim == nullptr)
	{
		return -1;
	}

	if (anim->tex == nullptr)
	{
		return -1;
	}

	int current_frame = get_current_frame(anim, loop, anim_start_tick);

	sdl_blit_tex(anim->tex[current_frame], rect, angle, zoom_x, zoom_y, flip, overlay);

	return 0;
}

/************************************************************************
 ************************************************************************/
void sdl_get_string_size(TTF_Font * font, const char * string, int * w, int *h)
{
	SDL_Rect r =
	{ 0, 0, 0, 0 };

	TTF_SizeText(font, string, &r.w, &r.h);
	*w = r.w;
	*h = r.h;
}

/************************************************************************
 ************************************************************************/
void sdl_print_item(item_t * item)
{
	SDL_Surface * surf = nullptr;
	SDL_Color fg =
	{ 0xff, 0xff, 0xff };
	SDL_Rect rect =
	{ 0, 0, 0, 0 };
	anim_t * bg_anim = nullptr;
	int l_TextWidth = 0;
	int l_TextHeight = 0;
	int l_BackgroundWidth = item->rect.w;
	int l_BackgroundHeight = item->rect.h;
	char * l_StringToDisplay = nullptr;

	if (item->string != nullptr)
	{
		l_StringToDisplay = item->string;
	}
	if (item->m_Buffer != nullptr)
	{
		l_StringToDisplay = item->m_Buffer;
	}

	TTF_SizeText(item->font, l_StringToDisplay, &l_TextWidth, &l_TextHeight);
	if (l_BackgroundWidth < l_TextWidth)
	{
		l_BackgroundWidth = l_TextWidth;
	}

	if (l_BackgroundHeight < l_TextHeight)
	{
		l_BackgroundHeight = l_TextHeight;
	}

	rect.x = item->rect.x;
	rect.y = item->rect.y;

	if (item->string_bg != 0)
	{
		rect.w = l_BackgroundWidth;
		rect.h = l_BackgroundHeight;
		bg_anim = anim_create_color(rect.w, rect.h, item->string_bg);
		sdl_blit_anim(bg_anim, &rect, item->angle, item->zoom_x, item->zoom_y, item->flip, 0, item->overlay, 0);
		si_anim_free(bg_anim);
	}

	if (item->str_tex == nullptr)
	{
		surf = TTF_RenderText_Blended(item->font, l_StringToDisplay, fg);
		item->str_tex = SDL_CreateTextureFromSurface(renderer, surf);
		SDL_FreeSurface(surf);
	}

	rect.w = l_TextWidth;
	rect.h = l_TextHeight;
	sdl_blit_tex(item->str_tex, &rect, item->angle, item->zoom_x, item->zoom_y, item->flip, item->overlay);
}

/************************************************************************
 ************************************************************************/
int sdl_blit_item(item_t * item)
{
	SDL_Rect rect =
	{ 0, 0, 0, 0 };
	anim_array_t * anim_array = nullptr;
	int i = 0;

	if (item->anim.list && item->anim.list[0])
	{
		anim_array = &item->anim;
	}
	if (item->anim_click.list && item->anim_click.list[0])
	{
		anim_array = &item->anim_click;
	}
	if (item->anim_over.list && item->anim_over.list[0])
	{
		anim_array = &item->anim_over;
	}

	if (anim_array)
	{
		int max_width = 0;
		int max_height = 0;

		if (item->layout == SiLayout::CENTER)
		{
			for (i = 0; i < anim_array->num; i++)
			{
				if (anim_array->list[i]->w > max_width)
				{
					max_width = anim_array->list[i]->w;
				}
				if (anim_array->list[i]->h > max_height)
				{
					max_height = anim_array->list[i]->h;
				}
			}
		}

		for (i = 0; i < anim_array->num; i++)
		{
			rect.x = item->rect.x;
			rect.y = item->rect.y;
			switch (item->layout)
			{
			case SiLayout::CENTER:
				rect.x = item->rect.x + ((max_width - anim_array->list[i]->w) / 2);
				rect.y = item->rect.y + ((max_height - anim_array->list[i]->h) / 2);
				break;

			case SiLayout::TOP_LEFT:
			default:
				rect.x = item->rect.x;
				rect.y = item->rect.y;
				break;
			}
			rect.w = anim_array->list[i]->w;
			rect.h = anim_array->list[i]->h;
			sdl_blit_anim(anim_array->list[i], &rect, item->angle, item->zoom_x, item->zoom_y, item->flip, item->anim_loop, item->overlay,
					item->anim_start_tick);
		}
	}

	if (item->font != nullptr && (item->string != nullptr || item->m_Buffer != nullptr))
	{
		sdl_print_item(item);
	}

	return 0;
}

/************************************************************************
 ************************************************************************/
void sdl_blit_item_list(item_t * list)
{
	item_t * item = nullptr;

	item = list;
	while (item != nullptr)
	{
		sdl_blit_item(item);
		item = item->next;
	}
}

/************************************************************************
 ************************************************************************/
void sdl_keyboard_text_init(char * buf, const size_t p_BufferSize, void (*cb)(void*arg))
{
	if (buf == nullptr)
	{
		return;
	}

	keyboard_text_index = 0U;
	keyboard_text_index_max = p_BufferSize;
	keyboard_text_buf = buf;
	keyboard_text_cb = cb;
}

/************************************************************************
 ************************************************************************/
void sdl_keyboard_text_reset()
{
	keyboard_text_index = 0U;
	keyboard_text_index_max = 0U;
	keyboard_text_buf = nullptr;
	keyboard_text_cb = nullptr;
}

/************************************************************************
 ************************************************************************/
char * sdl_keyboard_text_get_buf()
{
	return keyboard_text_buf;
}

/************************************************************************
 ************************************************************************/
bool sdl_keyboard_manager(SDL_Event * event)
{
	const Uint8 *keystate = nullptr;
	keycb_t * key = nullptr;

	switch (event->type)
	{
	case SDL_KEYUP:
		key = key_callback;
		if (key)
		{
			do
			{
				if (event->key.keysym.scancode == key->code)
				{
					if (key->cb_up)
					{
						key->cb_up(key->arg);
					}
				}
				key = key->next;
			} while (key);
		}
		break;
	case SDL_KEYDOWN:
		// If no keyboard_text ready, key are used for UI
		if (keyboard_text_buf == nullptr)
		{
			key = key_callback;
			if (key)
			{
				do
				{
					if (event->key.keysym.scancode == key->code)
					{
						key->cb(key->arg);
					}
					key = key->next;
				} while (key);
			}
			break;
		}

		// Else keys are used to enter text
		if (event->key.keysym.sym == SDLK_RETURN)
		{
			if (keyboard_text_cb)
			{
				keyboard_text_cb(keyboard_text_buf);
			}
		}

		if (event->key.keysym.sym == SDLK_DELETE || event->key.keysym.sym == SDLK_BACKSPACE)
		{
			if (keyboard_text_index > 0)
			{
				keyboard_text_index--;
			}
			keyboard_text_buf[keyboard_text_index] = 0;
		}

		if (event->key.keysym.sym >= SDLK_SPACE && event->key.keysym.sym < SDLK_DELETE)
		{
			// Upper case
			keystate = SDL_GetKeyboardState(nullptr);
			if ((keystate[SDL_SCANCODE_RSHIFT] || keystate[SDL_SCANCODE_LSHIFT]) && (event->key.keysym.sym >= SDLK_a && event->key.keysym.sym <= SDLK_z))
			{
				event->key.keysym.sym = (SDL_Scancode) (event->key.keysym.sym - 32);
			}
			keyboard_text_buf[keyboard_text_index] = event->key.keysym.sym;
			keyboard_text_index++;

			if (keyboard_text_index >= keyboard_text_index_max)
			{
				keyboard_text_index--;
			}
			keyboard_text_buf[keyboard_text_index] = 0;
		}
		return true;
		break;
	default:
		break;
	}

	return false;
}

/************************************************************************
 ************************************************************************/
void sdl_blit_to_screen()
{
	SDL_RenderPresent(renderer);
}

/************************************************************************
 ************************************************************************/
void sdl_set_virtual_x(int x)
{
	old_vx = current_vx;
	if (x != virtual_x)
	{
		virtual_x = x;
		virtual_tick = global_time;
	}
}

/************************************************************************
 ************************************************************************/
void sdl_set_virtual_y(int y)
{
	old_vy = current_vy;
	if (y != virtual_y)
	{
		virtual_y = y;
		virtual_tick = global_time;
	}
}

/************************************************************************
 ************************************************************************/
void sdl_set_virtual_z(double z)
{
	if (z != virtual_z)
	{
		old_vz = current_vz;
		virtual_z = z;
		virtual_tick = global_time;
	}
}

/************************************************************************
 ************************************************************************/
int sdl_get_virtual_x()
{
	return virtual_x;
}

/************************************************************************
 ************************************************************************/
int sdl_get_virtual_y()
{
	return virtual_y;
}

/************************************************************************
 ************************************************************************/
double sdl_get_virtual_z()
{
	return virtual_z;
}

/************************************************************************
 ************************************************************************/
void sdl_force_virtual_x(int x)
{
	virtual_x = x;
	current_vx = x;
	old_vx = x;
}

/************************************************************************
 ************************************************************************/
void sdl_force_virtual_y(int y)
{
	virtual_y = y;
	current_vy = y;
	old_vy = y;
}

/************************************************************************
 ************************************************************************/
void sdl_force_virtual_z(double z)
{
	virtual_z = z;
	current_vz = z;
	old_vz = z;
}

/************************************************************************
 ************************************************************************/
void sdl_add_keycb(SDL_Scancode code, void (*cb)(void*), void (*cb_up)(void*), void * arg)
{
	keycb_t * key = nullptr;

	if (key_callback == nullptr)
	{
		key = (keycb_t *) malloc(sizeof(keycb_t));
		key_callback = key;
		key->code = code;
		key->cb = cb;
		key->cb_up = cb_up;
		key->arg = arg;
		key->next = nullptr;
		return;
	}
	else
	{
		key = key_callback;
		while (key->next != nullptr)
		{
			key = key->next;
		}
		key->next = (keycb_t *) malloc(sizeof(keycb_t));
		key = key->next;
		key->code = code;
		key->cb = cb;
		key->cb_up = cb_up;
		key->arg = arg;
		key->next = nullptr;
		return;
	}
}

/************************************************************************
 ************************************************************************/
static void rec_free_keycb(keycb_t * key)
{
	if (key == nullptr)
	{
		return;
	}

	if (key->next)
	{
		rec_free_keycb(key->next);
	}

	free(key);
}

/************************************************************************
 ************************************************************************/
void sdl_free_keycb()
{
	rec_free_keycb(key_callback);

	key_callback = nullptr;
}

/************************************************************************
 ************************************************************************/
void sdl_add_mousecb(Uint32 event_type, void (*cb)(Uint32, Uint32))
{
	mousecb_t * mouse = nullptr;

	if (mouse_callback == nullptr)
	{
		mouse = (mousecb_t *) malloc(sizeof(mousecb_t));
		mouse_callback = mouse;
		mouse->event_type = event_type;
		mouse->cb = cb;
		mouse->next = nullptr;
		return;
	}
	else
	{
		mouse = mouse_callback;
		while (mouse->next != nullptr)
		{
			mouse = mouse->next;
		}
		mouse->next = (mousecb_t *) malloc(sizeof(mousecb_t));
		mouse = mouse->next;
		mouse->event_type = event_type;
		mouse->cb = cb;
		mouse->next = nullptr;
		return;
	}
}

/************************************************************************
 ************************************************************************/
static void rec_free_mousecb(mousecb_t * mouse)
{
	if (mouse == nullptr)
	{
		return;
	}

	if (mouse->next)
	{
		rec_free_mousecb(mouse->next);
	}

	free(mouse);
}

/************************************************************************
 ************************************************************************/
void sdl_free_mousecb()
{
	rec_free_mousecb(mouse_callback);

	mouse_callback = nullptr;
}

/************************************************************************
 ************************************************************************/
Uint32 sdl_get_global_time()
{
	return global_time;
}

/*****************************************************************************/
anim_t * sdl_get_minimal_anim()
{
	anim_t * def_anim = (anim_t*) malloc(sizeof(anim_t));

	def_anim->num_frame = 1;
	def_anim->tex = (SDL_Texture**) malloc(sizeof(SDL_Texture*));
	def_anim->tex[0] = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STATIC, 1, 1);
	def_anim->w = 1;
	def_anim->h = 1;
	def_anim->delay = (Uint32*) malloc(sizeof(Uint32));

	def_anim->delay[0] = 0;
	def_anim->total_duration = 0;

	return def_anim;
}

/*****************************************************************************/
void sdl_set_background_color(int R, int G, int B, int A)
{
	SDL_SetRenderDrawColor(renderer, R, G, B, A);
}

/*****************************************************************************/
void sdl_get_output_size(int * width, int * height)
{
	SDL_GetRendererOutputSize(renderer, width, height);
}

/*****************************************************************************/
void sdl_clear()
{
	SDL_RenderClear(renderer);
}

