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

#ifndef DARKSEED2_NECURSORS_H
#define DARKSEED2_NECURSORS_H

#include "common/str.h"
#include "common/array.h"
#include "common/list.h"
#include "common/file.h"

#include "engines/darkseed2/darkseed2.h"

namespace DarkSeed2 {

struct NECursor {
	uint16 width;
	uint16 height;
	uint16 hotspotX;
	uint16 hotspotY;

	uint32 dataSize;
	byte *data;

	NECursor();
	~NECursor();
};

struct NECursorGroup {
	Common::String name;
	Common::Array<NECursor> cursors;
};

class NEResources {
public:
	NEResources();
	~NEResources();

	void clear();

	bool loadFromEXE(const Common::String &fileName);

	const Common::Array<NECursorGroup> &getCursors() const;

private:
	enum IDType {
		kIDTypeNumerical,
		kIDTypeString
	};

	struct Resource {
		IDType idType;

		Common::String name;
		uint32 id;

		uint16 type;

		uint32 offset;
		uint32 size;

		uint16 flags;
		uint16 handle;
		uint16 usage;
	};

	Common::String _fileName;
	Common::File _exe;

	Common::List<Resource> _resources;

	Common::Array<NECursorGroup> _cursors;

	bool tryOpen();
	void close();

	uint32 getResourceTableOffset();
	bool readResourceTable(uint32 offset);

	bool readCursors();
	bool readCursorGroup(NECursorGroup &group, const Resource &resource);
	bool readCursor(NECursor &cursor, const Resource &resource);

	const Resource *findResource(uint16 type, uint16 id) const;

	static Common::String getResourceString(Common::SeekableReadStream &exe, uint32 offset);

};

} // End of namespace DarkSeed2

#endif // DARKSEED2_NECURSORS_H
