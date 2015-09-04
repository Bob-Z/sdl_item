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
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>

#include <gif_lib.h>
#include <png.h>

#define GIF_GCE 0xf9

#define DISPOSE_DO_NOT 1 /* Draw on top of previous image */
#define DISPOSE_BACKGROUND 2 /* Clean with the background color */
#define DISPOSE_PREVIOUS 3 /* Restore to previous content */

#define DEFAULT_DELAY 40

/************************************************************************
return NULL if error
http://www.imagemagick.org/Usage/anim_basics/#dispose
http://wwwcdf.pd.infn.it/libgif/gif89.txt
http://wwwcdf.pd.infn.it/libgif/gif_lib.html
************************************************************************/
static anim_t * giflib_load(SDL_Renderer * render, const char * filename)
{
	GifFileType * gif = NULL;
	int i = 0;
	int j = 0;
	int transparent = 0;
	unsigned char transparent_color = 0;
	int disposal = 0;
	int delay = 0;
	ColorMapObject * global_pal = NULL;
	ColorMapObject * pal = NULL;
	SDL_Surface* surf = NULL;
	SDL_Surface* prev_surf = NULL;
	int x = 0;
	int y = 0;
	int col = 0;
	int pix_index = 0;
	anim_t * anim = NULL;
	int render_width;
	int render_height;
	int frame_left = 0;
	int frame_top = 0;
	int frame_width = 0;
	int frame_height = 0;
	int allow_draw = 1;

	gif = DGifOpenFileName(filename);
	if(gif == NULL) {
		return NULL;
	}
	//Using giflib to decode
	DGifSlurp(gif);

	anim = malloc(sizeof(anim_t));
	memset(anim,0,sizeof(anim_t));

	anim->num_frame = gif->ImageCount;
	anim->tex = malloc(sizeof(SDL_Texture *) * anim->num_frame);
	anim->w = gif->SWidth;
	anim->h = gif->SHeight;
	anim->delay = malloc(sizeof(Uint32) * anim->num_frame);

	render_width = gif->SWidth;
	render_height = gif->SHeight;
	//bg_color = gif->SBackGroundColor;

	surf = SDL_CreateRGBSurface(0,render_width,render_height,32,0xff000000,0x00ff0000,0x0000ff00,0x000000ff);
	prev_surf = SDL_CreateRGBSurface(0,render_width,render_height,32,0xff000000,0x00ff0000,0x0000ff00,0x000000ff);

	global_pal = gif->SColorMap;
	pal = global_pal;

	/* Init with transparent background */
	memset(surf->pixels,0, render_height * render_width * 4 );

	for(i=0; i<gif->ImageCount; i++) {
		frame_left = gif->SavedImages[i].ImageDesc.Left;
		frame_top = gif->SavedImages[i].ImageDesc.Top;
		frame_width = gif->SavedImages[i].ImageDesc.Width;
		frame_height = gif->SavedImages[i].ImageDesc.Height;

		/* select palette */
		pal = global_pal;
		if( gif->SavedImages[i].ImageDesc.ColorMap ) {
			pal = gif->SavedImages[i].ImageDesc.ColorMap;
		}
		/* GCE */
		for(j=0; j<gif->SavedImages[i].ExtensionBlockCount; j++) {
			if(gif->SavedImages[i].ExtensionBlocks[j].Function == GIF_GCE) {
				transparent = gif->SavedImages[i].ExtensionBlocks[j].Bytes[0] & 0x01;
				disposal = (gif->SavedImages[i].ExtensionBlocks[j].Bytes[0] & 28)>>2;
				delay = (gif->SavedImages[i].ExtensionBlocks[j].Bytes[1] + gif->SavedImages[i].ExtensionBlocks[j].Bytes[2] * 256)*10;
				if(delay==0) {
					delay = DEFAULT_DELAY;
				}
				if(transparent) {
					transparent_color = gif->SavedImages[i].ExtensionBlocks[j].Bytes[3];
				}
			}
		}

		/* Save the current render if needed */
		if( disposal == DISPOSE_PREVIOUS ) {
			memcpy(prev_surf->pixels,surf->pixels, render_height * render_width * 4);
		}

		/* Fill surface buffer with raster bytes */
		if( allow_draw ) { // See DISPOSE_DO_NOT
			for(y=0; y<frame_height; y++) {
				for(x=0; x<frame_width; x++) {
					pix_index = ((x+frame_left) + ( (y+frame_top) *render_width))*4;
					col = gif->SavedImages[i].RasterBits[(x)+(y)*frame_width];
					if( col == transparent_color && transparent) {
						/* Transparent color means do not touch the render */
					} else {
						((char*)surf->pixels)[pix_index+3] = pal->Colors[col].Red;
						((char*)surf->pixels)[pix_index+2] = pal->Colors[col].Green;
						((char*)surf->pixels)[pix_index+1] = pal->Colors[col].Blue;
						((char*)surf->pixels)[pix_index+0] = 0xFF;
					}
				}
			}
		}

		anim->delay[i] = delay;
		anim->tex[i] = SDL_CreateTextureFromSurface(render,surf);

		/* Prepare next rendering depending of disposal */
		allow_draw = 1;
		switch( disposal ) {
		/* Do not touch render for next frame */
		case DISPOSE_DO_NOT:
			allow_draw = 0;
			break;
		case DISPOSE_BACKGROUND:
			/* Draw transparent color in frame */
			for(y=0; y<frame_height; y++) {
				for(x=0; x<frame_width; x++) {
					pix_index = ((x+frame_left)+((y+frame_top)*render_width))*4;
					((char*)surf->pixels)[pix_index+3] = 0;
					((char*)surf->pixels)[pix_index+2] = 0;
					((char*)surf->pixels)[pix_index+1] = 0;
					((char*)surf->pixels)[pix_index+0] = 0;
				}
			}
			break;
		case DISPOSE_PREVIOUS:
			/* Restore previous render in frame*/
			memcpy(surf->pixels,prev_surf->pixels, render_height * render_width * 4);
			break;
		default:
			break;
		}
	}
	SDL_FreeSurface(surf);

	DGifCloseFile(gif);

	return anim;
}

/************************************************************************
return NULL if error
************************************************************************/
static anim_t * libpng_load(SDL_Renderer * render, const char * filename)
{
	FILE *fp = NULL;
	png_structp png_ptr = NULL;
	png_infop info_ptr = NULL;
	SDL_Surface* surf = NULL;
	anim_t * anim = NULL;
	png_bytep *row_pointers = NULL;
	png_uint_32 width = 0;
	png_uint_32 height = 0;
	int bit_depth = 0;
	int color_type = 0;
	int i = 0;
	png_byte magic[8];

	/* open image file */
	fp = fopen (filename, "rb");
	if (!fp) {
		fprintf (stderr, "error: couldn't open \"%s\"!\n", filename);
		return NULL;
	}

	/* read magic number */
	fread (magic, 1, sizeof (magic), fp);

	/* check for valid magic number */
	if (!png_check_sig (magic, sizeof (magic))) {
		return NULL;
	}

	/* create a png read struct */
	png_ptr = png_create_read_struct
			  (PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!png_ptr) {
		return NULL;
	}

	/* create a png info struct */
	info_ptr = png_create_info_struct (png_ptr);
	if (!info_ptr) {
		png_destroy_read_struct (&png_ptr, NULL, NULL);
		return NULL;
	}

	//wlog(LOGDEBUG,"Using libpng to decode %s",filename);

	/* set error handling */
	if (setjmp(png_ptr->jmpbuf)) {
		png_read_destroy(png_ptr, info_ptr, (png_info *)0);
		fclose(fp);
		free(png_ptr);
		free(info_ptr);
		/* If we get here, we had a problem reading the file */
		return NULL;
	}

	/* setup libpng for using standard C fread() function
	   with our FILE pointer */
	png_init_io (png_ptr, fp);

	/* tell libpng that we have already read the magic number */
	png_set_sig_bytes (png_ptr, sizeof (magic));

	/* read the file information */
	png_read_info(png_ptr, info_ptr);

	/* get some usefull information from header */
	bit_depth = png_get_bit_depth (png_ptr, info_ptr);
	color_type = png_get_color_type (png_ptr, info_ptr);

	/* convert index color images to RGB images */
	if (color_type == PNG_COLOR_TYPE_PALETTE) {
		png_set_palette_to_rgb (png_ptr);
	}

	/* convert 1-2-4 bits grayscale images to 8 bits
	   grayscale. */
	if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8) {
		png_set_gray_1_2_4_to_8 (png_ptr);
	}

	if (png_get_valid (png_ptr, info_ptr, PNG_INFO_tRNS)) {
		png_set_tRNS_to_alpha (png_ptr);
	}

	if (bit_depth == 16) {
		png_set_strip_16 (png_ptr);
	} else if (bit_depth < 8) {
		png_set_packing (png_ptr);
	}

	if ( !(color_type &= PNG_COLOR_MASK_ALPHA) ) {
		png_set_filler(png_ptr, 0xff, PNG_FILLER_AFTER);
	}

	/* optional call to update the info structure */
	png_read_update_info(png_ptr, info_ptr);

	/* retrieve updated information */
	png_get_IHDR (png_ptr, info_ptr,
				  &width,&height,
				  &bit_depth, &color_type,
				  NULL, NULL, NULL);

	//wlog(LOGDEBUG,"size: %dx%d bit_depth: %d, type: %d",width,height,bit_depth,color_type);
	/* allocate the memory to hold the image using the fields
	   of png_info. */
	anim = malloc(sizeof(anim_t));
	memset(anim,0,sizeof(anim_t));

	anim->num_frame = 1;
	anim->tex = malloc(sizeof(SDL_Texture *) * anim->num_frame);
	anim->w = width;
	anim->h = height;
	anim->delay = malloc(sizeof(Uint32) * anim->num_frame);
	anim->delay[0] = 0;

	surf = SDL_CreateRGBSurface(0,width,height,32,0x000000ff,0x0000ff00,0x00ff0000,0xff000000);
	memset(surf->pixels,0,width*height*sizeof(Uint32));
	row_pointers = (png_bytep *)malloc (sizeof (png_bytep) * height);

	for (i = 0; i < height; i++) {
		row_pointers[height-i-1] = (png_bytep)(surf->pixels +
											   ((height - (i + 1)) * width * sizeof(Uint32)));
	}

	/* the easiest way to read the image */
	png_read_image(png_ptr, row_pointers);

	free(row_pointers);

	/* read the rest of the file, getting any additional chunks
	   in info_ptr */
	png_read_end(png_ptr, info_ptr);

	/* clean up after the read, and free any memory allocated */
	png_destroy_read_struct (&png_ptr, &info_ptr, NULL);

	/* close the file */
	fclose(fp);

	anim->tex[0] = SDL_CreateTextureFromSurface(render,surf);
	SDL_FreeSurface(surf);

	return anim;
}

/************************************************************************
return NULL if error
************************************************************************/
static anim_t * libav_load(SDL_Renderer * render, const char * filename)
{
	anim_t * anim = NULL;
	anim_t * ret = NULL;
	AVFormatContext *pFormatCtx = NULL;
	int i = 0;
	int videoStream = 0;
	AVCodecContext *pCodecCtx = NULL;
	AVCodec *pCodec = NULL;
	AVFrame *pFrame = NULL;
	AVFrame *pFrameRGB = NULL;
	struct SwsContext * pSwsCtx = NULL;
	AVPacket packet;
	int frameFinished = 0;
	int numBytes = 0;
	uint8_t *buffer = NULL;
	int delay = 0;

	anim = malloc(sizeof(anim_t));
	memset(anim,0,sizeof(anim_t));

	// Register all formats and codecs
	av_register_all();

	// Open video file
	if (avformat_open_input(&pFormatCtx, filename, NULL, NULL) != 0) {
		//Cannot open file
		goto error;
	}

	// Retrieve stream information
	if (avformat_find_stream_info(pFormatCtx, NULL) < 0) {
		//Cannot find stream information
		goto error;
	}

	//wlog(LOGDEBUG,"Loading %d streams in %s",pFormatCtx->nb_streams,filename);
	// Find the first video stream
	videoStream = -1;
	for (i = 0; i < pFormatCtx->nb_streams; i++)
		if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
			videoStream = i;
			/* total stream duration (in nanosecond) / number of image in the stream / 1000 (to get milliseconds */
			delay = pFormatCtx->duration / pFormatCtx->streams[i]->duration / 1000;
			/* If the above doesn't work try with frame_rate :
			pFormatCtx->streams[i]->r_frame_rate
			*/
			anim->num_frame = pFormatCtx->streams[i]->duration;
			anim->tex = (SDL_Texture**)malloc(anim->num_frame * sizeof(SDL_Texture*));
			anim->delay = (Uint32*)malloc(anim->num_frame * sizeof(Uint32));
			break;
		}

	if (videoStream == -1) {
		//Didn't find a video stream
		goto error;
	}

	// Get a pointer to the codec context for the video stream
	pCodecCtx = pFormatCtx->streams[videoStream]->codec;

	// Find the decoder for the video stream
	pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
	if (pCodec == NULL) {
		//Unsupported codec
		goto error;
	}

	// Open codec
	if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0) {
		//Could not open codec
		goto error;
	}

	// Allocate video frame
#if LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(55,28,1)
	//Allocate a video frame
	pFrame = av_frame_alloc();

	// Allocate an AVFrame structure
	pFrameRGB = av_frame_alloc();
#else
	pFrame = avcodec_alloc_frame();
	pFrameRGB = avcodec_alloc_frame();
#endif
	if (pFrame == NULL) {
		goto error;
	}
	if (pFrameRGB == NULL) {
		goto error;
	}

	// Determine required buffer size and allocate buffer
	numBytes = avpicture_get_size(PIX_FMT_RGBA, pCodecCtx->width,
								  pCodecCtx->height);
	buffer = (uint8_t *) av_malloc(numBytes * sizeof(uint8_t));

	// Assign appropriate parts of buffer to image planes in pFrameRGB
	// Note that pFrameRGB is an AVFrame, but AVFrame is a superset
	// of AVPicture
	avpicture_fill((AVPicture *) pFrameRGB, buffer, PIX_FMT_RGBA,
				   pCodecCtx->width, pCodecCtx->height);

	pSwsCtx = sws_getContext(pCodecCtx->width,
							 pCodecCtx->height, pCodecCtx->pix_fmt,
							 pCodecCtx->width, pCodecCtx->height,
							 PIX_FMT_RGBA, SWS_BILINEAR, NULL, NULL, NULL);

	anim->w = pCodecCtx->width;
	anim->h = pCodecCtx->height;

	if (pSwsCtx == NULL) {
		//Cannot initialize sws context
		goto error;
	}

	// Read frames
	i = 0;
	while (av_read_frame(pFormatCtx, &packet) >= 0) {
		// Is this a packet from the video stream?
		if (packet.stream_index == videoStream) {
			// Decode video frame
			avcodec_decode_video2(pCodecCtx, pFrame, &frameFinished, &packet);
			// Did we get a video frame?
			if (frameFinished) {
				// Convert the image from its native format to ABGR
				sws_scale(pSwsCtx,
						  (const uint8_t * const *) pFrame->data,
						  pFrame->linesize, 0, pCodecCtx->height,
						  pFrameRGB->data,
						  pFrameRGB->linesize);

				anim->delay[i] = delay;
				anim->tex[i] = SDL_CreateTexture(render, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STATIC, pCodecCtx->width,pCodecCtx->height);
				if( anim->tex[i] == NULL ) {
					//SDL_CreateTexture error
				}
				/* Copy decoded bits to render texture */
				if (SDL_UpdateTexture(anim->tex[i],NULL,pFrameRGB->data[0],pFrameRGB->linesize[0]) < 0) {
					//SDL_UpdateTexture error
				}
				i++;
			}
		}

		// Free the packet that was allocated by av_read_frame
		av_free_packet(&packet);
	}

	ret = anim;

error:
	if(ret == NULL) {
		if(anim) {
			if(anim->tex) {
				free(anim->tex);
			}
			free(anim);
		}
	}

	// Free the RGB image
	if(buffer) {
		av_free(buffer);
	}

	if(pFrameRGB) {
		av_free(pFrameRGB);
	}

	// Free the YUV frame
	if(pFrame) {
		av_free(pFrame);
	}

	// Close the codec
	if(pCodecCtx) {
		avcodec_close(pCodecCtx);
	}

	// Close the video file
	if(pFormatCtx) {
		avformat_close_input(&pFormatCtx);
	}

	return ret;
}

/************************************************************************
************************************************************************/
void anim_reset_anim(anim_t * anim)
{
	anim->prev_time=0;
	anim->current_frame=0;
}

/************************************************************************
************************************************************************/
anim_t * anim_load(SDL_Renderer * render, const char * filename)
{
	anim_t * ret;

	ret = giflib_load(render, filename);
	if(ret != NULL) {
		return ret;
	}

	ret = libpng_load(render, filename);
	if(ret != NULL) {
		return ret;
	}

	ret = libav_load(render, filename);

	return ret;
}

/************************************************************************
color is RGBA
************************************************************************/
anim_t * anim_create_color(SDL_Renderer * render, Uint32 width, Uint32 height, Uint32 color)
{
	anim_t * anim;
	SDL_Surface* surf;
	int i;
	Uint32 * to_fill;

	anim = malloc(sizeof(anim_t));
	memset(anim,0,sizeof(anim_t));

	anim->num_frame = 1;
	anim->tex = malloc(sizeof(SDL_Texture *) * anim->num_frame);
	anim->w = width;
	anim->h = height;
	anim->delay = malloc(sizeof(Uint32) * anim->num_frame);
	anim->delay[0] = 0;

	surf = SDL_CreateRGBSurface(0,width,height,32,0xff000000,0x00ff0000,0x0000ff00,0x000000ff);

	to_fill = surf->pixels;
	for(i=0; i<width*height; i++) {
		to_fill[i] = color;
	}

	anim->tex[0] = SDL_CreateTextureFromSurface(render,surf);
	SDL_FreeSurface(surf);

	return anim;
}

/************************************************************************
Free memory used by an anim
************************************************************************/
void si_anim_free(anim_t * anim)
{
	int i;

	if(anim->tex) {
		for(i=0; i<anim->num_frame; i++) {
			SDL_DestroyTexture(anim->tex[i]);
		}
		free(anim->tex);
	}

	free(anim);
}

