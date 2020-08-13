/*
 World of Gnome is a 2D multiplayer role playing game.
 Copyright (C) 2020 carabobz@gmail.com

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
#include "si_gif.h"
#include "si_libav.h"
#include "si_png.h"
#include "si_zip.h"
#include "SiAnim.h"
#include <string>

/*****************************************************************************/
SiAnim * anim_load(const std::string & filePath)
{
	SiAnim * ret;

	ret = giflib_load(filePath);
	if (ret == nullptr)
	{
		ret = libpng_load(filePath);
		if (ret == nullptr)
		{
			ret = libzip_load(filePath);
			if (ret == nullptr)
			{
				ret = libav_load(filePath);
			}
		}
	}

	if (ret != nullptr)
	{
		Uint32 totalDuration = 0U;

		for (auto && delay : ret->getDelayArray())
		{
			totalDuration += delay;
		}

		ret->setTotalDuration(totalDuration);
	}

	return ret;
}

/******************************************************************************
 color is RGBA
 *****************************************************************************/
SiAnim * anim_create_color(int width, int height, Uint32 color)
{
	SiAnim * anim = new SiAnim;

	anim->setWidth(width);
	anim->setHeight(height);
	anim->pushDelay(0U);

	SDL_Surface* surf = SDL_CreateRGBSurface(0, width, height, 32, 0xff000000, 0x00ff0000, 0x0000ff00, 0x000000ff);

	Uint32 * to_fill = (Uint32*) (surf->pixels);
	for (int i = 0; i < width * height; i++)
	{
		to_fill[i] = color;
	}

	anim->pushTexture(SDL_CreateTextureFromSurface(sdl_get_renderer(), surf));

	SDL_FreeSurface(surf);

	return anim;
}
