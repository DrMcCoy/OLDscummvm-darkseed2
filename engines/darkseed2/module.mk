MODULE := engines/darkseed2

MODULE_OBJS := \
	darkseed2.o \
	detection.o \
	options.o \
	versionformats.o \
	resources.o \
	palette.o \
	imageconverter.o \
	font.o \
	sprite.o \
	graphics.o \
	graphicalobject.o \
	cursors.o \
	sound.o \
	music.o \
	variables.o \
	datfile.o \
	script.o \
	objects.o \
	room.o \
	roomconfig.o \
	talk.o \
	conversation.o \
	conversationbox.o \
	conversationboxwindows.o \
	conversationboxsaturn.o \
	inventory.o \
	inventorybox.o \
	cpk_decoder.o \
	movie.o \
	pathfinder.o \
	mike.o \
	inter.o \
	events.o \
	saveable.o \
	saveload.o

# This module can be built as a plugin
ifeq ($(ENABLE_DARKSEED2), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk
