/*
   sdl_item is a graphical library based on SDL.
   Copyright (C) 2013-2015 carabobz@gmail.com

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

#include "sdl.h"
#include <math.h>
#include <assert.h>
#include "const.h"

#ifdef __cplusplus
extern "C" {
#endif

static int fullscreen = 0;

static char *keyboard_text_buf = NULL;
static unsigned int keyboard_text_index = 0;
//static unsigned int keyboard_text_index_max = 0;
static void (*keyboard_text_cb)(void * arg) = NULL;

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

static keycb_t * key_callback = NULL;
static mousecb_t * mouse_callback = NULL;

static void (*screen_compose)(void) = NULL;

/************************************************************************
************************************************************************/
//You must SDL_LockSurface(surface); then SDL_UnlockSurface(surface); before calling this function
void sdl_set_pixel(SDL_Surface *surface, int x, int y, Uint32 R, Uint32 G, Uint32 B, Uint32 A)
{
	Uint32 color = (A << 24) + (R << 16) + (G << 8) + (B);
	Uint8 *target_pixel = (Uint8 *)surface->pixels + y * surface->pitch + x * surface->format->BytesPerPixel;
	*(Uint32 *)target_pixel = color;
}

/************************************************************************
************************************************************************/
//You must SDL_LockSurface(surface); then SDL_UnlockSurface(surface); before calling this function
Uint32 sdl_get_pixel(SDL_Surface *surface, int x, int y)
{
	Uint8 *target_pixel = (Uint8 *)surface->pixels + y * surface->pitch + x * surface->format->BytesPerPixel;
	return *(Uint32 *)target_pixel;
}

/************************************************************************
************************************************************************/
void sdl_cleanup()
{
	SDL_Quit();
}

/************************************************************************
************************************************************************/
void sdl_init(const char * title, SDL_Renderer ** render,SDL_Window ** window, void (*screen_compose_cb)(void), int vsync)
{
	if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
		exit(EXIT_FAILURE);
	}

	if (TTF_Init() == -1) {
		exit(EXIT_FAILURE);
	}

	atexit(sdl_cleanup);

	*window = SDL_CreateWindow(title,
							   SDL_WINDOWPOS_UNDEFINED,
							   SDL_WINDOWPOS_UNDEFINED,
							   DEFAULT_SCREEN_W, DEFAULT_SCREEN_H,
							   SDL_WINDOW_RESIZABLE);
	if( *window == NULL) {
		exit(EXIT_FAILURE);
	}

	*render = SDL_CreateRenderer(*window, -1, vsync?SDL_RENDERER_PRESENTVSYNC:0);
	if( *render == NULL) {
		exit(EXIT_FAILURE);
	}

	SDL_RenderSetLogicalSize(*render,DEFAULT_SCREEN_W,DEFAULT_SCREEN_H);
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");

	screen_compose = screen_compose_cb;
}

/************************************************************************
************************************************************************/
static void get_virtual(SDL_Renderer * render,int * vx, int * vy)
{
	int sx;
	int sy;

	SDL_GetRendererOutputSize(render,&sx,&sy);

	sx /= current_vz;
	sy /= current_vz;

	*vx = (sx/2)-current_vx;
	*vy = (sy/2)-current_vy;
}

/************************************************************************
************************************************************************/
void sdl_mouse_position_manager(SDL_Renderer * render, item_t * item_list)
{
	item_t * I = NULL;
	int mx;
	int my;
	int vx = 0;
	int vy = 0;
	int zoomed_x;
	int zoomed_y;
	int zoomed_w;
	int zoomed_h;

	if( ! mouse_in ) {
		return;
	}

	I = item_list;
	while(I) {
		if(I->overlay) {
			mx = mouse_x;
			my = mouse_y;
			zoomed_x = I->rect.x;
			zoomed_y = I->rect.y;
			zoomed_w = I->rect.w;
			zoomed_h = I->rect.h;
		} else {
			get_virtual(render,&vx,&vy);
			mx = mouse_x - (vx * current_vz);
			my = mouse_y - (vy * current_vz);
			zoomed_x = I->rect.x * current_vz;
			zoomed_y = I->rect.y * current_vz;
			zoomed_w = I->rect.w * current_vz;
			zoomed_h = I->rect.h * current_vz;
		}

		I->anim_over.list=NULL;
		I->anim_over.num=0;
		I->anim_click.list=NULL;
		I->anim_click.num=0;

		if( (zoomed_x <= mx) &&
				((zoomed_x+zoomed_w) > mx) &&
				(zoomed_y <= my) &&
				((zoomed_y+zoomed_h) > my) ) {
			I->anim_over.list = I->default_anim_over.list;
			I->anim_over.num = I->default_anim_over.num;
			/* Display clicked anim */
			if( SDL_GetMouseState(NULL,NULL) ) {
				I->anim_click.list=I->default_anim_click.list;
				I->anim_click.num=I->default_anim_click.num;
			}
		}


		I = I->next;
	}
}

/************************************************************************
************************************************************************/
void sdl_mouse_manager(SDL_Renderer * render, SDL_Event * event, item_t * item_list)
{
	int mx;
	int my;
	int vx = 0;
	int vy = 0;
	int zoomed_x;
	int zoomed_y;
	int zoomed_w;
	int zoomed_h;
	item_t * I = NULL;
	int overlay_first = 1;
	int skip_non_overlay = FALSE;
	static int timestamp = 0;
	int action_done = FALSE;
	mousecb_t * mousecb = mouse_callback;

	if (event->type == SDL_WINDOWEVENT) {
		switch (event->window.event) {
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

	if( ! mouse_in ) {
		return;
	}

	if(item_list == NULL) {
		return;
	}

	if ( event->type == SDL_MOUSEMOTION) {
		mouse_x = event->motion.x;
		mouse_y = event->motion.y;
	}

	/* First test overlay (UI) before background */
	while(overlay_first!=-1) {
		I = item_list;
		while(I) {
			if(I->overlay) {
				if(!overlay_first) {
					I = I->next;
					continue;
				}
				mx = mouse_x;
				my = mouse_y;
				zoomed_x = I->rect.x;
				zoomed_y = I->rect.y;
				zoomed_w = I->rect.w;
				zoomed_h = I->rect.h;
			} else {
				if(overlay_first) {
					I = I->next;
					continue;
				}
				get_virtual(render,&vx,&vy);
				mx = mouse_x - (vx * current_vz) ;
				my = mouse_y - (vy * current_vz) ;
				zoomed_x = I->rect.x * current_vz;
				zoomed_y = I->rect.y * current_vz;
				zoomed_w = I->rect.w * current_vz;
				zoomed_h = I->rect.h * current_vz;
			}

			/* Manage event related to mouse position */
			if( (zoomed_x <= mx) &&
					((zoomed_x+zoomed_w) > mx) &&
					(zoomed_y <= my) &&
					((zoomed_y+zoomed_h) > my) ) {
				/* We are on overlay item: skip, non-overlay item */
				if(overlay_first) {
					skip_non_overlay=TRUE;
				}

				switch (event->type) {
				case SDL_MOUSEMOTION:
					if( I->over ) {
						/* x,y is the mouse pointer position relative to the item itself.
						i.e. 0,0 is the mouse pointer is in the upper-left corner of the item */
						I->over(I->over_arg,mx-I->rect.x,my-I->rect.y);
					}
					action_done = TRUE;
					break;
				case SDL_MOUSEBUTTONDOWN:
					sdl_keyboard_text_reset();
					if( I->editable ) {
						sdl_keyboard_text_init(I->string,I->edit_cb);
					}

					if( I->click_left && event->button.button == SDL_BUTTON_LEFT) {
						I->clicked=1;
					}
					if( I->click_right && event->button.button == SDL_BUTTON_RIGHT) {
						I->clicked=1;
					}
					action_done = TRUE;
					break;
				case SDL_MOUSEBUTTONUP:
					I->clicked=0;
					if( I->click_left && event->button.button == SDL_BUTTON_LEFT && event->button.clicks == 1) {
						I->click_left(I->click_left_arg);
					}
					if( I->click_right && event->button.button == SDL_BUTTON_RIGHT && event->button.clicks == 1) {
						I->click_right(I->click_right_arg);
					}
					if( I->double_click_left && event->button.button == SDL_BUTTON_LEFT && event->button.clicks == 2) {
						I->double_click_left(I->double_click_left_arg);
					}
					if( I->double_click_right && event->button.button == SDL_BUTTON_RIGHT && event->button.clicks == 2) {
						I->double_click_right(I->double_click_right_arg);
					}
					action_done = TRUE;
					break;
				case SDL_MOUSEWHEEL:
					if( event->wheel.timestamp != timestamp ) {
						if( event->wheel.y > 0 && I->wheel_up ) {
							timestamp = event->wheel.timestamp;
							I->wheel_up(I->wheel_up_arg);
							action_done = TRUE;
						}
						if( event->wheel.y < 0 && I->wheel_down ) {
							timestamp = event->wheel.timestamp;
							I->wheel_down(I->wheel_down_arg);
							action_done = TRUE;
						}
					}
					break;
				}
			}

			if(I->clicked) {
			}
			I = I->next;
		}
		overlay_first--;
		if(skip_non_overlay == TRUE) {
			break;
		}
	}

	if ( action_done == TRUE ) {
		screen_compose();
		return;
	}

	while(mousecb) {
		switch (event->type) {
		case SDL_MOUSEMOTION:
			if( mousecb->event_type != MOUSE_MOTION) {
				mousecb = mousecb->next;
				continue;
			}
			mousecb->cb(mouse_x,mouse_y);
			break;
		case SDL_MOUSEBUTTONDOWN:
			if( mousecb->event_type != MOUSE_BUTTON_DOWN) {
				mousecb = mousecb->next;
				continue;
			}
			mousecb->cb(event->button.button,0);
			break;
		case SDL_MOUSEBUTTONUP:
			if( mousecb->event_type != MOUSE_BUTTON_UP) {
				mousecb = mousecb->next;
				continue;
			}
			mousecb->cb(event->button.button,0);
			break;
		case SDL_MOUSEWHEEL:
			if( mousecb->event_type != MOUSE_WHEEL_UP && mousecb->event_type != MOUSE_WHEEL_DOWN ) {
				mousecb = mousecb->next;
				continue;
			}
			if( event->wheel.timestamp != timestamp ) {
				if( event->wheel.y > 0 && mousecb->event_type == MOUSE_WHEEL_UP ) {
					mousecb->cb(event->wheel.y,0);
				}
				if( event->wheel.y < 0 && mousecb->event_type == MOUSE_WHEEL_DOWN ) {
					mousecb->cb(event->wheel.y,0);
				}
			}
			break;
		}
		mousecb = mousecb->next;
	}
}

/************************************************************************
Take care of system's windowing event
return 1 if caller need to redraw it's screen
************************************************************************/
int sdl_screen_manager(SDL_Window * window,SDL_Renderer * render,SDL_Event * event)
{
	const Uint8 *keystate;

	switch (event->type) {
	case SDL_WINDOWEVENT:
		switch(event->window.event) {
		case SDL_WINDOWEVENT_RESIZED:
			SDL_RenderSetLogicalSize(render,event->window.data1,event->window.data2);
			return 1;
		}
		break;
	case SDL_KEYDOWN:
		switch (event->key.keysym.sym) {
		case SDLK_RETURN:
			keystate = SDL_GetKeyboardState(NULL);

			if( keystate[SDL_SCANCODE_RALT] || keystate[SDL_SCANCODE_LALT] ) {
				if(!fullscreen) {
					fullscreen = SDL_WINDOW_FULLSCREEN_DESKTOP;
				} else {
					fullscreen = 0;
				}
				SDL_SetWindowFullscreen(window,fullscreen);
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

	if( old_timer == 0 ) {
		old_timer = SDL_GetTicks();
	}

	global_time = SDL_GetTicks();
#if 0
	if( timer < old_timer + FRAME_DELAY ) {
		SDL_Delay(old_timer + FRAME_DELAY - timer);
	}
#endif

	if( virtual_tick + VIRTUAL_CAMERA_ANIM_DURATION > global_time ) {
		current_vx = (int)((double)old_vx + (double)( virtual_x - old_vx ) * (double)(global_time - virtual_tick) / (double)VIRTUAL_CAMERA_ANIM_DURATION);
		current_vy = (int)((double)old_vy + (double)( virtual_y - old_vy ) * (double)(global_time - virtual_tick) / (double)VIRTUAL_CAMERA_ANIM_DURATION);
		current_vz = (double)old_vz + (double)( virtual_z - old_vz ) * (double)(global_time - virtual_tick) / (double)VIRTUAL_CAMERA_ANIM_DURATION;
	} else {
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
void sdl_blit_tex(SDL_Renderer * render,SDL_Texture * tex, SDL_Rect * rect, double angle, double zoom_x, double zoom_y, int flip, int overlay)
{
	SDL_Rect r;
	int vx;
	int vy;

	if( tex == NULL ) {
		return;
	}

	if(overlay) {
		r.x = rect->x;
		r.y = rect->y;
	} else {
		get_virtual(render,&vx,&vy);

		r.x = rect->x + vx;
		r.y = rect->y + vy;
	}

	r.w = rect->w;
	r.h = rect->h;

	/* Sprite zoom */
	r.w *= zoom_x;
	r.h *= zoom_y;

	/* Do not globaly zoom overlay */
	if( !overlay) {
		/* Virtual zoom */
		r.x = ceil( (double)r.x * current_vz);
		r.y = ceil( (double)r.y * current_vz);
		r.w = ceil( (double)r.w * current_vz);
		r.h = ceil( (double)r.h * current_vz);
	}

	if( tex ) {
//		if( SDL_RenderCopy(render,tex,NULL,&r) < 0) {
		if( SDL_RenderCopyEx(render,tex,NULL,&r,angle,NULL,flip) < 0) {
			//Error
		}
	}
}

/************************************************************************
************************************************************************/
int get_current_frame(anim_t * anim, int loop, Uint32 anim_start_tick)
{
	int current_frame = 0;

	if( anim->total_duration != 0 ) {
		if( loop == FALSE ) {
			if( anim_start_tick + anim->total_duration < global_time) {
				current_frame = anim->num_frame-1;
			} else {
				Uint32 tick = anim_start_tick;
				int i;
				for( i=0 ; i<anim->num_frame ; i++ ) {
					if( tick + anim->delay[i] > global_time) {
						current_frame = i;
						break;
					}
					tick += anim->delay[i];
				}
			}
		}
		// Loop animation
		else {
			Uint32 tick = (global_time - anim_start_tick )  % anim->total_duration;
			Uint32 current_delay = 0;
			int i;
			for( i=0 ; i<anim->num_frame ; i++ ) {
				current_delay += anim->delay[i];
				if( tick < current_delay ) {
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
int sdl_blit_anim(SDL_Renderer * render,anim_t * anim, SDL_Rect * rect, double angle, double zoom_x, double zoom_y, int flip, int loop, int overlay, Uint32 anim_start_tick)
{
	if(anim == NULL) {
		return -1;
	}

	if(anim->tex==NULL) {
		return -1;
	}

	int current_frame = get_current_frame(anim,loop,anim_start_tick);

	sdl_blit_tex(render,anim->tex[current_frame],rect,angle,zoom_x,zoom_y,flip,overlay);

	return 0;
}

/************************************************************************
************************************************************************/
void sdl_get_string_size(TTF_Font * font,const char * string,int * w,int *h)
{
	SDL_Rect r;

	TTF_SizeText(font, string, &r.w, &r.h);
	*w = r.w;
	*h = r.h;
}

/************************************************************************
************************************************************************/
void sdl_print_item(SDL_Renderer * render,item_t * item)
{
	SDL_Surface * surf;
	SDL_Color fg= {0xff,0xff,0xff};
	SDL_Rect rect;
	anim_t * bg_anim;

	/* Get center of item */
	rect.x = item->rect.x + (item->rect.w/2);
	rect.y = item->rect.y + (item->rect.h/2);

	/* Get top/left of text */
	TTF_SizeText(item->font, item->string, &rect.w, &rect.h);
	rect.x = rect.x-(rect.w/2);
	rect.y = rect.y-(rect.h/2);

	if( item->string_bg != 0 ) {
		bg_anim = anim_create_color(render,rect.w,rect.h,item->string_bg);
		sdl_blit_anim(render,bg_anim,&rect,item->angle,item->zoom_x,item->zoom_y,item->flip,0,item->overlay,0);
		si_anim_free(bg_anim);
	}

	if(item->str_tex == NULL ) {
		surf = TTF_RenderText_Blended(item->font, item->string, fg);
		item->str_tex=SDL_CreateTextureFromSurface(render,surf);
		SDL_FreeSurface(surf);
	}

	sdl_blit_tex(render,item->str_tex,&rect,item->angle,item->zoom_x,item->zoom_y,item->flip,item->overlay);
}

/************************************************************************
************************************************************************/
int sdl_blit_item(SDL_Renderer * render,item_t * item)
{
	SDL_Rect rect;
	anim_array_t * anim_array = NULL;
	int is_moving = false;
	int i;

	if( item->move_start_tick ) {
		if( item->move_start_tick + item->move_duration > global_time) {
			is_moving = true;
			item->rect.x = (int)((double)item->from_px + (double)(item->to_px - item->from_px) * (double)(global_time - item->move_start_tick) / (double)item->move_duration);
			item->rect.y = (int)((double)item->from_py + (double)(item->to_py - item->from_py) * (double)(global_time - item->move_start_tick) / (double)item->move_duration);
		} else {
			item->rect.x =item->to_px;
			item->rect.y =item->to_py;
		}
	}

	if( item->saved_px ) {
		*item->saved_px = item->rect.x;
	}
	if( item->saved_py ) {
		*item->saved_py = item->rect.y;
	}

	if(item->anim.list && item->anim.list[0]) {
		anim_array = &item->anim;
	}
	if(item->anim_click.list && item->anim_click.list[0]) {
		anim_array = &item->anim_click;
	}
	if(item->anim_over.list && item->anim_over.list[0]) {
		anim_array = &item->anim_over;
	}
	if(is_moving && item->anim_move.list && item->anim_move.list[0]) {
		anim_array = &item->anim_move;
	}

	if(anim_array) {
		int max_width = 0;
		int max_height = 0;

		if( item->layout == LAYOUT_CENTER) {
			for(i=0; i<anim_array->num; i++) {
				if( anim_array->list[i]->w > max_width ) {
					max_width = anim_array->list[i]->w;
				}
				if( anim_array->list[i]->h > max_height ) {
					max_height = anim_array->list[i]->h;
				}
			}
		}

		for(i=0; i<anim_array->num; i++) {
			rect.x = item->rect.x;
			rect.y = item->rect.y;
			switch( item->layout ) {
				case LAYOUT_CENTER:
					rect.x = item->rect.x + (( max_width - anim_array->list[i]->w )/2) ;
					rect.y = item->rect.y + (( max_height - anim_array->list[i]->h )/2) ;
					break;

				case LAYOUT_TOP_LEFT:
				default:
					rect.x = item->rect.x;
					rect.y = item->rect.y;
					break;
			}
			rect.w = anim_array->list[i]->w;
			rect.h = anim_array->list[i]->h;
			sdl_blit_anim(render,anim_array->list[i],&rect,item->angle,item->zoom_x,item->zoom_y,item->flip,item->anim_loop,item->overlay,item->anim_start_tick);
		}
	}

	if( item->font != NULL && item->string != NULL ) {
		sdl_print_item(render,item);
	}

	return 0;
}

/************************************************************************
************************************************************************/
void sdl_blit_item_list(SDL_Renderer * render,item_t * list)
{
	item_t * item;

	item = list;
	while(item)  {
		sdl_blit_item(render,item);
		item = item->next;
	}
}

/************************************************************************
************************************************************************/
void sdl_keyboard_text_init(char * buf, void (*cb)(void*arg))
{
	if ( buf == NULL ) {
		return;
	}

	keyboard_text_index=strlen(buf);
	keyboard_text_buf = buf;
	keyboard_text_cb = cb;
}

/************************************************************************
************************************************************************/
void sdl_keyboard_text_reset()
{
	keyboard_text_index=0;
	keyboard_text_buf = NULL;
	keyboard_text_cb = NULL;
}

/************************************************************************
************************************************************************/
char * sdl_keyboard_text_get_buf()
{
	return keyboard_text_buf;
}

/************************************************************************
************************************************************************/
void sdl_keyboard_manager(SDL_Event * event)
{
	const Uint8 *keystate;
	keycb_t * key;

	switch (event->type) {
	case SDL_KEYUP:
		key = key_callback;
		if(key) {
			do {
				if( event->key.keysym.scancode == key->code) {
					if(key->cb_up) {
						key->cb_up(key->arg);
					}
				}
				key=key->next;
			} while(key);
		}
		break;
	case SDL_KEYDOWN:
		/* If no keyboard_text ready, key are used for UI */
		if( keyboard_text_buf == NULL ) {
			key = key_callback;
			if(key) {
				do {
					if( event->key.keysym.scancode == key->code) {
						key->cb(key->arg);
					}
					key = key->next;
				} while(key);
			}
			break;
		}

		/* Else keys are used to enter text */
		if( event->key.keysym.sym == SDLK_RETURN ) {
			if( keyboard_text_cb ) {
				keyboard_text_cb(keyboard_text_buf);
			}
		}

		if( event->key.keysym.sym == SDLK_DELETE ||
				event->key.keysym.sym == SDLK_BACKSPACE) {
			if(keyboard_text_index > 0 ) {
				keyboard_text_index--;
			}
			keyboard_text_buf[keyboard_text_index]=0;
		}

		if( event->key.keysym.sym >= SDLK_SPACE &&
				event->key.keysym.sym < SDLK_DELETE ) {

			/* Uppercase */
			keystate = SDL_GetKeyboardState(NULL);
			if( (keystate[SDL_SCANCODE_RSHIFT] ||
					keystate[SDL_SCANCODE_LSHIFT] ) &&
					(event->key.keysym.sym >=SDL_SCANCODE_A &&
					 event->key.keysym.sym <=SDL_SCANCODE_Z) ) {
				event->key.keysym.sym = (SDL_Scancode)(event->key.keysym.sym-32);
			}
			keyboard_text_buf[keyboard_text_index]=event->key.keysym.sym;
			/* TODO: check max buffer size */
#if 0
			if( keyboard_text_index < sizeof(keyboard_text_buf)) {
				keyboard_text_index++;
			}
#endif
			keyboard_text_index++;
			keyboard_text_buf[keyboard_text_index]=0;
		}
		screen_compose();
		break;
	default:
		break;
	}
}

/************************************************************************
************************************************************************/
void sdl_blit_to_screen(SDL_Renderer * render)
{
	SDL_RenderPresent(render);
}

/************************************************************************
************************************************************************/
void sdl_set_virtual_x(int x)
{
	old_vx = current_vx;
	if( x != virtual_x ) {
		virtual_x = x;
		virtual_tick = global_time;
	}
}

/************************************************************************
************************************************************************/
void sdl_set_virtual_y(int y)
{
	old_vy = current_vy;
	if( y != virtual_y ) {
		virtual_y = y;
		virtual_tick = global_time;
	}
}

/************************************************************************
************************************************************************/
void sdl_set_virtual_z(double z)
{
	if( z != virtual_z ) {
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
	old_vx =x;
}

/************************************************************************
************************************************************************/
void sdl_force_virtual_y(int y)
{
	virtual_y = y;
	current_vy = y;
	old_vy =y;
}

/************************************************************************
************************************************************************/
void sdl_force_virtual_z(double z)
{
	virtual_z = z;
	current_vz = z;
	old_vz =z;
}

/************************************************************************
************************************************************************/
void sdl_add_keycb(SDL_Scancode code,void (*cb)(void*),void (*cb_up)(void*), void * arg)
{
	keycb_t * key;

	if(key_callback==NULL) {
		key = malloc(sizeof(keycb_t));
		key_callback = key;
		key->code = code;
		key->cb = cb;
		key->cb_up = cb_up;
		key->arg = arg;
		key->next = NULL;
		return;
	} else {
		key = key_callback;
		while(key->next != NULL) {
			key = key->next;
		}
		key->next = malloc(sizeof(keycb_t));
		key = key->next;
		key->code = code;
		key->cb = cb;
		key->cb_up = cb_up;
		key->arg = arg;
		key->next = NULL;
		return;
	}
}

/************************************************************************
************************************************************************/
static void rec_free_keycb(keycb_t * key)
{
	if(key == NULL) {
		return;
	}

	if (key->next) {
		rec_free_keycb(key->next);
	}

	free(key);
}

/************************************************************************
************************************************************************/
void sdl_free_keycb()
{
	rec_free_keycb(key_callback);

	key_callback = NULL;
}

/************************************************************************
************************************************************************/
void sdl_add_mousecb(Uint32 event_type,void (*cb)(Uint32,Uint32))
{
	mousecb_t * mouse;

	if(mouse_callback==NULL) {
		mouse = malloc(sizeof(mousecb_t));
		mouse_callback = mouse;
		mouse->event_type = event_type;
		mouse->cb = cb;
		mouse->next = NULL;
		return;
	} else {
		mouse = mouse_callback;
		while(mouse->next != NULL) {
			mouse = mouse->next;
		}
		mouse->next = malloc(sizeof(mousecb_t));
		mouse = mouse->next;
		mouse->event_type = event_type;
		mouse->cb = cb;
		mouse->next = NULL;
		return;
	}
}

/************************************************************************
************************************************************************/
static void rec_free_mousecb(mousecb_t * mouse)
{
	if(mouse == NULL) {
		return;
	}

	if (mouse->next) {
		rec_free_mousecb(mouse->next);
	}

	free(mouse);
}

/************************************************************************
************************************************************************/
void sdl_free_mousecb()
{
	rec_free_mousecb(mouse_callback);

	mouse_callback = NULL;
}

/************************************************************************
************************************************************************/
Uint32 sdl_get_global_time()
{
	return global_time;
}

#ifdef __cplusplus
}
#endif

