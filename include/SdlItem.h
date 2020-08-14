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

#ifndef SDL_ITEM_SDLITEM_H_
#define SDL_ITEM_SDLITEM_H_

#include <functional>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL.h>
#include <string>
#include <vector>

class SdlItem
{
public:
	enum class Layout
	{
		TOP_LEFT, CENTER
	};

	SdlItem();
	virtual ~SdlItem();

	void setPos(const int x, const int y);
	void setShape(const int width, const int height);

	const std::vector<SiAnim*>& getAnim() const;
	void setAnim(const std::vector<SiAnim*>& animArray);
	void setAnim(SiAnim *anim);
	void pushAnim(SiAnim * const anim);
	void clearAnim();

	const std::vector<SiAnim*>& getAnimClick() const;
	void setAnimClick(const std::vector<SiAnim*>& animClick);
	void clearAnimClick();

	const std::vector<SiAnim*>& getAnimOver() const;
	void setAnimOver(const std::vector<SiAnim*>& animOver);
	void setAnimOver(SiAnim *animOver);
	void clearAnimOver();

	const std::vector<SiAnim*>& getDefaultAnimClick() const;
	void setDefaultAnimClick(const std::vector<SiAnim*>& defaultAnimClick);

	const std::vector<SiAnim*>& getDefaultAnimOver() const;
	void setDefaultAnimOver(const std::vector<SiAnim*>& defaultAnimOver);

	Layout getLayout() const;
	void setLayout(Layout layout);

	const SDL_Rect& getRect() const;
	void setRect(const SDL_Rect& rect);
	void setRectX(const int x);
	void setRectY(const int y);

	double getAngle() const;
	void setAngle(double angle);

	SDL_RendererFlip getFlip() const;
	void setFlip(SDL_RendererFlip flip);

	double getZoomX() const;
	void setZoomX(double zoomX);

	double getZoomY() const;
	void setZoomY(double zoomY);

	Uint32 getAnimStartTick() const;
	void setAnimStartTick(Uint32 animStartTick);

	bool isOverlay() const;
	void setOverlay(bool overlay);

	bool isAnimLoop() const;
	void setAnimLoop(bool animLoop);

	const std::function<void()>& getClickLeftCb() const;
	void setClickLeftCb(const std::function<void()>& callBack);

	const std::function<void()>& getClickRightCb() const;
	void setClickRightCb(const std::function<void()>& clickRightCb);

	const std::function<void()>& getDoubleClickLeftCb() const;
	void setDoubleClickLeftCb(const std::function<void()>& doubleClickLeftCb);

	const std::function<void()>& getDoubleClickRightCb() const;
	void setDoubleClickRightCb(const std::function<void()>& doubleClickRightCb);

	const std::function<void(int x, int y)>& getOverCb() const;
	void setOverCb(const std::function<void(int x, int y)>& overCb);

	const std::function<void()>& getWheelDownCb() const;
	void setWheelDownCb(const std::function<void()>& wheelDownCb);

	const std::function<void()>& getWheelUpCb() const;
	void setWheelUpCb(const std::function<void()>& wheelUpCb);

	bool isEditable() const;
	void setEditable(bool isEditable);

	SDL_Texture* getTextTexture() const;
	void setTextTexture(SDL_Texture* textTexture);

	const std::string& getText() const;
	void setText(const std::string& text);
	void addToText(const std::string& text);
	void removeFromText(const int quantity);

	TTF_Font* getFont() const;
	void setFont(TTF_Font* font);

	Uint32 getBackGroudColor() const;
	void setBackGroudColor(Uint32 backGroudColor);

	const std::function<void(std::string)>& getEditCb() const;
	void setEditCb(const std::function<void(std::string)>& editCb);

	int getUser1() const;
	void setUser1(int user1);

	int getUser2() const;
	void setUser2(int user2);

	const std::string getUserString() const;
	void setUserString(const std::string & userString);

	const void* getUserPtr() const;
	void setUserPtr(const void* userPtr);

	bool isClicked() const;
	void setClicked(bool clicked);

private:
	SDL_Rect m_rect; // Current coordinate/size in pixels
	Uint32 m_animStartTick;	// Tick from when animation will be calculated
	SDL_RendererFlip m_flip;
	double m_angle;
	double m_zoomX;
	double m_zoomY;
	int m_user1;	// User defined
	int m_user2;	// User defined
	bool m_overlay;
	std::vector<SiAnim*> m_animArray;         //default sprite
	std::vector<SiAnim*> m_animOver;  //is set to default_anim_over, when needed (i.e. mouse over this item)
	std::vector<SiAnim*> m_defaultAnimOver;
	std::vector<SiAnim*> m_animClick; //is set to default_anim_click, when needed (i.e. click on this item)
	std::vector<SiAnim*> m_defaultAnimClick;

	Layout m_layout;	// How to display array of anim (default is top-left)
	bool m_animLoop;
	bool m_clicked;
	std::function<void()> m_clickLeftCb;
	std::function<void()> m_clickRightCb;
	std::function<void()> m_doubleClickLeftCb;
	std::function<void()> m_doubleClickRightCb;
	std::function<void()> m_wheelUpCb;
	std::function<void()> m_wheelDownCb;
	std::function<void(int x, int y)> m_overCb;
	std::string m_text;		// string centered on item
	Uint32 m_backGroudColor;	// Background color RGBA
	TTF_Font * m_font;
	SDL_Texture * m_textTexture;
	bool m_editable;
	std::function<void(std::string)> m_editCb;

	const void * m_userPtr;
	std::string m_userString;
};

#endif /* SDL_ITEM_SDLITEM_H_ */
