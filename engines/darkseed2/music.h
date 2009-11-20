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

#ifndef DARKSEED2_MUSIC_H
#define DARKSEED2_MUSIC_H

#include "common/mutex.h"

#include "sound/mixer.h"
#include "sound/mididrv.h"

#include "engines/darkseed2/darkseed2.h"

namespace Common {
	class SeekableReadStream;
	class String;
}

class MidiParser;

namespace DarkSeed2 {

class Resource;
class MidiPlayer;

class Music {
public:
	Music(Audio::Mixer &mixer, MidiDriver &driver);
	~Music();

	bool playMID(Common::SeekableReadStream &mid);
	bool playMID(const Resource &resource);

	void stop();

	/** Apply volume settings. */
	void syncSettings(const Options &options);

private:
	Audio::Mixer *_mixer;

	MidiPlayer *_midiPlayer;
};

// Taken from Draci, which took it from MADE, which took it from SAGA.

class MidiPlayer : public MidiDriver {
public:
	MidiPlayer(MidiDriver *driver, const char *pathMask);
	~MidiPlayer();

	bool isPlaying() { return _isPlaying; }
	void setPlaying(bool playing) { _isPlaying = playing; }

	void setVolume(int volume);
	int getVolume() { return _masterVolume; }
	void syncVolume();

	void setNativeMT32(bool b) { _nativeMT32 = b; }
	bool hasNativeMT32() { return _nativeMT32; }
	void playSMF(Common::SeekableReadStream &stream, bool loop);
	void stop();
	void pause();
	void resume();
	void setLoop(bool loop) { _looping = loop; }
	void setPassThrough(bool b) { _passThrough = b; }

	void setGM(bool isGM) { _isGM = isGM; }

	// MidiDriver interface implementation
	int open();
	void close();
	void send(uint32 b);

	void metaEvent(byte type, byte *data, uint16 length);

	void setTimerCallback(void *timerParam, void (*timerProc)(void *)) { }
	uint32 getBaseTempo() { return _driver ? _driver->getBaseTempo() : 0; }

	//Channel allocation functions
	MidiChannel *allocateChannel() { return 0; }
	MidiChannel *getPercussionChannel() { return 0; }

	MidiParser *_parser;
	Common::Mutex _mutex;

protected:

	static void onTimer(void *data);
	void setChannelVolume(int channel);

	MidiChannel *_channel[16];
	MidiDriver *_driver;
	MidiParser *_smfParser;
	Common::String _pathMask;
	byte _channelVolume[16];
	bool _nativeMT32;
	bool _isGM;
	bool _passThrough;

	bool _isPlaying;
	bool _looping;
	byte _masterVolume;
	int _track;

	byte *_midiMusicData;
};

} // End of namespace DarkSeed2

#endif // DARKSEED2_MUSIC_H
