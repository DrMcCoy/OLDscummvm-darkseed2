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

#include "common/stream.h"

#include "engines/darkseed2/talk.h"
#include "engines/darkseed2/resources.h"
#include "engines/darkseed2/sound.h"
#include "engines/darkseed2/graphics.h"

namespace DarkSeed2 {

TalkLine::TalkLine(const Resources &resources, const Common::String &talkName) {
	_resources = &resources;

	_talkName = talkName;

	_wav = 0;

	// Reading the sound
	if (_resources->hasResource(talkName + ".WAV"))
		_wav = _resources->getResource(talkName + ".WAV");

	// Reading the text
	Resource *txt;
	if (_resources->hasResource(talkName + ".TXT")) {
		txt = _resources->getResource(talkName + ".TXT");

		Common::SeekableReadStream &txtStream = txt->getStream();

		while (!txtStream.err() && !txtStream.eos()) {
			Common::String line = txtStream.readLine();
			if (line.empty() && txtStream.eos())
				continue;

			if (!_txt.empty())
				_txt += '\n';
			_txt += line;
		}

		delete txt;
	}
}

TalkLine::~TalkLine() {
	delete _wav;
}

bool TalkLine::hasWAV() const {
	return _wav != 0;
}

bool TalkLine::hasTXT() const {
	return !_txt.empty();
}

const Resource &TalkLine::getWAV() const {
	if (!_wav)
		error("Resource %s.WAV does not exist", _talkName.c_str());

	return *_wav;
}

const Common::String &TalkLine::getTXT() const {
	return _txt;
}


TalkManager::TalkManager(Sound &sound, Graphics &graphics) {
	_sound    = &sound;
	_graphics = &graphics;

	_curTalk = -1;
}

TalkManager::~TalkManager() {
}

bool TalkManager::talk(const TalkLine &talkLine) {
	endTalk();

	if (talkLine.hasWAV()) {
		if (!_sound->playWAV(talkLine.getWAV(), _curTalk, Audio::Mixer::kSpeechSoundType)) {
			warning("TalkManager::talk(): WAV playing failed");
			return false;
		}

		_graphics->talk(talkLine.getTXT());
	} else {
		warning("TalkManager::talk(): Talk line has no WAV");
		return false;
	}

	return true;
}

void TalkManager::endTalk() {
	_graphics->talkEnd();
	_curTalk = -1;
}

void TalkManager::updateStatus() {
	if (_curTalk == -1)
		return;

	if (!_sound->isIDPlaying(_curTalk))
		endTalk();
}

} // End of namespace DarkSeed2
