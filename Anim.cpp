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
#include "sdl.h"

/*****************************************************************************/
Anim::Anim() :
		m_textureArray(), m_width(0), m_height(0), m_delayArray(), m_totalDuration(0U)
{
}

/*****************************************************************************/
Anim::~Anim()
{
}

/*****************************************************************************/
int Anim::getHeight() const
{
	return m_height;
}

/*****************************************************************************/
void Anim::setHeight(int height)
{
	m_height = height;
}

/*****************************************************************************/
int Anim::getWidth() const
{
	return m_width;
}

/*****************************************************************************/
void Anim::setWidth(int width)
{
	m_width = width;
}

/*****************************************************************************/
const std::vector<std::shared_ptr<Texture>>& Anim::getTextureArray() const
{
	return m_textureArray;
}

/*****************************************************************************/
void Anim::pushTexture(SDL_Texture* texture)
{
	m_textureArray.push_back(std::make_shared<Texture>(texture));

	Uint32 format = 0;
	int access = 0;
	int width = 0;
	int height = 0;

	SDL_QueryTexture(texture, &format, &access, &width, &height);

	if (m_width < width)
	{
		m_width = width;
	}
	if (m_height < height)
	{
		m_height = height;
	}
}

/*****************************************************************************/
std::shared_ptr<Texture> Anim::getTexture(const int index) const
{
	return m_textureArray[index];
}

/*****************************************************************************/
Uint32 Anim::getTotalDuration() const
{
	return m_totalDuration;
}

/*****************************************************************************/
void Anim::setTotalDuration(Uint32 totalDuration)
{
	m_totalDuration = totalDuration;
}

/*****************************************************************************/
int Anim::getFrameQty() const
{
	return m_textureArray.size();
}

/*****************************************************************************/
const std::vector<Uint32>& Anim::getDelayArray() const
{
	return m_delayArray;
}

/*****************************************************************************/
void Anim::setDelayArray(const std::vector<Uint32>& delayArray)
{
	m_delayArray = delayArray;
}

/*****************************************************************************/
void Anim::pushDelay(const Uint32 delay)
{
	m_delayArray.push_back(delay);
}
