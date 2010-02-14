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

#ifndef DARKSEED2_SAVELOAD_H
#define DARKSEED2_SAVELOAD_H

#include "common/str.h"
#include "common/hashmap.h"
#include "common/stream.h"
#include "common/savefile.h"
#include "common/serializer.h"

#include "engines/game.h"

namespace DarkSeed2 {

struct SaveMetaInfo {
	Common::String description;
	uint32 saveDate;
	uint16 saveTime;
	uint32 playTime;

	SaveMetaInfo();

	void fillWithCurrentTime(uint32 startTime, uint32 prevPlayTime);

	int getSaveYear()   const;
	int getSaveMonth()  const;
	int getSaveDay()    const;
	int getSaveHour()   const;
	int getSaveMinute() const;

	int getPlayHour()   const;
	int getPlayMinute() const;

	uint32 getPlayTime() const;
};

class SaveLoad {
public:
	static const int kMaxSlot = 99;

	static bool syncMetaInfo(Common::Serializer &serializer, SaveMetaInfo &meta);
	static bool loadMetaInfo(Common::SeekableReadStream *stream, SaveMetaInfo &meta);

	static Common::String createFileName(const Common::String &base, int slot);

	static Common::OutSaveFile *openForSaving (const Common::String &file);
	static Common::InSaveFile  *openForLoading(const Common::String &file);

	static bool getState(SaveStateDescriptor &state, const Common::String &target, int slot);
	static bool getStates(SaveStateList &list, const Common::String &target);

	static bool removeSave(const Common::String &base, int slot);

	static bool skipThumbnailHeader(Common::SeekableReadStream &in);
	static bool saveThumbnail(Common::WriteStream &out);

	static void syncTimestamp(Common::Serializer &serializer, uint32 &time);

	template<typename T>
	static void sync(Common::Serializer &serializer, T &var);

	template<class Key, class Val, class HashFunc, class EqualFunc>
	static void sync(Common::Serializer &serializer, Common::HashMap<Key, Val, HashFunc, EqualFunc> &map) {
		uint32 size = map.size();

		sync(serializer, size);

		if (serializer.isSaving()) {
			typename Common::HashMap<Key, Val, HashFunc, EqualFunc>::iterator it;
			for (it = map.begin(); it != map.end(); ++it) {
				Key key = it->_key;
				sync(serializer, key);
				sync(serializer, it->_value);
			}
		} else {
			map.clear();
			for (uint32 i = 0; i < size; i++) {
				Key key;
				Val value;
				sync(serializer, key);
				sync(serializer, value);
				map.setVal(key, value);
			}
		}

	}

	template<class T>
	static void sync(Common::Serializer &serializer, Common::Array<T> &array) {
		uint32 size = array.size();

		sync(serializer, size);

		if (serializer.isSaving()) {
			typename Common::Array<T>::iterator it;
			for (it = array.begin(); it != array.end(); ++it) {
				sync(serializer, *it);
			}
		} else {
			array.clear();
			array.reserve(size);
			for (uint32 i = 0; i < size; i++) {
				T entry;
				sync(serializer, entry);
				array.push_back(entry);
			}
		}

	}

	template<class T>
	static void sync(Common::Serializer &serializer, Common::List<T> &list) {
		uint32 size = list.size();

		sync(serializer, size);

		if (serializer.isSaving()) {
			typename Common::List<T>::const_iterator it;
			for (it = list.begin(); it != list.end(); ++it) {
				T entry = *it;
				sync(serializer, entry);
			}
		} else {
			list.clear();
			for (uint32 i = 0; i < size; i++) {
				T entry;
				sync(serializer, entry);
				list.push_back(entry);
			}
		}

	}
};

} // End of namespace DarkSeed2

#endif // DARKSEED2_SAVELOAD_H
