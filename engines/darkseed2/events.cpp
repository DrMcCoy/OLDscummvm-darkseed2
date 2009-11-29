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

#include "common/events.h"

#include "engines/darkseed2/events.h"
#include "engines/darkseed2/graphics.h"
#include "engines/darkseed2/room.h"
#include "engines/darkseed2/talk.h"
#include "engines/darkseed2/conversationbox.h"
#include "engines/darkseed2/variables.h"
#include "engines/darkseed2/inter.h"

namespace DarkSeed2 {

Events::Events(DarkSeed2Engine &vm) : _vm(&vm) {
	_cursors = new ModeCursors[kCursorModeNone];

	// Getting cursors
	_cursors[kCursorModeWalk].inactive = _vm->_cursors->getCursor();
	_cursors[kCursorModeWalk].active   = _vm->_cursors->getCursor("c4Ways");
	_cursors[kCursorModeUse ].inactive = _vm->_cursors->getCursor("cHand");
	_cursors[kCursorModeUse ].active   = _vm->_cursors->getCursor("cUseIt");
	_cursors[kCursorModeLook].inactive = _vm->_cursors->getCursor("cLook");
	_cursors[kCursorModeLook].active   = _vm->_cursors->getCursor("cLookAt");

	_inIntro = false;

	// Set the default (pointer) cursor
	_canSwitchCursors = true;

	_cursorMode   = kCursorModeWalk;
	_cursorActive = false;
	setCursor();

	_vm->_cursors->setVisible(true);
}

Events::~Events() {
	delete[] _cursors;
}

bool Events::setupIntroSequence() {
	_canSwitchCursors = false;

	_cursorMode   = kCursorModeUse;
	_cursorActive = false;
	setCursor();

	// Cutscene room
	if (!roomGo("0001"))
		return false;

	_inIntro = true;

	// Run the main loop as long as scripts are still active
	mainLoop(true);

	// Title room
	if (!roomGo("0002"))
		return false;

	// Run the main loop as long as scripts are still active
	mainLoop(true);

	// Loading title parts
	_titleSprites[0].loadFromBMP(*_vm->_resources, "002TIT01", 145,  27);
	_titleSprites[1].loadFromBMP(*_vm->_resources, "002BTN01", 164, 196);
	_titleSprites[2].loadFromBMP(*_vm->_resources, "002BTN02", 164, 256);
	_titleSprites[3].loadFromBMP(*_vm->_resources, "002BTN03", 164, 314);
	_titleSprites[4].loadFromBMP(*_vm->_resources, "002BTN04", 164, 376);

	for (int i = 0; i < 5; i++) {
		// Draw those parts

		if (_titleSprites[i].empty()) {
			warning("Events::setupIntroSequence(): Couldn't load title screen elements");
			return false;
		}

		_vm->_graphics->blitToScreen(_titleSprites[i].getSprite(),
				_titleSprites[i].getX(), _titleSprites[i].getY(), true);
	}

	return true;
}

void Events::leaveIntro() {
	// Throw title parts away again
	for (int i = 0; i < 5; i++)
		_titleSprites[i].clear();

	// Intro movie room
	if (!roomGo("1501")) {
		warning("Events::leaveIntro(): Failed loading the intro movie room");
		_vm->quitGame();
		return;
	}

	// Run the main loop as long as scripts are still active
	mainLoop(true);

	_inIntro = false;

	// Restore normal cursor mode
	_canSwitchCursors = true;
	_cursorMode   = kCursorModeWalk;
	_cursorActive = false;
	setCursor();

	// First room
	if (!roomGo("0101")) {
		warning("Events::leaveIntro(): Failed loading the first room");
		_vm->quitGame();
		return;
	}
}

void Events::mainLoop(bool finishScripts) {
	bool scriptStateChanged;

	while (!_vm->shouldQuit()) {
		// Look for user input
		handleInput();

		// Update subsystem statuses
		_vm->_talkMan->updateStatus();
		_vm->_graphics->updateStatus();

		scriptStateChanged = _vm->_inter->updateStatus();
		if (finishScripts && !scriptStateChanged)
			// We run only to finish the scripts, but the scripts won't do anything
			break;

		// Room chaning
		if (_changeRoom) {
			_changeRoom = false;

			roomGo(_nextRoom);
		}

		// Update screen
		_vm->_graphics->retrace();
		g_system->updateScreen();

		// Wait
		g_system->delayMillis(10);
	}
}

void Events::handleInput() {
	Common::Event event;

	bool hasMove = false;
	uint32 mouseX, mouseY;

	while (g_system->getEventManager()->pollEvent(event)) {
		switch (event.type) {
		case Common::EVENT_MOUSEMOVE:
			hasMove = true;
			mouseX = event.mouse.x;
			mouseY = event.mouse.y;
			break;

		case Common::EVENT_LBUTTONUP:
			// If the mouse was moved, handle that beforehand
			if (hasMove)
				mouseMoved(mouseX, mouseY);
			hasMove = false;

			mouseClickedLeft(event.mouse.x, event.mouse.y);
			break;

		case Common::EVENT_RBUTTONUP:
			// If the mouse was moved, handle that beforehand
			if (hasMove)
				mouseMoved(mouseX, mouseY);
			hasMove = false;

			mouseClickedRight(event.mouse.x, event.mouse.y);
			break;

		case Common::EVENT_KEYDOWN:
			if (event.kbd.keycode == Common::KEYCODE_F5)
				// Options, handled by the GMM
				_vm->openMainMenuDialog();
			break;

		default:
			break;

		}
	}

	if (hasMove)
		mouseMoved(mouseX, mouseY);
}

void Events::mouseMoved(uint32 x, uint32 y) {
	if (_inIntro) {
		// Mouse in a button area?
		int titleSprite = checkTitleSprites(x, y);

		_cursorActive = (titleSprite != 0);
		setCursor();

		return;
	}

	// Update conversation box
	if (_vm->_graphics->getConversationBox().isActive()) {
		uint32 convX = x - Graphics::_conversationX;
		uint32 convY = y - Graphics::_conversationY;

		_vm->_graphics->getConversationBox().notifyMouseMove(convX, convY);

		return;
	}

	// Look for hotspots, but only if we're not currently doing something important
	if (_vm->_variables->get("SysCall") == 0)
		checkHotspot(x, y);
}

void Events::mouseClickedLeft(uint32 x, uint32 y) {
	if (_inIntro) {
		// Mouse in a button area?
		int titleSprite = checkTitleSprites(x, y);

		if        (titleSprite == 2) {
			// New game
			leaveIntro();
		} else if (titleSprite == 3) {
			// Load game
		} else if (titleSprite == 4) {
			// Options
			_vm->openMainMenuDialog();
		} else if (titleSprite == 5) {
			// Exit
			_vm->quitGame();
		}

		return;
	}

	// Update conversation box
	if (_vm->_graphics->getConversationBox().isActive()) {
		uint32 convX = x - Graphics::_conversationX;
		uint32 convY = y - Graphics::_conversationY;

		_vm->_graphics->getConversationBox().notifyClicked(convX, convY);
	}

	// Did we click any objects? (But ignore if we're doing something important)
	if (_vm->_variables->get("SysCall") == 0) {
		Object *curObject = _vm->_graphics->getRoom().findObject(x, y);
		if (curObject)
			doObjectVerb(*curObject, cursorModeToObjectVerb(_cursorMode));
	}
}

void Events::checkHotspot(uint32 x, uint32 y) {
	bool cursorActive = false;

	Object *curObject = _vm->_graphics->getRoom().findObject(x, y);
	if (curObject) {
		if (curObject->hasActiveVerb(cursorModeToObjectVerb(_cursorMode)))
			cursorActive = true;
	}

	if (cursorActive != _cursorActive) {
		_cursorActive = cursorActive;
		setCursor();
	}
}

void Events::mouseClickedRight(uint32 x, uint32 y) {
	if (!_vm->_graphics->getConversationBox().isActive() && !_vm->_talkMan->isTalking()) {
		// If no one is talking and the conversation box isn't shown, cycle the mouse cursors
		if (_vm->_variables->get("SysCall") == 0) {
			// If we're not doing something important
			cycleCursorMode();
			checkHotspot(x, y);
		}
	}

	if (_vm->_talkMan->isTalking())
		// Aborting talks with the right mouse button
		_vm->_talkMan->endTalk();
}

void Events::cycleCursorMode() {
	if (!_canSwitchCursors)
		return;

	_cursorMode = (CursorMode) ((((int) _cursorMode) + 1) % kCursorModeNone);
	setCursor();
}

void Events::setCursor() {
	setCursor(_cursorMode, _cursorActive);
}

void Events::setCursor(CursorMode cursor, bool active) {
	if (_cursorMode >= kCursorModeNone)
		return;

	if (active)
		setCursor(*_cursors[(int) cursor].active);
	else
		setCursor(*_cursors[(int) cursor].inactive);
}

void Events::setCursor(const Cursors::Cursor &cursor) {
	_vm->_cursors->setCursor(cursor);
}

void Events::doObjectVerb(Object &object, ObjectVerb verb) {
	_vm->_inter->interpret(object.getScripts(verb), true);
}

int Events::checkTitleSprites(uint32 x, uint32 y) const {
	for (int i = 1; i < 5; i++)
		if (_titleSprites[i].isIn(x, y))
			return i + 1;

	return 0;
}

bool Events::roomEnter() {
	Room &room = _vm->_graphics->getRoom();

	debugC(-1, kDebugRooms, "Entering room \"%s\"", room.getName().c_str());

	// Set the background
	_vm->_graphics->registerBackground(room.getBackground());

	// Evaluate music changes
	if (!_vm->_inter->interpret(room.getScripts(kRoomVerbMusic)))
		return false;

	// Evaluate the entry logic
	if (!_vm->_inter->interpret(room.getScripts(kRoomVerbEntry)))
		return false;

	// Evaluate the autostart objects
	executeAutoStart(room);

	_vm->_inter->interpret(room.getScripts(kRoomVerbSprite), true);

	return true;
}

void Events::roomLeave() {
	_vm->_graphics->unregisterBackground();
	_vm->_inter->clear();
}

bool Events::roomGo(const Common::String &room) {
	Room &curRoom = _vm->_graphics->getRoom();

	roomLeave();

	if (!curRoom.parse(*_vm->_resources, room))
		return false;

	if (!roomEnter())
		return false;

	return true;
}

void Events::setNextRoom(uint32 room) {
	if (!_inIntro) {
		_nextRoom = Common::String::printf("%04d", room);

		if (_nextRoom != _vm->_graphics->getRoom().getName()) {
			warning("Room transition to room %s", _nextRoom.c_str());
			_changeRoom = true;
		}
	}
}

static const char *autoStartName[] = {
	"autostart", "auto start", "autoroom", "auto room"
};

bool Events::executeAutoStart(Room &room) {
	bool has = false;
	Object *autoStart;

	// Looking for all different auto start objects
	for (int i = 0; i < ARRAYSIZE(autoStartName); i++) {
		autoStart = room.findObject(autoStartName[i]);
		if (autoStart) {
			_vm->_inter->interpret(autoStart->getScripts(kObjectVerbUse), true);
			has = true;
		}
	}

	return has;
}

ObjectVerb Events::cursorModeToObjectVerb(CursorMode cursorMode) {
	switch (cursorMode) {
	case kCursorModeWalk:
		return kObjectVerbGo;
	case kCursorModeUse:
		return kObjectVerbUse;
	case kCursorModeLook:
		return kObjectVerbLook;
	default:
		return kObjectVerbNone;
	}
}

} // End of namespace DarkSeed2
