/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#ifndef DARKSEED2_GRAPHICALOBJECT_H
#define DARKSEED2_GRAPHICALOBJECT_H

#include "common/rect.h"
#include "common/array.h"

#include "engines/darkseed2/darkseed2.h"

namespace Common {
	class String;
}

namespace DarkSeed2 {

class Sprite;

/** Base class for a graphical object. */
class GraphicalObject {
public:
	GraphicalObject();
	virtual ~GraphicalObject();

	/** Get the object's area. */
	Common::Rect getArea() const;

	/** Move the object. */
	virtual void move(uint32 x, uint32 y);

	/** Redraw the object. */
	virtual void redraw(Sprite &sprite, Common::Rect area) = 0;

protected:
	Common::Rect _area; ///< The object's area.

	/** Clear the object's area. */
	void clearArea();
};

/** A text graphic. */
class TextObject : public GraphicalObject {
public:
	TextObject(const Common::String &text, uint32 x, uint32 y,
			byte color, uint32 maxWidth = 0);
	~TextObject();

	/** Redraw the object. */
	void redraw(Sprite &sprite, Common::Rect area);

	/** Recolor the text. */
	void recolor(byte color);

	/** Create a wrapped StringList out of the supplied string. */
	static uint32 wrap(const Common::String &string, Common::StringList &list, uint32 maxWidth);

private:
	Sprite *_sprite; ///< The text's sprite.

	byte _color; ///< The text's current color.
};

/** A simple sprite object. */
class SpriteObject : public GraphicalObject {
public:
	SpriteObject();
	~SpriteObject();

	/** Move the object to its default coordinates. */
	void move();
	/** Move the object. */
	void move(uint32 x, uint32 y);
	/** Move the object relative to its feet. */
	void moveFeet(uint32 x, uint32 y);

	/** Clear the sprite. */
	void clear();

	/** Is the sprite empty? */
	bool empty() const;
	/** Are the coordinates within the sprite? */
	bool isIn(uint32 x, uint32 y) const;

	/** Return the sprite's x coordinate. */
	uint32 getX() const;
	/** Return the sprite's y coordinate. */
	uint32 getY() const;

	/** Return the sprite. */
	Sprite &getSprite();
	/** Return the sprite. */
	const Sprite &getSprite() const;

	/** Load the sprite from a BMP. */
	bool loadFromBMP(Resources &resources, const Common::String &bmp);

	/** Redraw the object. */
	void redraw(Sprite &sprite, Common::Rect area);

private:
	Sprite *_sprite; ///< The sprite.
};

/** An animation consisting of several sprite objects. */
class Animation {
public:
	Animation();
	~Animation();

	/** Clear the animation. */
	void clear();

	/** Is the animation empty? */
	bool empty() const;
	/** Return the number of sprites contained in this animation. */
	int frameCount() const;

	/** Is the animation visible? */
	bool isVisible() const;
	/** Set the visibility of the animations. */
	void setVisible(bool visible);

	/** Set the current frame. */
	void setFrame(int frame);
	/** Advance the animation to the next frame. */
	void nextFrame();

	/** Advance the animation to the next frame. */
	void operator++(int);

	/** Return the animation's name. */
	const Common::String &getName() const;

	/** Load the animation from files. */
	bool load(Resources &resources, const Common::String &base);

	/** Get the nth sprite. */
	SpriteObject &getFrame(int n);
	/** Get the nth sprite. */
	const SpriteObject &getFrame(int n) const;

	/** Get the nth sprite. */
	SpriteObject &operator[](int n);
	/** Get the nth sprite. */
	const SpriteObject &operator[](int n) const;

	/** Get the current frame. */
	SpriteObject &getCurrentFrame();
	/** Get the current frame. */
	const SpriteObject &getCurrentFrame() const;

	/** Get the current frame. */
	SpriteObject &operator*();
	/** Get the current frame. */
	const SpriteObject &operator*() const;

	/** Get the current frame. */
	SpriteObject *operator->();
	/** Get the current frame. */
	const SpriteObject *operator->() const;

private:
	/** The sprites. */
	Common::Array<SpriteObject *> _frames;
	Common::Array<SpriteObject *> _sprites;

	/** The animation's name. */
	Common::String _name;

	/** Is the animation visible? */
	bool _visible;

	/** The current frame. */
	int _curFrame;
};

} // End of namespace DarkSeed2

#endif // DARKSEED2_GRAPHICALOBJECT_H
