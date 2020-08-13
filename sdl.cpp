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
#include "sdl.h"
#include "SdlItem.h"
#include "SiAnim.h"
#include "SiKeyCallback.h"
#include "SiMouseEvent.h"
#include <assert.h>
#include <functional>
#include <iostream>
#include <math.h>
#include <string>
#include <vector>

static int fullscreen = 0;

static std::function<void(std::string)> keyboard_text_cb;
static SdlItem * focusedItem = nullptr;

static int virtual_x = 0;
static int virtual_y = 0;
static double virtual_z = 1.0;
static int old_vx = 0;
static int old_vy = 0;
static double old_vz = 1.0;
static int current_vx = 0;
static int current_vy = 0;
static double currentVz = 1.0;
static Uint32 virtual_tick = 0;
static int mouseX = 0;
static int mouseY = 0;
static bool isAppHasFocus = false;
static Uint32 globalTick;

static std::vector<struct SiKeyCallback> keyCbArray;
static std::vector<struct SiMouseEvent> globalMouseEventArray;

static SDL_Window * window = nullptr;
static SDL_Renderer * renderer = nullptr;

static constexpr int DEFAULT_SCREEN_W = 1024;
static constexpr int DEFAULT_SCREEN_H = 768;

/*****************************************************************************/
SDL_Renderer * sdl_get_renderer()
{
	return renderer;
}

/*****************************************************************************/
//You must SDL_LockSurface(surface); then SDL_UnlockSurface(surface); before calling this function
void sdl_set_pixel(SDL_Surface *surface, int x, int y, Uint32 R, Uint32 G, Uint32 B, Uint32 A)
{
	Uint32 color = (A << 24) + (R << 16) + (G << 8) + (B);
	Uint8 *target_pixel = (Uint8 *) surface->pixels + y * surface->pitch + x * surface->format->BytesPerPixel;
	*(Uint32 *) target_pixel = color;
}

/*****************************************************************************/
//You must SDL_LockSurface(surface); then SDL_UnlockSurface(surface); before calling this function
Uint32 sdl_get_pixel(SDL_Surface *surface, int x, int y)
{
	Uint8 *target_pixel = (Uint8 *) surface->pixels + y * surface->pitch + x * surface->format->BytesPerPixel;
	return *(Uint32 *) target_pixel;
}

/*****************************************************************************/
void sdl_cleanup()
{
	SDL_Quit();
}

/*****************************************************************************/
void sdl_init(const std::string & title, const bool vsync)
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
	SDL_WINDOWPOS_UNDEFINED, DEFAULT_SCREEN_W, DEFAULT_SCREEN_H, SDL_WINDOW_RESIZABLE);
	if (window == nullptr)
	{
		exit(EXIT_FAILURE);
	}

	Uint32 flags = SDL_RENDERER_ACCELERATED;
	if (vsync == true)
	{
		flags |= SDL_RENDERER_PRESENTVSYNC;
	}

	renderer = SDL_CreateRenderer(window, -1, flags);
	if (renderer == nullptr)
	{
		exit(EXIT_FAILURE);
	}

	SDL_RenderSetLogicalSize(renderer, DEFAULT_SCREEN_W, DEFAULT_SCREEN_H);
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
}

/*****************************************************************************/
void sdl_init_screen()
{
	focusedItem = nullptr;
}

/*****************************************************************************/
static void get_virtual(SDL_Renderer * renderer, int & virtual_x_px, int & virtual_y_px, int & width_px, int & height_px)
{
	SDL_GetRendererOutputSize(renderer, &width_px, &height_px);

	virtual_x_px = (width_px / currentVz / 2) - current_vx;
	virtual_y_px = (height_px / currentVz / 2) - current_vy;
}

/*****************************************************************************/
void sdl_mouse_position_manager(std::vector<SdlItem *> & itemArray)
{
	/*
	 if (isAppHasFocus == false)
	 {
	 return;
	 }
	 */

	int mx = 0;
	int my = 0;
	int vx = 0;
	int vy = 0;
	int width = 0;
	int height = 0;
	int zoomedX = 0;
	int zoomedY = 0;
	int zoomedW = 0;
	int zoomedH = 0;

	for (auto && item : itemArray)
	{
		if (item->isOverlay() == true)
		{
			mx = mouseX;
			my = mouseY;
			zoomedX = item->getRect().x;
			zoomedY = item->getRect().y;
			zoomedW = item->getRect().w;
			zoomedH = item->getRect().h;
		}
		else
		{
			get_virtual(renderer, vx, vy, width, height);
			mx = mouseX - (vx * currentVz);
			my = mouseY - (vy * currentVz);
			zoomedX = item->getRect().x * currentVz;
			zoomedY = item->getRect().y * currentVz;
			zoomedW = item->getRect().w * currentVz;
			zoomedH = item->getRect().h * currentVz;
		}

		item->clearAnimOver();
		item->clearAnimClick();

		if ((zoomedX <= mx) && ((zoomedX + zoomedW) > mx) && (zoomedY <= my) && ((zoomedY + zoomedH) > my))
		{
			item->setAnimOver(item->getDefaultAnimOver());

			// Display clicked anim
			if (SDL_GetMouseState(nullptr, nullptr))
			{
				item->setAnimClick(item->getDefaultAnimClick());
			}
		}
	}
}

/*****************************************************************************/
bool sdl_mouse_manager_with_overlay(SDL_Event * event, std::vector<SdlItem *> & itemArray, bool isOverlay, Uint32 & timeStamp)
{
	int mx = 0;
	int my = 0;
	int zoomedX = 0;
	int zoomedY = 0;
	int zoomedW = 0;
	int zoomedH = 0;
	focusedItem = nullptr;

	int vx = 0;
	int vy = 0;
	int width = 0;
	int height = 0;
	get_virtual(renderer, vx, vy, width, height);

	bool itemFound = false;

	for (auto && item : itemArray)
	{
		if (item->isOverlay() == true)
		{
			if (isOverlay == false)
			{
				continue;
			}
			mx = mouseX;
			my = mouseY;
			zoomedX = item->getRect().x;
			zoomedY = item->getRect().y;
			zoomedW = item->getRect().w;
			zoomedH = item->getRect().h;
		}
		else
		{
			if (isOverlay == true)
			{
				continue;
			}
			mx = mouseX - (vx * currentVz);
			my = mouseY - (vy * currentVz);
			zoomedX = item->getRect().x * currentVz;
			zoomedY = item->getRect().y * currentVz;
			zoomedW = item->getRect().w * currentVz;
			zoomedH = item->getRect().h * currentVz;
		}

		// Manage event related to mouse position
		if ((zoomedX <= mx) && (mx < (zoomedX + zoomedW)) && (zoomedY <= my) && (my < (zoomedY + zoomedH)))
		{
			if ((item->isOverlay() == true) && (isOverlay == true))
			{
				itemFound = true;
			}
			itemFound = true;

			switch (event->type)
			{
			case SDL_MOUSEMOTION:
				if (bool(item->getOverCb()) == true)
				{
					/* x,y is the mouse pointer position relative to the item itself.
					 i.e. 0,0 is the mouse pointer is in the upper-left corner of the item */
					item->getOverCb()(mx - item->getRect().x, my - item->getRect().y);
				}
				break;
			case SDL_MOUSEBUTTONDOWN:
				focusedItem = nullptr;

				if (item->isEditable() == true)
				{
					focusedItem = item;
				}

				if ((bool(item->getClickLeftCb()) == true) && (event->button.button == SDL_BUTTON_LEFT))
				{
					item->setClicked(true);
				}
				if ((bool(item->getClickRightCb()) == true) && (event->button.button == SDL_BUTTON_RIGHT))
				{
					item->setClicked(true);
				}
				break;
			case SDL_MOUSEBUTTONUP:
				item->setClicked(false);
				if ((bool(item->getClickLeftCb()) == true) && (event->button.button == SDL_BUTTON_LEFT) && (event->button.clicks == 1))
				{
					item->getClickLeftCb()();
				}
				if ((bool(item->getClickRightCb()) == true) && (event->button.button == SDL_BUTTON_RIGHT) && (event->button.clicks == 1))
				{
					item->getClickRightCb()();
				}
				if ((bool(item->getDoubleClickLeftCb()) == true) && (event->button.button == SDL_BUTTON_LEFT) && (event->button.clicks == 2))
				{
					item->getDoubleClickLeftCb()();
				}
				if ((bool(item->getDoubleClickRightCb()) == true) && (event->button.button == SDL_BUTTON_RIGHT) && (event->button.clicks == 2))
				{
					item->getDoubleClickRightCb()();
				}
				break;
			case SDL_MOUSEWHEEL:
				if (event->wheel.timestamp != timeStamp)
				{
					if ((event->wheel.y > 0) && (bool(item->getWheelUpCb()) == true))
					{
						timeStamp = event->wheel.timestamp;
						item->getWheelUpCb()();
					}
					if ((event->wheel.y < 0) && (bool(item->getWheelDownCb()) == true))
					{
						timeStamp = event->wheel.timestamp;
						item->getWheelDownCb()();
					}
				}
				break;
			}
		}
	}

	return itemFound;
}

/*****************************************************************************/
bool sdl_mouse_manager(SDL_Event * event, std::vector<SdlItem *> & itemArray)
{
	static Uint32 timeStamp = 0;

	if (event->type == SDL_WINDOWEVENT)
	{
		switch (event->window.event)
		{
		case SDL_WINDOWEVENT_ENTER:
			isAppHasFocus = true;
			break;
		case SDL_WINDOWEVENT_LEAVE:
			isAppHasFocus = false;
			break;
		default:
			break;
		}
	}

	if (isAppHasFocus == false)
	{
		return false;
	}

	if (event->type == SDL_MOUSEMOTION)
	{
		mouseX = event->motion.x;
		mouseY = event->motion.y;
	}

	if (sdl_mouse_manager_with_overlay(event, itemArray, true, timeStamp) == false)
	{
		sdl_mouse_manager_with_overlay(event, itemArray, false, timeStamp);
	}

	for (auto && mouseEvent : globalMouseEventArray)
	{
		switch (event->type)
		{
		case SDL_MOUSEMOTION:
			if (mouseEvent.getEventType() != MOUSE_MOTION)
			{
				continue;
			}
			mouseEvent.getCallBack()();
			break;
		case SDL_MOUSEBUTTONDOWN:
			if (mouseEvent.getEventType() != MOUSE_BUTTON_DOWN)
			{
				continue;
			}
			mouseEvent.getCallBack()();
			break;
		case SDL_MOUSEBUTTONUP:
			if (mouseEvent.getEventType() != MOUSE_BUTTON_UP)
			{
				continue;
			}
			mouseEvent.getCallBack()();
			break;
		case SDL_MOUSEWHEEL:
			if ((mouseEvent.getEventType() != MOUSE_WHEEL_UP) && (mouseEvent.getEventType() != MOUSE_WHEEL_DOWN))
			{
				continue;
			}
			if (event->wheel.timestamp != timeStamp)
			{
				if ((event->wheel.y > 0) && (mouseEvent.getEventType() == MOUSE_WHEEL_UP))
				{
					mouseEvent.getCallBack()();
				}
				if ((event->wheel.y < 0) && (mouseEvent.getEventType() == MOUSE_WHEEL_DOWN))
				{
					mouseEvent.getCallBack()();
				}
			}
			break;
		}
	}

	return false;
}

/******************************************************************************
 Take care of system's windowing event
 return 1 if caller need to redraw it's screen
 *****************************************************************************/
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

/*****************************************************************************/
void sdl_loop_manager()
{
	static Uint32 oldTimer = 0U;

	if (oldTimer == 0)
	{
		oldTimer = SDL_GetTicks();
	}

	globalTick = SDL_GetTicks();
#if 0
	if( timer < oldTimer + FRAME_DELAY )
	{
		SDL_Delay(oldTimer + FRAME_DELAY - timer);
	}
#endif

	if (virtual_tick + VIRTUAL_CAMERA_ANIM_DURATION > globalTick)
	{
		current_vx = (int) ((double) old_vx + (double) (virtual_x - old_vx) * (double) (globalTick - virtual_tick) / (double) VIRTUAL_CAMERA_ANIM_DURATION);
		current_vy = (int) ((double) old_vy + (double) (virtual_y - old_vy) * (double) (globalTick - virtual_tick) / (double) VIRTUAL_CAMERA_ANIM_DURATION);
		currentVz = (double) old_vz + (double) (virtual_z - old_vz) * (double) (globalTick - virtual_tick) / (double) VIRTUAL_CAMERA_ANIM_DURATION;
	}
	else
	{
		old_vx = virtual_x;
		current_vx = virtual_x;

		old_vy = virtual_y;
		current_vy = virtual_y;

		old_vz = virtual_z;
		currentVz = virtual_z;
	}
}

/******************************************************************************
 flip is one of SDL_FLIP_NONE, SDL_FLIP_HORIZONTAL, SDL_FLIP_VERTICAL
 *****************************************************************************/
void sdl_blit_tex(SDL_Texture * tex, SDL_Rect * rect, double angle, double zoom_x, double zoom_y, int flip, int overlay)
{
	SDL_Rect r =
	{ 0, 0, 0, 0 };
	int vx = 0;
	int vy = 0;
	int width_px = 0;
	int height_px = 0;

	if (tex == nullptr)
	{
		return;
	}

	get_virtual(renderer, vx, vy, width_px, height_px);

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
		r.x = ceil((double) r.x * currentVz);
		r.y = ceil((double) r.y * currentVz);
		r.w = ceil((double) r.w * currentVz);
		r.h = ceil((double) r.h * currentVz);
	}

	// Crop
	if ((r.x > width_px) || ((r.x + r.w) < 0) || (r.y > height_px) || ((r.y + r.h) < 0))
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

/*****************************************************************************/
static int get_current_frame(const SiAnim & anim, const bool isLoop, const Uint32 startTick)
{
	if (anim.getTotalDuration() != 0U)
	{
		if (isLoop == true)
		{
			Uint32 tick = (globalTick - startTick) % anim.getTotalDuration();
			Uint32 currentDelay = 0;
			int frameIndex = 0;
			for (auto && delay : anim.getDelayArray())
			{
				currentDelay += delay;
				if (tick < currentDelay)
				{
					return frameIndex;
				}
				frameIndex++;
			}
		}
		else
		{
			if (startTick + anim.getTotalDuration() < globalTick)
			{
				return anim.getFrameQty() - 1;
			}
			else
			{
				Uint32 tick = startTick;
				int frameIndex = 0;
				for (auto && delay : anim.getDelayArray())
				{
					if (tick + delay > globalTick)
					{
						return frameIndex;
					}
					tick += delay;
					frameIndex++;
				}
			}
		}
	}

	return 0;
}
/******************************************************************************
 return 0 if blit OK
 return -1 if blit NOK
 *****************************************************************************/
int sdl_blit_anim(const SiAnim & anim, SDL_Rect * rect, const double angle, const double zoomX, const double zoomY, const bool isFlip, const bool isLoop,
		const bool isOverlay, const Uint32 animStartTick)
{
	if (anim.getTextureArray().size() == 0)
	{
		return -1;
	}

	int current_frame = get_current_frame(anim, isLoop, animStartTick);

	sdl_blit_tex(anim.getTexture(current_frame)->getTexture(), rect, angle, zoomX, zoomY, isFlip, isOverlay);

	return 0;
}

/*****************************************************************************/
void sdl_get_string_size(TTF_Font * font, const std::string & text, int * w, int *h)
{
	SDL_Rect r =
	{ 0, 0, 0, 0 };

	TTF_SizeText(font, text.c_str(), &r.w, &r.h);
	*w = r.w;
	*h = r.h;
}

/*****************************************************************************/
void sdl_print_item(SdlItem & item)
{
	if (item.getText().empty() == true)
	{
		return;
	}

	if (item.getFont() == nullptr)
	{
		return;
	}

	int textWidth = 0;
	int textHeight = 0;
	int backgroundWidth = item.getRect().w;
	int backgroundHeight = item.getRect().h;

	TTF_SizeText(item.getFont(), item.getText().c_str(), &textWidth, &textHeight);

	if (backgroundWidth < textWidth)
	{
		backgroundWidth = textWidth;
	}

	if (backgroundHeight < textHeight)
	{
		backgroundHeight = textHeight;
	}

	SDL_Rect rect =
	{ 0, 0, 0, 0 };

	rect.x = item.getRect().x;
	rect.y = item.getRect().y;

	if (item.getBackGroudColor() != 0)
	{
		rect.w = backgroundWidth;
		rect.h = backgroundHeight;
		SiAnim * bgAnim = anim_create_color(rect.w, rect.h, item.getBackGroudColor());
		sdl_blit_anim(*bgAnim, &rect, item.getAngle(), item.getZoomX(), item.getZoomY(), item.getFlip(), false, item.isOverlay(), 0);
		delete bgAnim;
	}

	if (item.getTextTexture() == nullptr)
	{
		SDL_Color fg =
		{ 0xff, 0xff, 0xff };

		SDL_Surface * surf = TTF_RenderText_Blended(item.getFont(), item.getText().c_str(), fg);
		item.setTextTexture(SDL_CreateTextureFromSurface(renderer, surf));
		SDL_FreeSurface(surf);
	}

	rect.w = textWidth;
	rect.h = textHeight;
	sdl_blit_tex(item.getTextTexture(), &rect, item.getAngle(), item.getZoomX(), item.getZoomY(), item.getFlip(), item.isOverlay());
}

/*****************************************************************************/
static void sdl_blit_anim_array(const SdlItem & item, const std::vector<SiAnim *> & animArray)
{
	int max_width = 0;
	int max_height = 0;

	if (item.getLayout() == SdlItem::Layout::CENTER)
	{
		for (auto && anim : animArray)
		{
			if (anim->getWidth() > max_width)
			{
				max_width = anim->getWidth();
			}
			if (anim->getHeight() > max_height)
			{
				max_height = anim->getHeight();
			}
		}
	}

	SDL_Rect rect =
	{ 0, 0, 0, 0 };

	for (auto anim : animArray)
	{
		switch (item.getLayout())
		{
		case SdlItem::Layout::CENTER:
			rect.x = item.getRect().x + ((max_width - anim->getWidth()) / 2);
			rect.y = item.getRect().y + ((max_height - anim->getHeight()) / 2);
			break;

		case SdlItem::Layout::TOP_LEFT:
		default:
			rect.x = item.getRect().x;
			rect.y = item.getRect().y;
			break;
		}

		rect.w = anim->getWidth();
		rect.h = anim->getHeight();

		sdl_blit_anim(*anim, &rect, item.getAngle(), item.getZoomX(), item.getZoomY(), item.getFlip(), item.isAnimLoop(), item.isOverlay(),
				item.getAnimStartTick());
	}
}

/*****************************************************************************/
int sdl_blit_item(SdlItem & item)
{
	sdl_blit_anim_array(item, item.getAnim());
	sdl_blit_anim_array(item, item.getAnimClick());
	sdl_blit_anim_array(item, item.getAnimOver());

	sdl_print_item(item);

	return 0;
}

/*****************************************************************************/
void sdl_blit_item_list(std::vector<SdlItem> & itemArray)
{
	for (auto && item : itemArray)
	{
		sdl_blit_item(item);
	}
}

/*****************************************************************************/
bool sdl_keyboard_manager(SDL_Event * event)
{
	const Uint8 *keystate = nullptr;

	switch (event->type)
	{
	case SDL_KEYUP:
		if (event->key.repeat == 0)
		{
			for (auto && key : keyCbArray)
			{
				if (event->key.keysym.scancode == key.getCode())
				{
					if (bool(key.getUpCallBack()) == true)
					{
						key.getUpCallBack()();
					}
				}
			}
		}
		break;
	case SDL_KEYDOWN:
		if (event->key.repeat == 0)
		{
			if (focusedItem == nullptr)
			{
				for (auto && key : keyCbArray)
				{
					if (event->key.keysym.scancode == key.getCode())
					{
						if (bool(key.getDownCallBack()) == true)
						{
							key.getDownCallBack()();
						}
					}
				}
			}
		}
		else
		{
			// Keys are used to enter text
			if (event->key.keysym.sym == SDLK_RETURN)
			{
				focusedItem->getEditCb()(focusedItem->getText());
			}

			if (event->key.keysym.sym == SDLK_DELETE || event->key.keysym.sym == SDLK_BACKSPACE)
			{
				focusedItem->removeFromText(1);
			}

			if (event->key.keysym.sym >= SDLK_SPACE && event->key.keysym.sym < SDLK_DELETE)
			{
				// Upper case
				keystate = SDL_GetKeyboardState(nullptr);
				if ((keystate[SDLK_RSHIFT] || keystate[SDLK_LSHIFT]) && (event->key.keysym.sym >= SDLK_a && event->key.keysym.sym <= SDLK_z))
				{
					event->key.keysym.sym = (SDL_Keycode) (event->key.keysym.sym - 32);
				}

				char keyString[2];
				keyString[0] = (char) (event->key.keysym.sym);
				keyString[1] = 0;
				focusedItem->addToText(std::string(keyString));
			}
			return true;
		}
		break;
	default:
		break;
	}

	return false;
}

/*****************************************************************************/
void sdl_blit_to_screen()
{
	SDL_RenderPresent(renderer);
}

/*****************************************************************************/
void sdl_set_virtual_x(int x)
{
	old_vx = current_vx;
	if (x != virtual_x)
	{
		virtual_x = x;
		virtual_tick = globalTick;
	}
}

/*****************************************************************************/
void sdl_set_virtual_y(int y)
{
	old_vy = current_vy;
	if (y != virtual_y)
	{
		virtual_y = y;
		virtual_tick = globalTick;
	}
}

/*****************************************************************************/
void sdl_set_virtual_z(double z)
{
	if (z != virtual_z)
	{
		old_vz = currentVz;
		virtual_z = z;
		virtual_tick = globalTick;
	}
}

/*****************************************************************************/
int sdl_get_virtual_x()
{
	return virtual_x;
}

/*****************************************************************************/
int sdl_get_virtual_y()
{
	return virtual_y;
}

/*****************************************************************************/
double sdl_get_virtual_z()
{
	return virtual_z;
}

/*****************************************************************************/
void sdl_force_virtual_x(int x)
{
	virtual_x = x;
	current_vx = x;
	old_vx = x;
}

/*****************************************************************************/
void sdl_force_virtual_y(int y)
{
	virtual_y = y;
	current_vy = y;
	old_vy = y;
}

/*****************************************************************************/
void sdl_force_virtual_z(double z)
{
	virtual_z = z;
	currentVz = z;
	old_vz = z;
}

/*****************************************************************************/
void sdl_add_down_key_cb(const SDL_Scancode code, const std::function<void()> & downCb)
{
	struct SiKeyCallback key;

	key.setCode(code);
	key.setDownCallBack(downCb);

	keyCbArray.push_back(key);
}

/*****************************************************************************/
void sdl_add_up_key_cb(const SDL_Scancode code, const std::function<void()> & upCb)
{
	struct SiKeyCallback key;

	key.setCode(code);
	key.setUpCallBack(upCb);

	keyCbArray.push_back(key);
}

/*****************************************************************************/
void sdl_clean_key_cb()
{
	keyCbArray.clear();
}

/*****************************************************************************/
void sdl_add_mousecb(Uint32 eventType, std::function<void()> callBack)
{
	SiMouseEvent mouseEvent;
	mouseEvent.setEventType(eventType);
	mouseEvent.setCallBack(callBack);

	globalMouseEventArray.push_back(mouseEvent);
}

/*****************************************************************************/
void sdl_free_mousecb()
{
	globalMouseEventArray.clear();
}

/*****************************************************************************/
Uint32 sdl_get_global_time()
{
	return globalTick;
}

/*****************************************************************************/
SiAnim * sdl_get_minimal_anim()
{
	SiAnim * def_anim = new SiAnim;

	def_anim->pushTexture(SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STATIC, 1, 1));
	def_anim->setWidth(1);
	def_anim->setHeight(1);
	def_anim->pushDelay(0U);
	def_anim->setTotalDuration(0U);

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

