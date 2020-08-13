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
#include "SiAnim.h"
#include <SDL2/SDL.h>
#include <string>

#ifdef __cplusplus
extern "C"
{
#endif

#include <gif_lib.h>

#ifdef __cplusplus
}
#endif

static constexpr int GIF_GCE = 0xf9;

static constexpr int DEFAULT_DELAY_MS = 40;

/************************************************************************
 return nullptr if error
 http://www.imagemagick.org/Usage/anim_basics/#dispose
 http://wwwcdf.pd.infn.it/libgif/gif89.txt
 http://wwwcdf.pd.infn.it/libgif/gif_lib.html
 ************************************************************************/
SiAnim * giflib_load(const std::string & filePath)
{
	GifFileType * gif = nullptr;
	int i = 0;
	int j = 0;
	int transparent = 0;
	unsigned char transparent_color = 0;
	int disposal = 0;
	int delay = 0;
	ColorMapObject * global_pal = nullptr;
	ColorMapObject * pal = nullptr;
	SDL_Surface* surf = nullptr;
	SDL_Surface* prev_surf = nullptr;
	int x = 0;
	int y = 0;
	int col = 0;
	int pix_index = 0;
	SiAnim * anim = nullptr;
	int render_width;
	int render_height;
	int frame_left = 0;
	int frame_top = 0;
	int frame_width = 0;
	int frame_height = 0;
	int allow_draw = 1;
	int error = 0;
	int ret;

	gif = DGifOpenFileName(filePath.c_str(), &error);
	if (gif == nullptr)
	{
#if 0
		printf("%s: %s\n", filename,GifErrorString(error));
#endif
		return nullptr;
	}

	ret = DGifSlurp(gif);
	if (ret != GIF_OK)
	{
		DGifCloseFile(gif, &error);
		return nullptr;
	}
	if (gif->Error != D_GIF_SUCCEEDED)
	{
		DGifCloseFile(gif, &error);
		return nullptr;
	}

	anim = new SiAnim;

	anim->setWidth(gif->SWidth);
	anim->setHeight(gif->SHeight);

	render_width = gif->SWidth;
	render_height = gif->SHeight;
	//bg_color = gif->SBackGroundColor;

	surf = SDL_CreateRGBSurface(0, render_width, render_height, 32, 0xff000000, 0x00ff0000, 0x0000ff00, 0x000000ff);
	prev_surf = SDL_CreateRGBSurface(0, render_width, render_height, 32, 0xff000000, 0x00ff0000, 0x0000ff00, 0x000000ff);

	global_pal = gif->SColorMap;
	pal = global_pal;

	// Init with transparent background
	memset(surf->pixels, 0, render_height * render_width * 4);

	for (i = 0; i < gif->ImageCount; i++)
	{
		frame_left = gif->SavedImages[i].ImageDesc.Left;
		frame_top = gif->SavedImages[i].ImageDesc.Top;
		frame_width = gif->SavedImages[i].ImageDesc.Width;
		frame_height = gif->SavedImages[i].ImageDesc.Height;

		// select palette
		pal = global_pal;
		if (gif->SavedImages[i].ImageDesc.ColorMap)
		{
			pal = gif->SavedImages[i].ImageDesc.ColorMap;
		}
		// GCE
		for (j = 0; j < gif->SavedImages[i].ExtensionBlockCount; j++)
		{
			if (gif->SavedImages[i].ExtensionBlocks[j].Function == GIF_GCE)
			{
				transparent = gif->SavedImages[i].ExtensionBlocks[j].Bytes[0] & 0x01;
				disposal = (gif->SavedImages[i].ExtensionBlocks[j].Bytes[0] & 28) >> 2;
				delay = (gif->SavedImages[i].ExtensionBlocks[j].Bytes[1] + gif->SavedImages[i].ExtensionBlocks[j].Bytes[2] * 256) * 10;
				if (delay == 0)
				{
					delay = DEFAULT_DELAY_MS;
				}
				if (transparent)
				{
					transparent_color = gif->SavedImages[i].ExtensionBlocks[j].Bytes[3];
				}
			}
		}

		// Save the current render if needed
		if (disposal == DISPOSE_PREVIOUS)
		{
			memcpy(prev_surf->pixels, surf->pixels, render_height * render_width * 4);
		}

		// Fill surface buffer with raster bytes
		if (allow_draw)
		{ // See DISPOSE_DO_NOT
			for (y = 0; y < frame_height; y++)
			{
				for (x = 0; x < frame_width; x++)
				{
					pix_index = ((x + frame_left) + ((y + frame_top) * render_width)) * 4;
					col = gif->SavedImages[i].RasterBits[(x) + (y) * frame_width];
					if (col == transparent_color && transparent)
					{
						// Transparent color means do not touch the render
					}
					else
					{
						((char*) surf->pixels)[pix_index + 3] = pal->Colors[col].Red;
						((char*) surf->pixels)[pix_index + 2] = pal->Colors[col].Green;
						((char*) surf->pixels)[pix_index + 1] = pal->Colors[col].Blue;
						((char*) surf->pixels)[pix_index + 0] = 0xFF;
					}
				}
			}
		}

		anim->pushDelay(delay);
		anim->pushTexture(SDL_CreateTextureFromSurface(sdl_get_renderer(), surf));

		// Prepare next rendering depending of disposal
		allow_draw = 1;
		switch (disposal)
		{
		// Do not touch render for next frame
		case DISPOSE_DO_NOT:
			allow_draw = 0;
			break;
		case DISPOSE_BACKGROUND:
			// Draw transparent color in frame
			for (y = 0; y < frame_height; y++)
			{
				for (x = 0; x < frame_width; x++)
				{
					pix_index = ((x + frame_left) + ((y + frame_top) * render_width)) * 4;
					((char*) surf->pixels)[pix_index + 3] = 0;
					((char*) surf->pixels)[pix_index + 2] = 0;
					((char*) surf->pixels)[pix_index + 1] = 0;
					((char*) surf->pixels)[pix_index + 0] = 0;
				}
			}
			break;
		case DISPOSE_PREVIOUS:
			// Restore previous render in frame
			memcpy(surf->pixels, prev_surf->pixels, render_height * render_width * 4);
			break;
		default:
			break;
		}
	}
	SDL_FreeSurface(surf);
	SDL_FreeSurface(prev_surf);

	DGifCloseFile(gif, &error);

	return anim;
}
