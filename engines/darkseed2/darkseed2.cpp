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

// Base stuff
#include "common/endian.h"
#include "common/md5.h"
#include "common/random.h"
#include "base/plugins.h"
#include "common/config-manager.h"
#include "common/EventRecorder.h"
#include "common/debug-channels.h"
#include "common/macresman.h"

#include "engines/util.h"

// Audio
#include "audio/mixer.h"
#include "audio/mididrv.h"

// Save/Load
#include "gui/saveload.h"
#include "engines/darkseed2/saveload.h"

// Dark Seed II subsystems
#include "engines/darkseed2/darkseed2.h"
#include "engines/darkseed2/options.h"
#include "engines/darkseed2/cursors.h"
#include "engines/darkseed2/resources.h"
#include "engines/darkseed2/font.h"
#include "engines/darkseed2/script.h"
#include "engines/darkseed2/imageconverter.h"
#include "engines/darkseed2/graphics.h"
#include "engines/darkseed2/room.h"
#include "engines/darkseed2/conversationbox.h"
#include "engines/darkseed2/sound.h"
#include "engines/darkseed2/music.h"
#include "engines/darkseed2/variables.h"
#include "engines/darkseed2/talk.h"
#include "engines/darkseed2/mike.h"
#include "engines/darkseed2/movie.h"
#include "engines/darkseed2/roomconfig.h"
#include "engines/darkseed2/inter.h"
#include "engines/darkseed2/events.h"

namespace DarkSeed2 {

// Files
static const char *kExecutable    = "dark0001.exe";
static const char *kResourceIndex = "gfile.hdr";
static const char *kVariableIndex = "GAMEVAR";

DarkSeed2Engine::DarkSeed2Engine(OSystem *syst, const DS2GameDescription *gameDesc) :
	Engine(syst), _gameDescription(gameDesc) {

	DebugMan.addDebugChannel(kDebugResources   , "Resources"   , "Resource handling debug level");
	DebugMan.addDebugChannel(kDebugGraphics    , "Graphics"    , "Graphics debug level");
	DebugMan.addDebugChannel(kDebugMusic       , "Music"       , "Music debug level");
	DebugMan.addDebugChannel(kDebugSound       , "Sound"       , "Sound debug level");
	DebugMan.addDebugChannel(kDebugTalk        , "Talk"        , "Talk debug level");
	DebugMan.addDebugChannel(kDebugMovie       , "Movie"       , "Movie debug level");
	DebugMan.addDebugChannel(kDebugScript      , "Script"      , "Script debug level");
	DebugMan.addDebugChannel(kDebugRooms       , "Rooms"       , "Rooms debug level");
	DebugMan.addDebugChannel(kDebugObjects     , "Objects"     , "Objects debug level");
	DebugMan.addDebugChannel(kDebugConversation, "Conversation", "Conversation debug level");
	DebugMan.addDebugChannel(kDebugOpcodes     , "Opcodes"     , "Script functions debug level");
	DebugMan.addDebugChannel(kDebugRoomConf    , "RoomConf"    , "Room config debug level");
	DebugMan.addDebugChannel(kDebugGameflow    , "Gameflow"    , "Gameflow debug level");

	// Setup mixer
	_mixer->setVolumeForSoundType(Audio::Mixer::kMusicSoundType, ConfMan.getInt("music_volume"));

	_options        = 0;
	_cursors        = 0;
	_resources      = 0;
	_fontMan        = 0;
	_sound          = 0;
	_music          = 0;
	_variables      = 0;
	_scriptRegister = 0;
	_graphics       = 0;
	_talkMan        = 0;
	_mike           = 0;
	_movie          = 0;
	_roomConfMan    = 0;
	_inter          = 0;
	_events         = 0;
	_macExeResFork  = 0;

	_rnd = new Common::RandomSource();
	g_eventRec.registerRandomSource(*_rnd, "ds2");

	_engineStartTime = 0;
	_playTime        = 0;
}

DarkSeed2Engine::~DarkSeed2Engine() {
	_music->stop();
	_sound->stopAll();

	_mixer->stopAll();

	delete _events;
	delete _inter;
	delete _movie;
	delete _mike;
	delete _talkMan;
	delete _graphics;
	delete _roomConfMan;

	delete _variables;
	delete _scriptRegister;
	delete _music;
	delete _sound;
	delete _fontMan;
	delete _resources;
	delete _cursors;
	delete _options;

	delete _midiDriver;

	delete _rnd;

	delete _macExeResFork;
}

Common::Error DarkSeed2Engine::run() {
	int32 width, height;
	if (!getScreenResolution(width, height))
		return Common::kUnknownError;

	if (!initGraphics(width, height))
		return Common::kUnknownError;

	if (!init(width, height))
		return Common::kUnknownError;

	if (!initGraphicsSystem())
		return Common::kUnknownError;

	debug(-1, "Done initializing.");

	_engineStartTime = g_system->getMillis();

	while (!shouldQuit()) {
		_events->setLoading(false);
		if (!_events->run()) {
			return Common::kUnknownError;
		}
	}

	return Common::kNoError;
}

void DarkSeed2Engine::pauseEngineIntern(bool pause) {
	_mixer->pauseAll(pause);
}

void DarkSeed2Engine::syncSoundSettings() {
	Engine::syncSoundSettings();

	_options->syncSettings();

	_sound->syncSettings(*_options);
	_music->syncSettings(*_options);
	_talkMan->syncSettings(*_options);
}

bool DarkSeed2Engine::getScreenResolution(int32 &width, int32 &height) const {
	if (isWindowsPC() || isMac()) {
		width  = 640;
		height = 480;
		return true;
	} else if (isSaturn()) {
		width  = 320;
		height = 240;
		return true;
	} else
		warning("DarkSeed2Engine::getScreenResolution(): Unknown game version");

	return false;
}

bool DarkSeed2Engine::init(int32 width, int32 height) {
	uint32 midiDriver = MidiDriver::detectDevice(MDT_MIDI | MDT_ADLIB | MDT_PREFER_GM);
	bool native_mt32 = ((MidiDriver::getMusicType(midiDriver) == MT_MT32) || ConfMan.getBool("native_mt32"));

	_midiDriver = MidiDriver::createMidi(midiDriver);

	if (native_mt32)
		_midiDriver->property(MidiDriver::PROP_CHANNEL_MASK, 0x03FE);

	debug(-1, "Creating subclasses...");

	if (isMac()) {
		// Open up the Mac resource fork for the executable
		_macExeResFork = new Common::MacResManager();
		if (!_macExeResFork->open("Dark Seed II/Dark Seed II")) {
			warning("Could not open 'Dark Seed II'");
			return false;
		}
	}

	_options        = new Options();
	_variables      = new Variables(*_rnd);
	_scriptRegister = new ScriptRegister();
	_resources      = new Resources();
	_fontMan        = new FontManager(*_resources);
	_sound          = new Sound(*_mixer, *_variables);
	_music          = new Music(*_mixer, *_midiDriver);

	// The cursors need to be created after Resources but before Graphics
	if (isWindowsPC())
		_cursors    = new CursorsWindows(kExecutable);
	else if (isSaturn())
		_cursors    = new CursorsSaturn(*_resources);
	else if (isMac())
		_cursors    = new CursorsMac(*_macExeResFork);

	_graphics       = new Graphics(width, height, *_resources, *_variables, *_cursors, *_fontMan);
	_talkMan        = new TalkManager(_resources->getVersionFormats(), *_sound, *_graphics, *_fontMan);
	_mike           = new Mike(*_resources, *_variables, *_graphics);
	_movie          = new Movie(*_mixer, *_graphics, *_cursors, *_sound);
	_roomConfMan    = new RoomConfigManager(*this);
	_inter          = new ScriptInterpreter(*this);
	_events         = new Events(*this);

	syncSoundSettings();

	debug(-1, "Indexing resources...");

	if (isSaturn()) {
		if (!_resources->indexPGF()) {
			warning("DarkSeed2Engine::init(): Couldn't index resources");
			return false;
		}

		_resources->setGameVersion(kGameVersionSaturn, getLanguage());
	} else if (isWindowsPC()) {
		if (!_resources->index(kResourceIndex)) {
			warning("DarkSeed2Engine::init(): Couldn't index resources");
			return false;
		}

		_resources->setGameVersion(kGameVersionWindows, getLanguage());
	} else if (isMac()) {
		if (!_resources->indexMacResources()) {
			warning("DarkSeed2Engine::init(): Indexing Mac resources not yet supported");
			return false;
		}

		_resources->setGameVersion(kGameVersionMac, getLanguage());
	}

	_sound->init(_resources->getVersionFormats().getSoundType());

	if (!_cursors->load()) {
		warning("DarkSeed2Engine::init(): Couldn't load cursors");
		return false;
	}

	if (!_fontMan->init(_resources->getVersionFormats().getGameVersion(), getLanguage())) {
		warning("DarkSeed2Engine::init(): Couldn't initialize the font manager");
		return false;
	}

	if (!_events->init()) {
		warning("DarkSeed2Engine::init(): Couldn't initialize the event handler");
		return false;
	}

	debug(-1, "Initializing game variables...");

	if (isMac()) {
		Common::SeekableReadStream *stream = _macExeResFork->getResource(kVariableIndex);
		if (!_variables->loadFromIDX(*stream)) {
			delete stream;
			warning("DarkSeed2Engine::init(): Couldn't load initial variables values");
			return false;
		}
		delete stream;
	} else if (!_variables->loadFromIDX(*_resources, kVariableIndex)) {
		warning("DarkSeed2Engine::init(): Couldn't load initial variables values");
		return false;
	}

	bool needPalette = !isSaturn();
	if (!_mike->init(needPalette)) {
		warning("DarkSeed2Engine::init(): Couldn't initialize Mike");
		return false;
	}

	return true;
}

bool DarkSeed2Engine::initGraphics(int32 width, int32 height) {
	debug(-1, "Setting up graphics...");

	::initGraphics(width, height, width == 640, 0);

	ImgConv.setPixelFormat(g_system->getScreenFormat());

	return true;
}

bool DarkSeed2Engine::initGraphicsSystem() {
	debug(-1, "Setting up the graphics system...");

	if (!_graphics->init(*_talkMan, *_scriptRegister, *_roomConfMan, *_movie))
		return false;

	return true;
}

bool DarkSeed2Engine::doLoadDialog() {
	const EnginePlugin *plugin = 0;
	EngineMan.findGame(getGameId(), &plugin);
	assert(plugin);

	GUI::SaveLoadChooser *dialog = new GUI::SaveLoadChooser("Load game:", "Load");

	dialog->setSaveMode(false);
	int slot = dialog->runModalWithPluginAndTarget(plugin, ConfMan.getActiveDomainName());

	bool result = false;
	if (slot >= 0)
		if (!loadGameState(slot))
			result = true;

	delete dialog;
	return result;
}

void DarkSeed2Engine::clearAll() {
	_movie->stop();
	_music->stop();
	_talkMan->endTalk();
	_sound->stopAll();
	_mike->setWalkMap();

	_graphics->unregisterBackground();
	_inter->clear();

	_graphics->getRoom().clear();
	_graphics->getConversationBox().stop();

	_scriptRegister->clear();
}

bool DarkSeed2Engine::isWindowsPC() const {
	return getPlatform() == Common::kPlatformPC;
}

bool DarkSeed2Engine::isSaturn() const {
	// TODO: kPlatformSaturn
	return getPlatform() == Common::kPlatformUnknown;
}

bool DarkSeed2Engine::isMac() const {
	return getPlatform() == Common::kPlatformMacintosh;
}

bool DarkSeed2Engine::canLoadGameStateCurrently() {
	// We can always load
	return true;
}

bool DarkSeed2Engine::canSaveGameStateCurrently() {
	// We can always save
	return true;
}

Common::Error DarkSeed2Engine::loadGameState(int slot) {
	SaveMetaInfo meta;

	Common::InSaveFile *file = SaveLoad::openForLoading(SaveLoad::createFileName(_targetName, slot));
	if (!file)
		return Common::kUnknownError;

	if (!SaveLoad::skipThumbnail(*file))
		return Common::kUnknownError;

	Common::Serializer serializer(file, 0);

	if (!saveLoad(serializer, meta))
		return Common::kUnknownError;

	delete file;

	_playTime = meta.getPlayTime();

	_events->setLoading(true);

	_graphics->retrace();
	g_system->updateScreen();

	return Common::kNoError;
}

Common::Error DarkSeed2Engine::saveGameState(int slot, const char *desc) {
	_graphics->retrace();

	SaveMetaInfo meta;

	meta.description = desc;
	meta.fillWithCurrentTime(_engineStartTime, _playTime);

	Common::OutSaveFile *file = SaveLoad::openForSaving(SaveLoad::createFileName(_targetName, slot));
	if (!file)
		return Common::kUnknownError;

	if (!SaveLoad::saveThumbnail(*file))
		return Common::kUnknownError;

	Common::Serializer serializer(0, file);

	if (!saveLoad(serializer, meta))
		return Common::kUnknownError;

	bool flushed = file->flush();
	if (!flushed || file->err())
		return Common::kUnknownError;

	delete file;

	return Common::kNoError;
}

bool DarkSeed2Engine::saveLoad(Common::Serializer &serializer, SaveMetaInfo &meta) {
	if (!SaveLoad::syncMetaInfo(serializer, meta))
		return false;

	if (serializer.isLoading())
		clearAll();

	if (!_variables->doSaveLoad(serializer, *_resources))
		return false;

	if (!_music->doSaveLoad(serializer, *_resources))
		return false;

	if (!_scriptRegister->doSaveLoad(serializer, *_resources))
		return false;

	if (!_graphics->doSaveLoad(serializer, *_resources))
		return false;

	if (!_roomConfMan->doSaveLoad(serializer, *_resources))
		return false;

	if (!_movie->doSaveLoad(serializer, *_resources))
		return false;

	if (!_inter->doSaveLoad(serializer, *_resources))
		return false;

	if (!_mike->doSaveLoad(serializer, *_resources))
		return false;

	if (!_events->doSaveLoad(serializer, *_resources))
		return false;

	if (!_cursors->doSaveLoad(serializer, *_resources))
		return false;

	return true;
}

} // End of namespace DarkSeed2
