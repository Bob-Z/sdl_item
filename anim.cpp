/*
 sdl_item is a graphical library based on SDL.
 Copyright (C) 2013-2019 carabobz@gmail.com

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

#define GIF_GCE			(0xf9)

#define DEFAULT_DELAY		(40)

#define ZIP_TIMING_FILE		"timing"
#define ZIP_TMP_FILE		"/tmp/si_tmp"

#ifdef __cplusplus
extern "C"
{
#endif

#include <gif_lib.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
#include <png.h>
#include <zip.h>

/************************************************************************
 return nullptr if error
 http://www.imagemagick.org/Usage/anim_basics/#dispose
 http://wwwcdf.pd.infn.it/libgif/gif89.txt
 http://wwwcdf.pd.infn.it/libgif/gif_lib.html
 ************************************************************************/
static anim_t * giflib_load(const char * filename)
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
	anim_t * anim = nullptr;
	int render_width;
	int render_height;
	int frame_left = 0;
	int frame_top = 0;
	int frame_width = 0;
	int frame_height = 0;
	int allow_draw = 1;
	int error = 0;
	int ret;

	gif = DGifOpenFileName(filename, &error);
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

	anim = (anim_t*) malloc(sizeof(anim_t));
	memset(anim, 0, sizeof(anim_t));

	anim->num_frame = gif->ImageCount;
	anim->tex = (SDL_Texture **) malloc(sizeof(SDL_Texture *) * anim->num_frame);
	anim->w = gif->SWidth;
	anim->h = gif->SHeight;
	anim->delay = (Uint32*) malloc(sizeof(Uint32) * anim->num_frame);

	render_width = gif->SWidth;
	render_height = gif->SHeight;
	//bg_color = gif->SBackGroundColor;

	surf = SDL_CreateRGBSurface(0, render_width, render_height, 32, 0xff000000, 0x00ff0000, 0x0000ff00, 0x000000ff);
	prev_surf = SDL_CreateRGBSurface(0, render_width, render_height, 32, 0xff000000, 0x00ff0000, 0x0000ff00, 0x000000ff);

	global_pal = gif->SColorMap;
	pal = global_pal;

	/* Init with transparent background */
	memset(surf->pixels, 0, render_height * render_width * 4);

	for (i = 0; i < gif->ImageCount; i++)
	{
		frame_left = gif->SavedImages[i].ImageDesc.Left;
		frame_top = gif->SavedImages[i].ImageDesc.Top;
		frame_width = gif->SavedImages[i].ImageDesc.Width;
		frame_height = gif->SavedImages[i].ImageDesc.Height;

		/* select palette */
		pal = global_pal;
		if (gif->SavedImages[i].ImageDesc.ColorMap)
		{
			pal = gif->SavedImages[i].ImageDesc.ColorMap;
		}
		/* GCE */
		for (j = 0; j < gif->SavedImages[i].ExtensionBlockCount; j++)
		{
			if (gif->SavedImages[i].ExtensionBlocks[j].Function == GIF_GCE)
			{
				transparent = gif->SavedImages[i].ExtensionBlocks[j].Bytes[0] & 0x01;
				disposal = (gif->SavedImages[i].ExtensionBlocks[j].Bytes[0] & 28) >> 2;
				delay = (gif->SavedImages[i].ExtensionBlocks[j].Bytes[1] + gif->SavedImages[i].ExtensionBlocks[j].Bytes[2] * 256) * 10;
				if (delay == 0)
				{
					delay = DEFAULT_DELAY;
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

		/* Fill surface buffer with raster bytes */
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

		anim->delay[i] = delay;
		anim->tex[i] = SDL_CreateTextureFromSurface(sdl_get_renderer(), surf);

		/* Prepare next rendering depending of disposal */
		allow_draw = 1;
		switch (disposal)
		{
		// Do not touch render for next frame
		case DISPOSE_DO_NOT:
			allow_draw = 0;
			break;
		case DISPOSE_BACKGROUND:
			/* Draw transparent color in frame */
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

/************************************************************************
 return nullptr if error
 ************************************************************************/
static SDL_Texture * libpng_load_texture(const char * filename, int * width_out, int * height_out)
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
	fp = fopen(filename, "rb");
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

/************************************************************************
 return nullptr if error
 ************************************************************************/
static anim_t * libpng_load(const char * filename)
{
	anim_t * anim = nullptr;
	SDL_Texture * tex;
	int width;
	int height;

	tex = libpng_load_texture(filename, &width, &height);
	if (tex == nullptr)
	{
		return nullptr;
	}

	anim = (anim_t*) malloc(sizeof(anim_t));
	memset(anim, 0, sizeof(anim_t));

	anim->num_frame = 1;
	anim->tex = (SDL_Texture **) malloc(sizeof(SDL_Texture *) * anim->num_frame);
	anim->delay = (Uint32*) malloc(sizeof(Uint32) * anim->num_frame);
	anim->delay[0] = 0;
	anim->tex[0] = tex;
	anim->w = width;
	anim->h = height;

	return anim;
}

/************************************************************************
 ************************************************************************/
static int cmp(const void *p1, const void *p2)
{
	return strcmp(*(char **) p1, *(char **) p2);
}

/************************************************************************
 ************************************************************************/
static void read_timing(const char * filename, int count, Uint32 * delay)
{
	FILE * file;
	int i = 0;

	file = fopen(filename, "r");
	for (i = 0; i < count; i++)
	{
		fscanf(file, "%d", &delay[i]);
	}
	fclose(file);
}

/************************************************************************
 return <0 on failure
 ************************************************************************/
static int extract_zip(struct zip *fd_zip, int index)
{
	struct zip_stat file_stat;
	struct zip_file *file_zip = nullptr;
	char * data;
	FILE *file_dest;

	zip_stat_index(fd_zip, index, 0, &file_stat);

	file_zip = zip_fopen(fd_zip, file_stat.name, ZIP_FL_UNCHANGED);
	if (file_zip == 0)
	{
		return -1;
	}

	data = (char*) malloc((size_t) (file_stat.size));
	if (zip_fread(file_zip, data, (size_t) (file_stat.size)) != (int64_t) file_stat.size)
	{
		free(data);
		zip_fclose(file_zip);
		return -1;
	}

	file_dest = fopen(ZIP_TMP_FILE, "wb");
	if (file_dest == nullptr)
	{
		free(data);
		zip_fclose(file_zip);
		return -1;
	}

	if (fwrite(data, sizeof(char), (size_t) file_stat.size, file_dest) != file_stat.size)
	{
		fclose(file_dest);
		free(data);
		zip_fclose(file_zip);
		return -1;
	}

	fclose(file_dest);
	free(data);
	zip_fclose(file_zip);

	return 0;
}

/************************************************************************
 return nullptr if error
 ************************************************************************/
static anim_t * libzip_load(const char * filename)
{
	anim_t * anim = nullptr;
	struct zip *fd_zip = nullptr;
	int err = 0;
	int file_count = 0;
	int anim_count = 0;
	int i;
	char ** zip_filename = nullptr;
	int index = 0;

	fd_zip = zip_open(filename, ZIP_CHECKCONS, &err);
	if (err != ZIP_ER_OK)
	{
#if 0
		zip_error_to_str(buf_erreur, sizeof buf_erreur, err, errno);
		printf("Error %d : %s\n",err, buf_erreur);
#endif
		return nullptr;
	}

	if (fd_zip == nullptr)
	{
		return nullptr;
	}

	file_count = zip_get_num_files(fd_zip);
	if (file_count <= 0)
	{
		zip_close(fd_zip);
		return nullptr;
	}

	// Remove timing file
	anim_count = file_count - 1;
	anim = (anim_t*) malloc(sizeof(anim_t));
	memset(anim, 0, sizeof(anim_t));
	anim->tex = (SDL_Texture**) malloc(anim_count * sizeof(SDL_Texture*));
	anim->delay = (Uint32*) malloc(anim_count * sizeof(Uint32));
	for (i = 0; i < anim_count; i++)
	{
		anim->delay[i] = DEFAULT_DELAY;
	}

	// Get zip archive filenames and sort them alphabetically
	zip_filename = (char**) malloc(sizeof(char*) * file_count);
	for (i = 0; i < file_count; i++)
	{
		zip_filename[i] = strdup(zip_get_name(fd_zip, i, ZIP_FL_UNCHANGED));
	}

	qsort(zip_filename, file_count, sizeof(char*), cmp);

	// Read file in archive and process them (either as PNG file or timing file
	for (i = 0; i < file_count; i++)
	{
		index = zip_name_locate(fd_zip, zip_filename[i], 0);
		if (index == -1)
		{
			continue;
		}

		// Create ZIP_TMP_FILE file
		if (extract_zip(fd_zip, index) < 0)
		{
			continue;
		}

		// timing file
		if (!strcmp(zip_filename[i], ZIP_TIMING_FILE))
		{
			read_timing(ZIP_TMP_FILE, file_count - 1, anim->delay);
			continue;
		}

		/* PNG file */
		anim->tex[anim->num_frame] = libpng_load_texture(ZIP_TMP_FILE, &anim->w, &anim->h);
		if (anim->tex[anim->num_frame] == nullptr)
		{
#if 0
			printf("Corrupted PNG file");
#endif
			continue;
		}

		anim->num_frame++;
	}

	/* Clean-up */
	for (i = 0; i < file_count; i++)
	{
		free(zip_filename[i]);
	}

	zip_close(fd_zip);

	return anim;
}

/************************************************************************
 return nullptr if error
 ************************************************************************/
static anim_t * libav_load(const char * filename)
{
	anim_t * anim = nullptr;
	anim_t * ret = nullptr;
	AVFormatContext *pFormatCtx = nullptr;
	unsigned int i = 0U;
	int videoStream = 0;
	AVCodecContext *pCodecCtx = nullptr;
	AVCodec *pCodec = nullptr;
	AVFrame *pDecodedFrame = nullptr;
	AVFrame *pFrameRGBA = nullptr;
	struct SwsContext * pSwsCtx = nullptr;
	AVPacket packet;
	int delay = 0;

	anim = (anim_t*) malloc(sizeof(anim_t));
	memset(anim, 0, sizeof(anim_t));

	// Register all formats and codecs
	av_register_all();

	// Open video file
	if (avformat_open_input(&pFormatCtx, filename, nullptr, nullptr) != 0)
	{
		goto error;
	}

	if (avformat_find_stream_info(pFormatCtx, nullptr) < 0)
	{
		goto error;
	}

	// Find the first video stream
	videoStream = -1;
	for (i = 0; i < pFormatCtx->nb_streams; i++)
	{
		if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO)
		{
			videoStream = i;
			// total stream duration (in nanosecond) / number of image in the stream / 1000 (to get milliseconds
			delay = pFormatCtx->duration / pFormatCtx->streams[i]->duration / 1000;
			// If the above doesn't work try with frame_rate :
			//delay = pFormatCtx->streams[i]->r_frame_rate;

			anim->tex = nullptr;
			anim->delay = nullptr;
			break;
		}
	}

	if (videoStream == -1)
	{
		goto error;
	}

	// Get a pointer to the codec context for the video stream
	pCodecCtx = pFormatCtx->streams[videoStream]->codec;

	// Find the decoder for the video stream
	pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
	if (pCodec == nullptr)
	{
		goto error;
	}

	// Open codec
	if (avcodec_open2(pCodecCtx, pCodec, nullptr) < 0)
	{
		goto error;
	}

	// Allocate video frame
#if LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(55,28,1)
	pDecodedFrame = av_frame_alloc();
	pFrameRGBA = av_frame_alloc();
#else
	pDecodedFrame = avcodec_alloc_frame();
	pFrameRGBA = avcodec_alloc_frame();
#endif

	if (pDecodedFrame == nullptr)
	{
		goto error;
	}
	if (pFrameRGBA == nullptr)
	{
		goto error;
	}

	pFrameRGBA->format = AV_PIX_FMT_RGBA;
	pFrameRGBA->height = pCodecCtx->height;
	pFrameRGBA->width = pCodecCtx->width;
	av_frame_get_buffer(pFrameRGBA, 16);

	pSwsCtx = sws_getContext(pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt, pCodecCtx->width, pCodecCtx->height, AV_PIX_FMT_RGBA, SWS_BILINEAR,
			nullptr, nullptr, nullptr);

	if (pSwsCtx == nullptr)
	{
		goto error;
	}

	anim->w = pCodecCtx->width;
	anim->h = pCodecCtx->height;

// Read frames
	i = 0;
	while (av_read_frame(pFormatCtx, &packet) >= 0)
	{
		// Is this a packet from the video stream?
		if (packet.stream_index == videoStream)
		{
			// Decode video frame
			if (avcodec_send_packet(pCodecCtx, &packet) == 0)
			{
				if (avcodec_receive_frame(pCodecCtx, pDecodedFrame) == 0)
				{
					// Convert the image from its native format to RGBA
					sws_scale(pSwsCtx, (const uint8_t * const *) pDecodedFrame->data, pDecodedFrame->linesize, 0, pCodecCtx->height, pFrameRGBA->data,
							pFrameRGBA->linesize);

					anim->delay = (Uint32*) realloc(anim->delay, (i + 1) * sizeof(Uint32));
					anim->delay[i] = delay;
					anim->tex = (SDL_Texture**) realloc(anim->tex, (i + 1) * sizeof(SDL_Texture*));
					anim->tex[i] = SDL_CreateTexture(sdl_get_renderer(), SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STATIC, pCodecCtx->width,
							pCodecCtx->height);
					if (anim->tex[i] == nullptr)
					{
						//SDL_CreateTexture error
					}
					// Copy decoded bits to render texture
					if (SDL_UpdateTexture(anim->tex[i], nullptr, pFrameRGBA->data[0], pFrameRGBA->linesize[0]) < 0)
					{
						//SDL_UpdateTexture error
					}
					i++;
				}
			}
		}
		anim->num_frame = i;

		av_packet_unref(&packet);
	}

	ret = anim;

	error: if (ret == nullptr)
	{
		if (anim)
		{
			if (anim->tex)
			{
				free(anim->tex);
			}
			free(anim);
		}
	}

	if (pFrameRGBA)
	{
		av_free(pFrameRGBA);
	}

	if (pDecodedFrame)
	{
		av_free(pDecodedFrame);
	}

	if (pCodecCtx)
	{
		avcodec_close(pCodecCtx);
	}

	if (pFormatCtx)
	{
		avformat_close_input(&pFormatCtx);
	}

	return ret;
}

/************************************************************************
 ************************************************************************/
anim_t * anim_load(const char * filename)
{
	anim_t * ret;

	if (filename == nullptr)
	{
		return nullptr;
	}

	ret = giflib_load(filename);
	if (ret == nullptr)
	{
		ret = libpng_load(filename);
		if (ret == nullptr)
		{
			ret = libzip_load(filename);
			if (ret == nullptr)
			{
				ret = libav_load(filename);
			}
		}
	}

	if (ret != nullptr)
	{
		ret->total_duration = 0;
		int i;
		for (i = 0; i < ret->num_frame; i++)
		{
			ret->total_duration += ret->delay[i];
		}
	}

	return ret;
}

/************************************************************************
 color is RGBA
 ************************************************************************/
anim_t * anim_create_color(Uint32 width, Uint32 height, Uint32 color)
{
	anim_t * anim;
	SDL_Surface* surf;
	unsigned int i;
	Uint32 * to_fill;

	anim = (anim_t *) malloc(sizeof(anim_t));
	memset(anim, 0, sizeof(anim_t));

	anim->num_frame = 1;
	anim->tex = (SDL_Texture **) malloc(sizeof(SDL_Texture *) * anim->num_frame);
	anim->w = width;
	anim->h = height;
	anim->delay = (Uint32*) malloc(sizeof(Uint32) * anim->num_frame);
	anim->delay[0] = 0;

	surf = SDL_CreateRGBSurface(0, width, height, 32, 0xff000000, 0x00ff0000, 0x0000ff00, 0x000000ff);

	to_fill = (Uint32*) (surf->pixels);
	for (i = 0; i < width * height; i++)
	{
		to_fill[i] = color;
	}

	anim->tex[0] = SDL_CreateTextureFromSurface(sdl_get_renderer(), surf);
	SDL_FreeSurface(surf);

	return anim;
}

/************************************************************************
 Free memory used by an anim
 ************************************************************************/
void si_anim_free(anim_t * anim)
{
	int i;

	if (anim->tex)
	{
		for (i = 0; i < anim->num_frame; i++)
		{
			SDL_DestroyTexture(anim->tex[i]);
		}
		free(anim->tex);
	}

	free(anim);
}

#ifdef __cplusplus
}
#endif
