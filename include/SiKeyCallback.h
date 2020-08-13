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

#ifndef SDL_ITEM_INCLUDE_SIKEYCALLBACK_H_
#define SDL_ITEM_INCLUDE_SIKEYCALLBACK_H_

#include <SDL2/SDL.h>
#include <functional>

class SiKeyCallback
{
public:
	SiKeyCallback();
	virtual ~SiKeyCallback();

	SDL_Scancode getCode() const;
	void setCode(SDL_Scancode code);

	const std::function<void()>& getDownCallBack() const;
	void setDownCallBack(const std::function<void()>& downCallBack);

	const std::function<void()>& getUpCallBack() const;
	void setUpCallBack(const std::function<void()>& upCallBack);

private:
	SDL_Scancode code;
	std::function<void()> downCallBack;
	std::function<void()> upCallBack;
};

#endif /* SDL_ITEM_INCLUDE_SIKEYCALLBACK_H_ */
