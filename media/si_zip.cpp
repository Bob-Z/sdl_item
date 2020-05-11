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

#include "Anim.h"
#include "si_png.h"
#include "stdio.h"
#include <algorithm>
#include <string>
#include <vector>

extern "C"
{
#include <zip.h>
}

const std::string ZIP_TIMING_FILE = "timing";
const std::string ZIP_TMP_FILE = "/tmp/si_tmp";

static constexpr int DEFAULT_DELAY_MS = 40;

/*****************************************************************************/
static void read_timing(const std::string & filePath, int timingQuantity, std::vector<Uint32> & delayArray)
{
	FILE * file = nullptr;
	file = fopen(filePath.c_str(), "r");

	int i = 0;
	for (i = 0; i < timingQuantity; i++)
	{
		Uint32 delay = 0;
		fscanf(file, "%d", &delay);
		delayArray.push_back(delay);
	}
	fclose(file);
}

/*****************************************************************************/
static int extract_zip(struct zip *fdZip, int index)
{
	struct zip_stat fileStat;
	struct zip_file *fileZip = nullptr;
	char * data;
	FILE *file_dest;

	zip_stat_index(fdZip, index, 0, &fileStat);

	fileZip = zip_fopen(fdZip, fileStat.name, ZIP_FL_UNCHANGED);
	if (fileZip == 0)
	{
		return -1;
	}

	data = (char*) malloc((size_t) (fileStat.size));
	if (zip_fread(fileZip, data, (size_t) (fileStat.size)) != (int64_t) fileStat.size)
	{
		free(data);
		zip_fclose(fileZip);
		return -1;
	}

	file_dest = fopen(ZIP_TMP_FILE.c_str(), "wb");
	if (file_dest == nullptr)
	{
		free(data);
		zip_fclose(fileZip);
		return -1;
	}

	if (fwrite(data, sizeof(char), (size_t) fileStat.size, file_dest) != fileStat.size)
	{
		fclose(file_dest);
		free(data);
		zip_fclose(fileZip);
		return -1;
	}

	fclose(file_dest);
	free(data);
	zip_fclose(fileZip);

	return 0;
}

/*****************************************************************************/
Anim * libzip_load(const std::string & filePath)
{
	int err = 0;
	struct zip * fdZip = zip_open(filePath.c_str(), ZIP_CHECKCONS, &err);

	if (err != ZIP_ER_OK)
	{
#if 0
		zip_error_to_str(buf_erreur, sizeof buf_erreur, err, errno);
		printf("Error %d : %s\n",err, buf_erreur);
#endif
		return nullptr;
	}

	if (fdZip == nullptr)
	{
		return nullptr;
	}

	const int fileQty = zip_get_num_files(fdZip);

	if (fileQty <= 0)
	{
		zip_close(fdZip);
		return nullptr;
	}

	// Remove timing file
	int animQty = fileQty - 1;

	Anim * anim = new Anim;

	for (int i = 0; i < animQty; i++)
	{
		anim->pushDelay(DEFAULT_DELAY_MS);
	}

	// Get zip archive filenames and sort them alphabetically
	std::vector<std::string> zipFileNameArray;

	for (int i = 0; i < fileQty; i++)
	{
		zipFileNameArray.push_back(std::string(zip_get_name(fdZip, i, ZIP_FL_UNCHANGED)));
	}

	std::sort(zipFileNameArray.begin(), zipFileNameArray.end());

	// Read file in archive and process them (either as PNG file or timing file
	int index = 0;

	for (auto && zipFileName : zipFileNameArray)
	{
		index = zip_name_locate(fdZip, zipFileName.c_str(), 0);
		if (index == -1)
		{
			continue;
		}

		// Create ZIP_TMP_FILE file
		if (extract_zip(fdZip, index) < 0)
		{
			continue;
		}

		std::vector<Uint32> delayArray;

		// timing file
		if (zipFileName == ZIP_TIMING_FILE)
		{
			read_timing(ZIP_TMP_FILE, fileQty - 1, delayArray);
			anim->setDelayArray(delayArray);
			continue;
		}

		// PNG file
		int width = 0;
		int height = 0;
		anim->pushTexture(libpng_load_texture(ZIP_TMP_FILE, &width, &height));
		anim->setWidth(width);
		anim->setHeight(height);
	}

	// Clean-up
	zip_close(fdZip);

	return anim;
}
