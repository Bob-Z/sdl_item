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
#include "SdlItem.h"

/*****************************************************************************/
SdlItem::SdlItem() :
		m_rect(
		{ -1, -1, 0, 0 }), m_animStartTick(0U), m_flip(SDL_FLIP_NONE), m_angle(0.0), m_zoomX(1.0), m_zoomY(1.0), m_user1(0), m_user2(0), m_overlay(false), m_animArray(), m_animOver(), m_defaultAnimOver(), m_animClick(), m_defaultAnimClick(), m_layout(
				Layout::TOP_LEFT), m_animLoop(true), m_clicked(false), m_clickLeftCb(), m_clickRightCb(), m_doubleClickLeftCb(), m_doubleClickRightCb(), m_wheelUpCb(), m_wheelDownCb(), m_overCb(), m_text(), m_backGroudColor(
				0U), m_font(nullptr), m_textTexture(nullptr), m_editable(false), m_editCb(), m_userPtr(nullptr), m_user1Ptr(nullptr)
{
}

/*****************************************************************************/
SdlItem::~SdlItem()
{
	if (m_textTexture != nullptr)
	{
		SDL_DestroyTexture(m_textTexture);
	}
}

/*****************************************************************************/
void SdlItem::setPos(const int x, const int y)
{
	m_rect.x = x;
	m_rect.y = y;
}

/*****************************************************************************/
void SdlItem::setShape(const int width, const int height)
{
	m_rect.w = width;
	m_rect.h = height;
}

/*****************************************************************************/
const std::vector<Anim*>& SdlItem::getAnim() const
{
	return m_animArray;
}

/*****************************************************************************/
void SdlItem::setAnim(const std::vector<Anim*>& animArray)
{
	m_animArray = animArray;

	for (auto && anim : animArray)
	{
		if (anim->getWidth() > m_rect.w)
		{
			m_rect.w = anim->getWidth();
		}

		if (anim->getHeight() > m_rect.h)
		{
			m_rect.h = anim->getHeight();
		}
	}
}

/*****************************************************************************/
void SdlItem::setAnim(Anim *anim)
{
	std::vector<Anim*> animArray;
	animArray.push_back(anim);

	m_animArray = animArray;

	setShape(anim->getWidth(), anim->getHeight());
}

/*****************************************************************************/
void SdlItem::pushAnim(Anim * const anim)
{
	int maxWidth = 0;
	int maxHeight = 0;

	maxWidth = anim->getWidth();
	maxHeight = anim->getHeight();

	m_animArray.push_back(anim);

	if (m_text.size() != 0)
	{
		int width = 0;
		int height = 0;
		sdl_get_string_size(m_font, m_text, &width, &height);
		if (width > maxWidth)
		{
			maxWidth = width;
		}
		if (height > maxHeight)
		{
			maxHeight = height;
		}
	}

	if (m_rect.w < maxWidth)
	{
		m_rect.w = maxWidth;
	}
	if (m_rect.h < maxHeight)
	{
		m_rect.h = maxHeight;
	}
}

/*****************************************************************************/
void SdlItem::clearAnim()
{
	m_animArray.clear();
}

/*****************************************************************************/
const std::vector<Anim*>& SdlItem::getAnimClick() const
{
	return m_animClick;
}

/*****************************************************************************/
void SdlItem::setAnimClick(const std::vector<Anim*>& animClick)
{
	m_animClick = animClick;
}

/*****************************************************************************/
void SdlItem::clearAnimClick()
{
	m_animClick.clear();
}

/*****************************************************************************/
const std::vector<Anim*>& SdlItem::getAnimOver() const
{
	return m_animOver;
}

/*****************************************************************************/
void SdlItem::setAnimOver(const std::vector<Anim*>& animOver)
{
	m_animOver = animOver;
}

/*****************************************************************************/
void SdlItem::setAnimOver(Anim *animOver)
{
	std::vector<Anim*> animArray;
	animArray.push_back(animOver);

	m_animOver = animArray;
}

/*****************************************************************************/
void SdlItem::clearAnimOver()
{
	m_animOver.clear();
}

/*****************************************************************************/
const std::vector<Anim*>& SdlItem::getDefaultAnimClick() const
{
	return m_defaultAnimClick;
}

/*****************************************************************************/
void SdlItem::setDefaultAnimClick(const std::vector<Anim*>& defaultAnimClick)
{
	m_defaultAnimClick = defaultAnimClick;
}

/*****************************************************************************/
const std::vector<Anim*>& SdlItem::getDefaultAnimOver() const
{
	return m_defaultAnimOver;
}

/*****************************************************************************/
void SdlItem::setDefaultAnimOver(const std::vector<Anim*>& defaultAnimOver)
{
	m_defaultAnimOver = defaultAnimOver;
}

/*****************************************************************************/
SdlItem::Layout SdlItem::getLayout() const
{
	return m_layout;
}

/*****************************************************************************/
void SdlItem::setLayout(const SdlItem::Layout layout)
{
	m_layout = layout;
}

/*****************************************************************************/
const SDL_Rect& SdlItem::getRect() const
{
	return m_rect;
}

/*****************************************************************************/
void SdlItem::setRect(const SDL_Rect& rect)
{
	m_rect = rect;
}

/*****************************************************************************/
void SdlItem::setRectX(const int x)
{
	m_rect.x = x;
}

/*****************************************************************************/
void SdlItem::setRectY(const int y)
{
	m_rect.y = y;
}

/*****************************************************************************/
double SdlItem::getAngle() const
{
	return m_angle;
}

/*****************************************************************************/
void SdlItem::setAngle(double angle)
{
	m_angle = angle;
}

/*****************************************************************************/
SDL_RendererFlip SdlItem::getFlip() const
{
	return m_flip;
}

/*****************************************************************************/
void SdlItem::setFlip(SDL_RendererFlip flip)
{
	m_flip = flip;
}

/*****************************************************************************/
double SdlItem::getZoomX() const
{
	return m_zoomX;
}

/*****************************************************************************/
void SdlItem::setZoomX(double zoomX)
{
	m_zoomX = zoomX;
}

/*****************************************************************************/
double SdlItem::getZoomY() const
{
	return m_zoomY;
}

/*****************************************************************************/
void SdlItem::setZoomY(double zoomY)
{
	m_zoomY = zoomY;
}

/*****************************************************************************/
Uint32 SdlItem::getAnimStartTick() const
{
	return m_animStartTick;
}

/*****************************************************************************/
void SdlItem::setAnimStartTick(Uint32 animStartTick)
{
	m_animStartTick = animStartTick;
}

/*****************************************************************************/
bool SdlItem::isOverlay() const
{
	return m_overlay;
}

/*****************************************************************************/
void SdlItem::setOverlay(bool overlay)
{
	m_overlay = overlay;
}

/*****************************************************************************/
bool SdlItem::isAnimLoop() const
{
	return m_animLoop;
}

/*****************************************************************************/
void SdlItem::setAnimLoop(bool animLoop)
{
	m_animLoop = animLoop;
}

/*****************************************************************************/
const std::function<void()>& SdlItem::getClickLeftCb() const
{
	return m_clickLeftCb;
}

/*****************************************************************************/
void SdlItem::setClickLeftCb(const std::function<void()>& callBack)
{
	m_clickLeftCb = callBack;
}

/*****************************************************************************/
const std::function<void()>& SdlItem::getClickRightCb() const
{
	return m_clickRightCb;
}

/*****************************************************************************/
void SdlItem::setClickRightCb(const std::function<void()>& clickRightCb)
{
	m_clickRightCb = clickRightCb;
}

/*****************************************************************************/
const std::function<void()>& SdlItem::getDoubleClickLeftCb() const
{
	return m_doubleClickLeftCb;
}

/*****************************************************************************/
void SdlItem::setDoubleClickLeftCb(const std::function<void()>& doubleClickLeftCb)
{
	m_doubleClickLeftCb = doubleClickLeftCb;
}

/*****************************************************************************/
const std::function<void()>& SdlItem::getDoubleClickRightCb() const
{
	return m_doubleClickRightCb;
}

/*****************************************************************************/
void SdlItem::setDoubleClickRightCb(const std::function<void()>& doubleClickRightCb)
{
	m_doubleClickRightCb = doubleClickRightCb;
}

/*****************************************************************************/
const std::function<void(int x, int y)>& SdlItem::getOverCb() const
{
	return m_overCb;
}

/*****************************************************************************/
void SdlItem::setOverCb(const std::function<void(int x, int y)>& overCb)
{
	m_overCb = overCb;
}

/*****************************************************************************/
const std::function<void()>& SdlItem::getWheelDownCb() const
{
	return m_wheelDownCb;
}

/*****************************************************************************/
void SdlItem::setWheelDownCb(const std::function<void()>& wheelDownCb)
{
	m_wheelDownCb = wheelDownCb;
}

/*****************************************************************************/
const std::function<void()>& SdlItem::getWheelUpCb() const
{
	return m_wheelUpCb;
}

/*****************************************************************************/
void SdlItem::setWheelUpCb(const std::function<void()>& wheelUpCb)
{
	m_wheelUpCb = wheelUpCb;
}

/*****************************************************************************/
bool SdlItem::isEditable() const
{
	return m_editable;
}

/*****************************************************************************/
void SdlItem::setEditable(bool isEditable)
{
	m_editable = isEditable;
}

/*****************************************************************************/
SDL_Texture* SdlItem::getTextTexture() const
{
	return m_textTexture;
}

/*****************************************************************************/
void SdlItem::setTextTexture(SDL_Texture* textTexture)
{
	m_textTexture = textTexture;
}

/*****************************************************************************/
const std::string& SdlItem::getText() const
{
	return m_text;
}

/*****************************************************************************/
void SdlItem::setText(const std::string& text)
{
	m_text = text;
}

/*****************************************************************************/
void SdlItem::addToText(const std::string& text)
{
	m_text = m_text + text;
}

/*****************************************************************************/
void SdlItem::removeFromText(const int quantity)
{
	m_text = m_text.substr(0, m_text.size() - quantity);
}

/*****************************************************************************/
TTF_Font* SdlItem::getFont() const
{
	return m_font;
}

/*****************************************************************************/
void SdlItem::setFont(TTF_Font* font)
{
	m_font = font;
}

/*****************************************************************************/
Uint32 SdlItem::getBackGroudColor() const
{
	return m_backGroudColor;
}

/*****************************************************************************/
void SdlItem::setBackGroudColor(Uint32 backGroudColor)
{
	m_backGroudColor = backGroudColor;
}

/*****************************************************************************/
const std::function<void(std::string)>& SdlItem::getEditCb() const
{
	return m_editCb;
}

/*****************************************************************************/
void SdlItem::setEditCb(const std::function<void(std::string)>& editCb)
{
	m_editCb = editCb;
}

/*****************************************************************************/
int SdlItem::getUser1() const
{
	return m_user1;
}

/*****************************************************************************/
void SdlItem::setUser1(int user1)
{
	m_user1 = user1;
}

/*****************************************************************************/
int SdlItem::getUser2() const
{
	return m_user2;
}

/*****************************************************************************/
void SdlItem::setUser2(int user2)
{
	m_user2 = user2;
}

/*****************************************************************************/
const void* SdlItem::getUser1Ptr() const
{
	return m_user1Ptr;
}

/*****************************************************************************/
void SdlItem::setUser1Ptr(const void* user1Ptr)
{
	m_user1Ptr = user1Ptr;
}

/*****************************************************************************/
const void* SdlItem::getUserPtr() const
{
	return m_userPtr;
}

/*****************************************************************************/
void SdlItem::setUserPtr(const void* userPtr)
{
	m_userPtr = userPtr;
}

/*****************************************************************************/
bool SdlItem::isClicked() const
{
	return m_clicked;
}

/*****************************************************************************/
void SdlItem::setClicked(bool clicked)
{
	m_clicked = clicked;
}
