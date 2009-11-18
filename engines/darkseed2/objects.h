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

#ifndef DARKSEED2_OBJECTS_H
#define DARKSEED2_OBJECTS_H

#include "common/array.h"
#include "common/list.h"

#include "engines/darkseed2/darkseed2.h"

namespace DarkSeed2 {

class Variables;
class DATFile;
class ScriptChunk;

enum ObjectVerb {
	kObjectVerbGo = 0,
	kObjectVerbLook,
	kObjectVerbUse,
	kObjectVerbNone
};

class Object {
public:
	/** Default constructor, only for the sake of Common::Array<>. NOT USEABLE! */
	Object();

	Object(const Variables &variables);
	~Object();

	/** Parse an object out of a suitably positioned DAT file. */
	bool parse(DATFile &dat);

	const Common::List<ScriptChunk *> *getScripts(ObjectVerb verb) const;

private:
	const Variables *_variables;

	Common::String _name;
	uint32 _left, _top, _right, _bottom;

	Common::Array< Common::List<ScriptChunk *> > _scripts;

	static ObjectVerb parseObjectVerb(const Common::String &verb);
};

class ObjectContainer {
public:
	ObjectContainer(const Variables &variables);
	~ObjectContainer();

	void clear();

	/** Parse objects out of a OBJ_*.DAT file. */
	bool parse(DATFile &dat);

protected:
	Common::Array<Object> _objects;

private:
	const Variables *_variables;
};

} // End of namespace DarkSeed2

#endif // DARKSEED2_OBJECTS_H