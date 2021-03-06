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
class ScriptRegister;
class ScriptChunk;

/** Object Verbs. */
enum ObjectVerb {
	kObjectVerbGo   = 0, ///< Go.
	kObjectVerbLook = 1, ///< Look.
	kObjectVerbUse  = 2, ///< Use.
	kObjectVerbNone = 3  ///< None.
};

/** A script object. */
class Object {
public:
	/** Default constructor, only for the sake of Common::Array<>. NOT USEABLE! */
	Object();

	Object(const Variables &variables, ScriptRegister &scriptRegister);
	~Object();

	/** Parse an object out of a suitably positioned DAT file. */
	bool parse(DATFile &dat);

	/** Return the object's name. */
	const Common::String &getName() const;

	/** Get the scripts for that verb. */
	Common::List<ScriptChunk *> &getScripts(ObjectVerb verb);
	/** Get the scripts for that verb. */
	const Common::List<ScriptChunk *> &getScripts(ObjectVerb verb) const;

	/** Are those coordinates within the object's area? */
	bool isIn(uint32 x, uint32 y) const;
	/** Has the objects scripts for that verb? */
	bool hasVerb(ObjectVerb verb) const;
	/** Has the objects scripts with met conditions for that verb? */
	bool hasActiveVerb(ObjectVerb verb) const;

private:
	const Variables *_variables;

	ScriptRegister *_scriptRegister;

	Common::String _name; ///< The object's name.
	Common::Rect   _area; ///< The object's position.

	/** All scripts. */
	Common::Array< Common::List<ScriptChunk *> > _scripts;

	// Parsing helpers
	bool setName(const Common::String &args);
	bool setDimensions(const Common::String &args);
	bool setVerb(const Common::String &cmd, ObjectVerb &curVerb);
	bool addScriptChunk(const Common::String &cmd, DATFile &dat, ObjectVerb curVerb);

	/** Parse a verb string. */
	static ObjectVerb parseObjectVerb(const Common::String &verb);
};

/** A container holding several objects. */
class ObjectContainer {
public:
	ObjectContainer(const Variables &variables, ScriptRegister &scriptRegister);
	~ObjectContainer();

	/** Get all objects. */
	Common::Array<Object> &getObjects();

	/** Find the object with the specified name. */
	Object *findObject(const Common::String &name);
	/** Find the object that's at the specified coordinates. */
	Object *findObject(uint32 x, uint32 y);
	/** Find the autostart object ("auto*") */
	Object *findAutoObject();

	/** Empty the container. */
	void clear();

	/** Parse objects out of a OBJ_*.DAT file. */
	bool parse(DATFile &dat);

protected:
	Common::Array<Object> _objects; ///< All objects.

private:
	const Variables *_variables;

	ScriptRegister *_scriptRegister;
};

} // End of namespace DarkSeed2

#endif // DARKSEED2_OBJECTS_H
