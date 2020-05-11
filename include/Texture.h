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

#ifndef SDL_ITEM_TEXTURE_H_
#define SDL_ITEM_TEXTURE_H_

struct SDL_Texture;

class Texture
{
public:
	Texture(SDL_Texture * texture);
	virtual ~Texture();

	SDL_Texture* getTexture();

private:
	SDL_Texture * m_texture;
};

#endif /* SDL_ITEM_TEXTURE_H_ */
