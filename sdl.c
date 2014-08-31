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

#include "sdl.h"
#include <math.h>

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

static keycb_t * key_callback = NULL;

static void (*screen_compose)(void) = NULL;

/************************************************************************
************************************************************************/
//You must SDL_LockSurface(surface); then SDL_UnlockSurface(surface); before calling this function
void sdl_set_pixel(SDL_Surface *surface, int x, int y, Uint32 R, Uint32 G, Uint32 B, Uint32 A)
{
	Uint32 color = (A << 24) + (R << 16) + (G << 8) + (B);
	Uint8 *target_pixel = (Uint8 *)surface->pixels + y * surface->pitch + x * sizeof(color);
	*(Uint32 *)target_pixel = color;
}

/************************************************************************
************************************************************************/
void sdl_cleanup()
{
	SDL_Quit();
}

/************************************************************************
************************************************************************/
void sdl_init(const char * title, SDL_Renderer ** render,SDL_Window ** window, void (*screen_compose_cb)(void))
{
	if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
		exit(EXIT_FAILURE);
	}

	if (TTF_Init() == -1){
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

	*render = SDL_CreateRenderer(*window, -1, SDL_RENDERER_PRESENTVSYNC);
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
void sdl_mouse_manager(SDL_Renderer * render, SDL_Event * event, item_t * item_list)
{
	static int mouse_in = 1;
	SDL_Rect rect;
	int vx = 0;
	int vy = 0;
	item_t * I = NULL;
	int overlay_first = 1;
	int skip_non_overlay = 0;
	static int timestamp = 0;
	static int mouse_x = 0;
	static int mouse_y = 0;

	if (event->type == SDL_WINDOWEVENT) {
		switch (event->window.event) {
			case SDL_WINDOWEVENT_ENTER:
				mouse_in = 1;
//				printf("Mouse in\n");
				break;
			case SDL_WINDOWEVENT_LEAVE:
				mouse_in = 0;
//				printf("Mouse out\n");
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
				rect.x = mouse_x;
				rect.y = mouse_y;
			}
			else {
				if(overlay_first) {
					I = I->next;
					continue;
				}
				get_virtual(render,&vx,&vy);
				rect.x = mouse_x - vx;
				rect.y = mouse_y - vy;
			}

			I->current_frame = I->frame_normal;

			/* Manage event not related to mouse position */
			switch (event->type) {
			}

			/* Manage event related to mouse position */
			if( (I->rect.x < rect.x) &&
					((I->rect.x+I->rect.w) > rect.x) &&
					(I->rect.y < rect.y) &&
					((I->rect.y+I->rect.h) > rect.y) ) {
				/* We are on overlay item: skip, non-overlay item */
				if(overlay_first) {
					skip_non_overlay=1;
				}
				switch (event->type) {
					case SDL_MOUSEMOTION:
						I->current_frame = I->frame_over;
						if( I->over ) {
							I->over(I->over_arg);
						}
						screen_compose();
						break;
					case SDL_MOUSEBUTTONDOWN:
						sdl_keyboard_text_reset();
						if( I->editable ) {
							sdl_keyboard_text_init(I->string,I->edit_cb);
						}

						I->current_frame = I->frame_click;
						if( I->click_left && event->button.button == SDL_BUTTON_LEFT) {
							I->current_frame=I->frame_click;
							I->clicked=1;
						}
						if( I->click_right && event->button.button == SDL_BUTTON_RIGHT) {
							I->current_frame=I->frame_click;
							I->clicked=1;
						}
						screen_compose();
						break;
					case SDL_MOUSEBUTTONUP:
						I->clicked=0;
						I->current_frame = I->frame_normal;
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
						screen_compose();
						break;
					case SDL_MOUSEWHEEL:
						if( event->wheel.timestamp != timestamp ) {
							timestamp = event->wheel.timestamp;
							if( event->wheel.y > 0 && I->wheel_up ) {
								I->wheel_up(I->wheel_up_arg);
							}
							if( event->wheel.y < 0 && I->wheel_down ) {
								I->wheel_down(I->wheel_down_arg);
							}
							screen_compose();
							break;
						}
				}
			}
			if(I->clicked) {
				I->current_frame = I->frame_click;
			}
			I = I->next;
		}
		overlay_first--;
		if(skip_non_overlay) break;
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
	Uint32 timer;

	if( old_timer == 0 ) {
		old_timer = SDL_GetTicks();
	}

	timer = SDL_GetTicks();

	if( timer < old_timer + FRAME_DELAY ) {
		SDL_Delay(old_timer + FRAME_DELAY - timer);
	}

	timer = SDL_GetTicks();
	if( virtual_tick + VIRTUAL_ANIM_DURATION > timer ) {
		current_vx = (int)((double)old_vx + (double)( virtual_x - old_vx ) * (double)(timer - virtual_tick) / (double)VIRTUAL_ANIM_DURATION);
		current_vy = (int)((double)old_vy + (double)( virtual_y - old_vy ) * (double)(timer - virtual_tick) / (double)VIRTUAL_ANIM_DURATION);
		current_vz = (double)old_vz + (double)( virtual_z - old_vz ) * (double)(timer - virtual_tick) / (double)VIRTUAL_ANIM_DURATION;
	}
	else {
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

	if(overlay) {
		r.x = rect->x;
		r.y = rect->y;
	}
	else {
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

/*******************************
return 0 if blit OK
return -1 if blit NOK
*******************************/
int sdl_blit_anim(SDL_Renderer * render,anim_t * anim, SDL_Rect * rect, double angle, double zoom_x, double zoom_y, int flip, int start, int end,int overlay)
{
	Uint32 time = SDL_GetTicks();

	if(anim->tex==NULL) {
		return -1;
	}

	sdl_blit_tex(render,anim->tex[anim->current_frame],rect,angle,zoom_x,zoom_y,flip,overlay);

	if( anim->prev_time == 0 ) {
		anim->prev_time = time;
	}
	if( time >= anim->prev_time + anim->delay[anim->current_frame]) {
		(anim->current_frame)++;
		anim->prev_time = time;
		if( end != -1 ) {
			if(anim->current_frame >= end) {
				anim->current_frame = start;
				return 0;
			}
		} else {
			if(anim->current_frame >= anim->num_frame) {
				anim->current_frame = 0;
				return 0;
			}
		}
	}

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
//	SDL_Color bg={0,0,0};
	SDL_Color fg={0xff,0xff,0xff};
	SDL_Rect r;

	/* Get center of item */
        r.x = item->rect.x + (item->rect.w/2);
        r.y = item->rect.y + (item->rect.h/2);

	/* Get top/left of text */
	TTF_SizeText(item->font, item->string, &r.w, &r.h);
	r.x = r.x-(r.w/2);
	r.y = r.y-(r.h/2);

	if(item->str_tex == NULL ) {
		surf = TTF_RenderText_Blended(item->font, item->string, fg);
		item->str_tex=SDL_CreateTextureFromSurface(render,surf);
		SDL_FreeSurface(surf);
	}

	sdl_blit_tex(render,item->str_tex,&r,item->angle,item->zoom_x,item->zoom_y,item->flip,item->overlay);
}

/************************************************************************
************************************************************************/
int sdl_blit_item(SDL_Renderer * render,item_t * item)
{
	Uint32 timer = SDL_GetTicks();

	if(item->anim) {
		if( item->timer ) {
			if( item->timer + VIRTUAL_ANIM_DURATION > timer) {
				item->rect.x = (int)((double)item->old_x + (double)(item->x - item->old_x) * (double)(timer - item->timer) / (double)VIRTUAL_ANIM_DURATION);
				item->rect.y = (int)((double)item->old_y + (double)(item->y - item->old_y) * (double)(timer - item->timer) / (double)VIRTUAL_ANIM_DURATION);
			}
			else {
				item->rect.x =item->x;
				item->rect.y =item->y;
//printf("+++ anim reset : %d x %d\n", item->rect.x,item->rect.y);
			}
		}

		if( item->frame_normal == -1 ) {
			sdl_blit_anim(render,item->anim,&item->rect,item->angle,item->zoom_x,item->zoom_y,item->flip,item->anim_start,item->anim_end,item->overlay);
		} else {
			sdl_blit_tex(render,item->anim->tex[item->frame_normal],&item->rect,item->angle,item->zoom_x,item->zoom_y, item->flip,item->overlay);
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
	if( x != virtual_x ) {
		old_vx = current_vx;
		virtual_x = x;
		virtual_tick = SDL_GetTicks();
	}
}

/************************************************************************
************************************************************************/
void sdl_set_virtual_y(int y)
{
	if( y != virtual_y ) {
		old_vy = current_vy;
		virtual_y = y;
		virtual_tick = SDL_GetTicks();
	}
}

/************************************************************************
************************************************************************/
void sdl_set_virtual_z(double z)
{
	if( z != virtual_z ) {
		old_vz = current_vz;
		virtual_z = z;
		virtual_tick = SDL_GetTicks();
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
keycb_t * sdl_add_keycb(SDL_Scancode code,void (*cb)(void*),void (*cb_up)(void*), void * arg)
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
		return key;
	}
	else {
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
		return key;
	}
}

/************************************************************************
************************************************************************/
static void rec_free_keycb(keycb_t * key) {
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
void sdl_free_keycb(keycb_t ** key)
{
	rec_free_keycb(key_callback);

	key_callback = NULL;
}

