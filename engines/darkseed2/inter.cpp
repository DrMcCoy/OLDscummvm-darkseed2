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

#include "engines/darkseed2/inter.h"
#include "engines/darkseed2/variables.h"
#include "engines/darkseed2/datfile.h"
#include "engines/darkseed2/sound.h"
#include "engines/darkseed2/music.h"
#include "engines/darkseed2/movie.h"
#include "engines/darkseed2/talk.h"
#include "engines/darkseed2/mike.h"
#include "engines/darkseed2/graphics.h"
#include "engines/darkseed2/room.h"
#include "engines/darkseed2/conversationbox.h"
#include "engines/darkseed2/events.h"
#include "engines/darkseed2/saveload.h"

namespace DarkSeed2 {

template<>
void SaveLoad::sync<ScriptInterpreter::Script>(Common::Serializer &serializer, ScriptInterpreter::Script &script) {
	byte waitingFor = (byte) script.waitingFor;

	SaveLoad::sync(serializer, script.signature);

	SaveLoad::sync(serializer, script.soundVar);
	SaveLoad::sync(serializer, script.soundName);
	SaveLoad::sync(serializer, script.soundTalk);

	SaveLoad::sync(serializer, waitingFor);

	script.waitingFor = (ScriptInterpreter::Wait) waitingFor;
}


ScriptInterpreter::Script::Script(ScriptChunk *chnk) {
	chunk      = chnk;
	action     = 0;
	soundID    = -1;
	waitingFor = kWaitNone;

	lastWaitDebug = 0;

	if (chnk)
		signature = chnk->getSignature();
}

#ifndef REDUCE_MEMORY_USAGE
	#define OPCODE(x)          { &ScriptInterpreter::x, #x }
#else
	#define OPCODE(x)          { &ScriptInterpreter::x, "" }
#endif

ScriptInterpreter::OpcodeEntry ScriptInterpreter::_scriptFunc[kScriptActionNone] = {
	OPCODE(oXYRoom),
	OPCODE(oCursor),
	OPCODE(oChange),
	OPCODE(oText),
	OPCODE(oMidi),
	OPCODE(oAnim),
	OPCODE(oFrom),
	OPCODE(oPaletteChange),
	OPCODE(oChangeAt),
	OPCODE(oDialog),
	OPCODE(oPicture),
	OPCODE(oSpeech),
	OPCODE(oSpeechVar),
	OPCODE(oWaitUntil),
	OPCODE(oEffect)
};

ScriptInterpreter::ScriptInterpreter(DarkSeed2Engine &vm) : _vm(&vm) {
	_updatesWithoutChanges = 0;
}

ScriptInterpreter::~ScriptInterpreter() {
}

bool ScriptInterpreter::hasScripts() const {
	return !_scripts.empty();
}

void ScriptInterpreter::clear() {
	_scripts.clear();
	_updatesWithoutChanges = 0;
}

bool ScriptInterpreter::updateStatus() {
	_updatesWithoutChanges++;

	Common::List<Script>::iterator script;

	// Interpret one action from every script in the queue
	for (script = _scripts.begin(); script != _scripts.end(); ++script) {
		if (_vm->shouldQuit())
			break;

		// Evaluating waiting orders
		if (script->waitingFor != kWaitNone) {
			bool waitEnd = false;

			if      (script->waitingFor == kWaitConversation)
				// Waiting for a conversation to end
				waitEnd = !_vm->_graphics->getConversationBox().isActive();
			else if (script->waitingFor == kWaitMovie)
				// Waiting for a movie to end
				waitEnd = !_vm->_movie->isPlaying();

			if (waitEnd)
				script->waitingFor = kWaitNone;
			else
				continue;

			_updatesWithoutChanges = 0;
		}

		// Interpret the next command
		Result result = interpret(*script);

		// Check result
		if        (result == kResultInvalid) {
			// Invalid opcode
			if (script->chunk)
				script->chunk->seekEnd();
			_updatesWithoutChanges = 0;
		} else if (result == kResultOK) {
			// Everything went okay
			script->chunk->next();
			script->lastWaitDebug = 0;
			_updatesWithoutChanges = 0;
		} else if (result == kResultStop) {
			// Script stops here
			script->chunk->seekEnd();
			_updatesWithoutChanges = 0;
		}
	}

	// Go through all scripts and erase those that ended
	script = _scripts.begin();
	while (script != _scripts.end()) {
		if (!script->chunk || script->chunk->atEnd()) {
			script = _scripts.erase(script);
			_updatesWithoutChanges = 0;
		} else
			++script;
	}

	return _updatesWithoutChanges < 10;
}

bool ScriptInterpreter::interpret(Common::List<ScriptChunk *> &chunks) {
	bool queued = false;

	// Create new random variables for the evaluation below
	_vm->_variables->reRollRandom();

	// Push the first script with met conditions into the queue
	for (Common::List<ScriptChunk *>::iterator it = chunks.begin(); it != chunks.end(); ++it) {
		if ((*it)->conditionsMet() && _vm->_events->cameFrom((*it)->getFrom())) {
			(*it)->rewind();
			_scripts.push_back(Script(*it));
			queued = true;
			break;
		}
	}

	if (queued)
		_updatesWithoutChanges = 0;

	return queued;
}

ScriptInterpreter::Result ScriptInterpreter::interpret(Script &script) {
	if (!script.chunk) {
		warning("ScriptInterpreter::interpret(): No such script \"%s\"", script.signature.c_str());
		return kResultInvalid;
	}

	// Get the next action
	script.action = &script.chunk->getAction();

	// Valid action?
	if (script.action->action >= kScriptActionNone) {
		warning("ScriptInterpreter::interpret(): Invalid script action %d",
				script.action->action);

		return kResultInvalid;
	}

	// Look if we want another debug message printed, to avoid spamming oWait lines
	bool doDebug = true;
	if (script.action->action == kScriptActionWaitUntil) {
		if (script.lastWaitDebug++ != 0)
			doDebug = false;

		if (script.lastWaitDebug >= 100)
			script.lastWaitDebug = 0;
	}

	if (doDebug)
		debugC(-1, kDebugOpcodes, "Script function %s [%s]", _scriptFunc[script.action->action].name,
				script.action->arguments.c_str());

	// Look if the opcode is existing
	func_t func = _scriptFunc[script.action->action].func;
	if (!func) {
		warning("ScriptInterpreter::interpret(): Invalid script action %d",
				script.action->action);
		return kResultInvalid;
	}

	// Interpret the action
	Result result = (this->*func)(script);

	script.action = 0;

	return result;
}

ScriptInterpreter::Result ScriptInterpreter::oXYRoom(Script &script) {
	// Position changing

	Common::Array<int32> args = DATFile::argGetInts(script.action->arguments, 5);

	warning("Going to %d+%d:%d (%d)", args[0], args[1], args[3], args[2]);

	_vm->_mike->go(args[0], args[1], (Mike::Direction) args[3]);

	if (args[2] != 0)
		_vm->_events->setNextRoom(args[2]);

	return kResultOK;
}

ScriptInterpreter::Result ScriptInterpreter::oCursor(Script &script) {
	// Change the current cursor

	warning("Unimplemented script function oCursor");
	return kResultInvalid;
}

ScriptInterpreter::Result ScriptInterpreter::oChange(Script &script) {
	// Apply a variables change set

	_vm->_variables->evalChange(script.action->arguments);
	return kResultOK;
}

ScriptInterpreter::Result ScriptInterpreter::oText(Script &script) {
	// Speak a line

	_vm->_talkMan->talk(*_vm->_resources, script.action->arguments);

	script.soundName = script.action->arguments;
	script.soundID   = _vm->_talkMan->getSoundID();
	script.soundTalk = true;

	return kResultOK;
}

ScriptInterpreter::Result ScriptInterpreter::oMidi(Script &script) {
	// Change the background music

	if (!_vm->_music->playMID(*_vm->_resources, script.action->arguments))
		warning("Failed playing music \"%s\"", script.action->arguments.c_str());

	return kResultOK;
}

ScriptInterpreter::Result ScriptInterpreter::oAnim(Script &script) {
	// Animation / Video

	Common::Array<Common::String> lArgs = DATFile::argGet(script.action->arguments);
	if (lArgs.size() >= 5) {
		if (!_vm->_movie->play(lArgs[4], atoi(lArgs[0].c_str()), atoi(lArgs[1].c_str())))
			warning("oAnim: Failed playing video \"%s\"", lArgs[4].c_str());
		else
			script.waitingFor = kWaitMovie;
	} else
		warning("TODO: oAnim \"%s\"", script.action->arguments.c_str());

	return kResultOK;
}

ScriptInterpreter::Result ScriptInterpreter::oFrom(Script &script) {
	// Changing position when coming from a specific room?

	Common::Array<int32> args = DATFile::argGetInts(script.action->arguments, 3);

	if (!_vm->_events->cameFrom(args[2]))
		return kResultStop;

	warning("Setting position to %d+%d", args[0], args[1]);

	_vm->_mike->setPosition(args[0], args[1]);

	return kResultOK;
}

ScriptInterpreter::Result ScriptInterpreter::oPaletteChange(Script &script) {
	// Palette changing

	warning("TODO: Unimplemented script function oPaletteChange");
	return kResultOK;
}

ScriptInterpreter::Result ScriptInterpreter::oChangeAt(Script &script) {
	warning("TODO: Unimplemented script function oChangeAt");
	return kResultOK;
}

ScriptInterpreter::Result ScriptInterpreter::oDialog(Script &script) {
	// Start a conversation

	_vm->_graphics->getInventoryBox().hide();
	_vm->_graphics->getConversationBox().start(script.action->arguments);

	script.waitingFor = kWaitConversation;

	return kResultOK;
}

ScriptInterpreter::Result ScriptInterpreter::oPicture(Script &script) {
	warning("Unimplemented script function oPicture");
	return kResultInvalid;
}

ScriptInterpreter::Result ScriptInterpreter::oSpeech(Script &script) {
	warning("Unimplemented script function oSpeech");
	return kResultInvalid;
}

ScriptInterpreter::Result ScriptInterpreter::oSpeechVar(Script &script) {
	// Set the variable that should change once the talking/SFX has finished

	script.soundVar = script.action->arguments;
	_vm->_sound->setSoundVar(script.soundID, script.soundVar);

	return kResultOK;
}

ScriptInterpreter::Result ScriptInterpreter::oWaitUntil(Script &script) {
	// Wait until a condition is met

	if (_vm->_variables->evalCondition(script.action->arguments))
		// Condition is true => Proceed
		return kResultOK;

	// Condition is false => Keep waiting here
	return kResultWait;
}

ScriptInterpreter::Result ScriptInterpreter::oEffect(Script &script) {
	// Play a sound effect

	_vm->_sound->playSound(*_vm->_resources, script.action->arguments, &script.soundID);

	script.soundName = script.action->arguments;
	script.soundTalk = false;

	return kResultOK;
}

bool ScriptInterpreter::saveLoad(Common::Serializer &serializer, Resources &resources) {
	for (Common::List<Script>::iterator script = _scripts.begin(); script != _scripts.end(); ++script)
		if (!_vm->_sound->isIDPlaying(script->soundID)) {
			script->soundVar.clear();
			script->soundName.clear();
		}

	SaveLoad::sync(serializer, _scripts);

	return true;
}

bool ScriptInterpreter::loading(Resources &resources) {
	_updatesWithoutChanges = 0;

	// Rebuild the script list
	for (Common::List<Script>::iterator script = _scripts.begin(); script != _scripts.end(); ++script) {
		script->chunk = _vm->_scriptRegister->getScript(script->signature);
		if (!script->soundName.empty()) {
			if (script->soundTalk) {
				_vm->_talkMan->talk(resources, script->soundName);
				script->soundID = _vm->_talkMan->getSoundID();
				_vm->_sound->setSoundVar(script->soundID, script->soundVar);
			} else
				_vm->_sound->playSound(resources, script->soundName, &script->soundID);
				_vm->_sound->setSoundVar(script->soundID, script->soundVar);
		}
	}

	return true;
}

} // End of namespace DarkSeed2
