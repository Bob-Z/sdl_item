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
#include <memory>
#include <string>

#ifdef __cplusplus
extern "C"
{
#endif

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>

#ifdef __cplusplus
}
#endif

/*****************************************************************************/
std::unique_ptr<SiAnim> libav_load(const std::string & filePath)
{
	bool loaded = false;
	unsigned int i = 0;
	auto anim = std::make_unique<SiAnim>();
	struct SwsContext * swsCtx;
	int videoStream = -1;
	int delay = 0;
	const AVCodec *codec = nullptr;
	AVFrame *decodedFrame = nullptr;
	AVFrame *frameRgba = nullptr;
	AVCodecContext *codexCtx = nullptr;

	// Open video file
	AVFormatContext *formatCtx = nullptr;
	if (avformat_open_input(&formatCtx, filePath.c_str(), nullptr, nullptr) != 0)
	{
		goto error;
	}

	if (avformat_find_stream_info(formatCtx, nullptr) < 0)
	{
		goto error;
	}

	// Find the first video stream
	for (unsigned int i = 0; i < formatCtx->nb_streams; i++)
	{
		if (formatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
		{
			videoStream = i;
			// total stream duration (in nanosecond) / number of image in the stream / 1000 (to get milliseconds
			delay = formatCtx->duration / formatCtx->streams[i]->duration / 1000;
			// If the above doesn't work try with frame_rate :
			//delay = pFormatCtx->streams[i]->r_frame_rate;
			break;
		}
	}

	if (videoStream == -1)
	{
		goto error;
	}

	// Find the decoder for the video stream
	codec = avcodec_find_decoder(formatCtx->streams[videoStream]->codecpar->codec_id);
	if (codec == nullptr)
	{
		goto error;
	}

	// Get a codec context for the video stream
	codexCtx = avcodec_alloc_context3(codec);
	if (codexCtx == nullptr)
	{
		goto error;
	}

	if (avcodec_parameters_to_context(codexCtx, formatCtx->streams[videoStream]->codecpar) < 0)
	{
		goto error;
	}

	// Open codec
	if (avcodec_open2(codexCtx, codec, nullptr) < 0)
	{
		goto error;
	}

	// Allocate video frames
	decodedFrame = av_frame_alloc();
	frameRgba = av_frame_alloc();

	if (decodedFrame == nullptr || frameRgba == nullptr)
	{
		goto error;
	}

	frameRgba->format = AV_PIX_FMT_RGBA;
	frameRgba->height = codexCtx->height;
	frameRgba->width = codexCtx->width;
	av_frame_get_buffer(frameRgba, 16);

	swsCtx = sws_getContext(codexCtx->width, codexCtx->height, codexCtx->pix_fmt, codexCtx->width, codexCtx->height, AV_PIX_FMT_RGBA,
	SWS_BILINEAR, nullptr, nullptr, nullptr);

	if (swsCtx == nullptr)
	{
		goto error;
	}

	anim->setWidth(codexCtx->width);
	anim->setHeight(codexCtx->height);

// Read frames
	AVPacket packet;
	while (av_read_frame(formatCtx, &packet) >= 0)
	{
		// Is this a packet from the video stream?
		if (packet.stream_index == videoStream)
		{
			// Decode video frame
			if (avcodec_send_packet(codexCtx, &packet) == 0)
			{
				if (avcodec_receive_frame(codexCtx, decodedFrame) == 0)
				{
					// Convert the image from its native format to RGBA
					sws_scale(swsCtx, (const uint8_t * const *) decodedFrame->data, decodedFrame->linesize, 0, codexCtx->height, frameRgba->data,
							frameRgba->linesize);

					anim->pushDelay(delay);
					anim->pushTexture(
							SDL_CreateTexture(sdl_get_renderer(), SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STATIC, codexCtx->width, codexCtx->height));

					// Copy decoded bits to render texture
					if (SDL_UpdateTexture(anim->getTexture(i)->getTexture(), nullptr, frameRgba->data[0], frameRgba->linesize[0]) < 0)
					{
						//SDL_UpdateTexture error
					}
					i++;
				}
			}
		}

		av_packet_unref(&packet);
	}

	loaded = true;

	error:

	if (frameRgba)
	{
		av_frame_free(&frameRgba);
	}

	if (decodedFrame)
	{
		av_frame_free(&decodedFrame);
	}

	if (codexCtx)
	{
		avcodec_free_context(&codexCtx);
	}

	if (formatCtx)
	{
		avformat_close_input(&formatCtx);
	}

	return loaded ? std::move(anim) : nullptr;
}
