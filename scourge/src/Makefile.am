INCLUDES = -I$(srcdir) -I$(srcdir)/gui -I$(srcdir)/events -I$(srcdir)/rpg
LDADD = $(srcdir)/freetype/libScourgeFreeType.a \
	$(srcdir)/gui/libScourgeGui.a \
	$(srcdir)/rpg/libScourgeRpg.a \
	$(srcdir)/net/libScourgeNet.a \
	$(srcdir)/render/libScourgeRender.a \
	$(srcdir)/io/libScourgeIo.a \
	$(srcdir)/sqbinding/libSqbinding.a \
	$(srcdir)/sqstdlib/libSqstdlib.a \
	$(srcdir)/squirrel/libSquirrel.a \
	$(srcdir)/events/libScourgeEvents.a \
	$(srcdir)/test/libScourgeTest.a
	

AM_CPPFLAGS = $(BINRELOC_CFLAGS) -DBR_PTHREAD=0

SUBDIRS = freetype gui test rpg events net render io squirrel sqstdlib sqbinding

bin_PROGRAMS = scourge

scourge_SOURCES = \
	prefix.c\
	prefix.h\
	preferences.h\
	constants.cpp\
	constants.h\
	persist.cpp\
	persist.h\
        creature.cpp\
        creature.h\
	calendar.cpp\
	calendar.h\
	date.cpp\
	date.h\
        item.cpp\
        item.h\
	terraingenerator.cpp\
	terraingenerator.h\
	cavemaker.cpp\
	cavemaker.h\
	dungeongenerator.cpp\
	dungeongenerator.h\
	main.cpp\
	main.h\
	mainmenu.cpp\
	mainmenu.h\
	mapeditor.cpp\
	mapeditor.h\
	mapwidget.cpp\
	mapwidget.h\
	netplay.cpp\
	netplay.h\
	containergui.cpp\
	containergui.h\
	conversationgui.cpp\
	conversationgui.h\
	infogui.cpp\
	infogui.h\
	projectile.cpp\
	projectile.h\
	session.cpp\
	session.h\
	gameadapter.cpp\
	gameadapter.h\
	scourge.cpp\
	scourge.h\
	scourgeview.cpp\
	scourgeview.h\
	scourgehandler.cpp\
	scourgehandler.h\
	sdleventhandler.cpp\
	sdleventhandler.h\
	sdlhandler.cpp\
	sdlhandler.h\
	sdlscreenview.cpp\
	sdlscreenview.h\
	sound.cpp\
	sound.h\
	inventory.cpp\
	inventory.h\
	shapepalette.cpp\
	shapepalette.h\
	util.cpp\
	util.h\
	text.cpp\
	text.h\
	minimap.cpp\
	minimap.h\
	optionsmenu.cpp\
	optionsmenu.h\
	partyeditor.cpp\
	partyeditor.h\
	multiplayer.cpp\
	multiplayer.h\
	userconfiguration.cpp\
	userconfiguration.h\
	board.cpp\
	board.h\
	battle.cpp\
	battle.h\
	party.cpp\
	party.h\
	spellcaster.cpp\
	spellcaster.h\
	itemlist.cpp\
	itemlist.h\
	tradedialog.cpp\
	tradedialog.h\
	healdialog.cpp\
	healdialog.h\
	donatedialog.cpp\
	donatedialog.h\
	texteffect.cpp\
	texteffect.h\
	traindialog.cpp\
	traindialog.h\
	characterinfo.cpp\
	characterinfo.h

