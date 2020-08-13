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
#include "stdio.h"
#include <string>

#ifdef __cplusplus
extern "C"
{
#endif

#include <png.h>

#ifdef __cplusplus
}
#endif

/*****************************************************************************/
SDL_Texture * libpng_load_texture(const std::string & filePath, int * width_out, int * height_out)
{
	FILE *fp = nullptr;
	png_structp png_ptr = nullptr;
	png_infop info_ptr = nullptr;
	SDL_Surface* surf = nullptr;
	png_bytep *row_pointers = nullptr;
	png_uint_32 width = 0U;
	png_uint_32 height = 0U;
	int bit_depth = 0;
	int color_type = 0;
	png_uint_32 i = 0U;
	png_byte magic[8];
	SDL_Texture * tex = nullptr;

	// open image file
	fp = fopen(filePath.c_str(), "rb");
	if (!fp)
	{
		//fprintf (stderr, "error: couldn't open \"%s\"!\n", filename);
		return nullptr;
	}

	// read magic number
	fread(magic, 1, sizeof(magic), fp);

	// check for valid magic number
	if (!png_check_sig(magic, sizeof(magic)))
	{
		fclose(fp);
		return nullptr;
	}

	// create a png read struct
	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
	if (png_ptr == nullptr)
	{
		fclose(fp);
		return nullptr;
	}

	// create a png info struct
	info_ptr = png_create_info_struct(png_ptr);
	if (info_ptr == nullptr)
	{
		png_destroy_read_struct(&png_ptr, nullptr, nullptr);
		fclose(fp);
		return nullptr;
	}

	//wlog(LOGDEBUG,"Using libpng to decode %s",filename);

	// set error handling
	if (setjmp(png_jmpbuf(png_ptr)))
	{
		png_destroy_read_struct(&png_ptr, &info_ptr, (png_info **) nullptr);
		fclose(fp);
		// If we get here, we had a problem reading the file
		return nullptr;
	}

	// setup libpng for using standard C fread() function with our FILE pointer
	png_init_io(png_ptr, fp);

	// tell libpng that we have already read the magic number
	png_set_sig_bytes(png_ptr, sizeof(magic));

	// read the file information
	png_read_info(png_ptr, info_ptr);

	// get some usefull information from header
	bit_depth = png_get_bit_depth(png_ptr, info_ptr);
	color_type = png_get_color_type(png_ptr, info_ptr);

	// convert index color images to RGB images
	if (color_type == PNG_COLOR_TYPE_PALETTE)
	{
		png_set_palette_to_rgb(png_ptr);
	}

	// convert 1-2-4 bits grayscale images to 8 bits grayscale.
	if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8)
	{
		png_set_expand_gray_1_2_4_to_8(png_ptr);
	}

	if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))
	{
		png_set_tRNS_to_alpha(png_ptr);
	}

	if (bit_depth == 16)
	{
		png_set_strip_16(png_ptr);
	}
	else if (bit_depth < 8)
	{
		png_set_packing(png_ptr);
	}

	if (!(color_type &= PNG_COLOR_MASK_ALPHA))
	{
		png_set_filler(png_ptr, 0xff, PNG_FILLER_AFTER);
	}

	if (color_type == PNG_COLOR_TYPE_GRAY || color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
	{
		png_set_gray_to_rgb(png_ptr);
	}

	// optional call to update the info structure
	png_read_update_info(png_ptr, info_ptr);

	// retrieve updated information
	png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type, nullptr, nullptr, nullptr);

	//wlog(LOGDEBUG,"size: %dx%d bit_depth: %d, type: %d",width,height,bit_depth,color_type);
	// allocate the memory to hold the image using the fields of png_info.
	surf = SDL_CreateRGBSurface(0, width, height, 32, 0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000);
	memset(surf->pixels, 0, width * height * sizeof(Uint32));
	row_pointers = (png_bytep *) malloc(sizeof(png_bytep) * height);

	for (i = 0; i < height; i++)
	{
		row_pointers[height - i - 1] = (png_bytep) (surf->pixels) + ((height - (i + 1)) * width * sizeof(Uint32));
	}

	// the easiest way to read the image
	png_read_image(png_ptr, row_pointers);

	free(row_pointers);

	// read the rest of the file, getting any additional chunks in info_ptr
	png_read_end(png_ptr, info_ptr);

	// clean up after the read, and free any memory allocated
	png_destroy_read_struct(&png_ptr, &info_ptr, nullptr);

	// close the file
	fclose(fp);

	tex = SDL_CreateTextureFromSurface(sdl_get_renderer(), surf);
	SDL_FreeSurface(surf);

	*width_out = width;
	*height_out = height;

	return tex;
}

/*****************************************************************************/
SiAnim * libpng_load(const std::string & filePath)
{
	int width = 0;
	int height = 0;

	SDL_Texture * tex = libpng_load_texture(filePath, &width, &height);
	if (tex == nullptr)
	{
		return nullptr;
	}

	SiAnim * anim = new SiAnim;

	anim->pushDelay(0U);
	anim->pushTexture(tex);
	anim->setWidth(width);
	anim->setHeight(height);

	return anim;
}
