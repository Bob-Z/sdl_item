/*
 World of Gnome is a 2D multiplayer role playing game.
 Copyright (C) 2019 carabobz@gmail.com

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

#include <SiKeyCallback.h>

SiKeyCallback::SiKeyCallback()
{
	// TODO Auto-generated constructor stub

}

SiKeyCallback::~SiKeyCallback()
{
	// TODO Auto-generated destructor stub
}

SDL_Scancode SiKeyCallback::getCode() const
{
	return code;
}

void SiKeyCallback::setCode(SDL_Scancode code)
{
	this->code = code;
}

const std::function<void()>& SiKeyCallback::getDownCallBack() const
{
	return downCallBack;
}

void SiKeyCallback::setDownCallBack(const std::function<void()>& downCallBack)
{
	this->downCallBack = downCallBack;
}

const std::function<void()>& SiKeyCallback::getUpCallBack() const
{
	return upCallBack;
}

void SiKeyCallback::setUpCallBack(const std::function<void()>& upCallBack)
{
	this->upCallBack = upCallBack;
}
