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

#include "SiMouseEvent.h"

SiMouseEvent::SiMouseEvent()
{
	// TODO Auto-generated constructor stub

}

SiMouseEvent::~SiMouseEvent()
{
	// TODO Auto-generated destructor stub
}

std::function<void()> SiMouseEvent::getCallBack() const
{
	return callBack;
}

void SiMouseEvent::setCallBack(std::function<void()> callBack)
{
	this->callBack = callBack;
}

Uint32 SiMouseEvent::getEventType() const
{
	return eventType;
}

void SiMouseEvent::setEventType(const Uint32 eventType)
{
	this->eventType = eventType;
}
