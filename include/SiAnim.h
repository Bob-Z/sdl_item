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

#ifndef SDL_ITEM_ANIM_H_
#define SDL_ITEM_ANIM_H_

#include <memory>
#include <SDL.h>
#include <SiTexture.h>
#include <vector>

class SiAnim
{
public:
	SiAnim();
	virtual ~SiAnim();

	int getHeight() const;
	void setHeight(int height);

	int getWidth() const;
	void setWidth(int width);

	const std::vector<std::shared_ptr<SiTexture>>& getTextureArray() const;
	void pushTexture(SDL_Texture*);
	std::shared_ptr<SiTexture> getTexture(const int index) const;

	Uint32 getTotalDuration() const;
	void setTotalDuration(Uint32 totalDuration);

	int getFrameQty() const;

	const std::vector<Uint32>& getDelayArray() const;
	void setDelayArray(const std::vector<Uint32>& delayArray);
	void pushDelay(const Uint32 delay);

private:
	std::vector<std::shared_ptr<SiTexture>> m_textureArray;
	int m_width;
	int m_height;
	std::vector<Uint32> m_delayArray; //delay between each frame in millisecond
	Uint32 m_totalDuration;
};

#endif /* SDL_ITEM_ANIM_H_ */
