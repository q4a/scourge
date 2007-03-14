localedir = $(datadir)/locale
INCLUDES = -I$(srcdir) -I$(srcdir)/gui -I$(srcdir)/events -I$(srcdir)/rpg -I../intl -I$(top_srcdir)/intl
DEFS = -DLOCALEDIR=\"$(localedir)\" @DEFS@
LDADD = $(top_builddir)/src/common/libScourgeCommon.a \
	$(top_builddir)/src/freetype/libScourgeFreeType.a \
	$(top_builddir)/src/gui/libScourgeGui.a \
	$(top_builddir)/src/rpg/libScourgeRpg.a \
	$(top_builddir)/src/net/libScourgeNet.a \
	$(top_builddir)/src/render/libScourgeRender.a \
	$(top_builddir)/src/io/libScourgeIo.a \
	$(top_builddir)/src/sqbinding/libSqbinding.a \
	$(top_builddir)/src/sqstdlib/libSqstdlib.a \
	$(top_builddir)/src/squirrel/libSquirrel.a \
	$(top_builddir)/src/events/libScourgeEvents.a \
	$(top_builddir)/src/test/libScourgeTest.a \
	@LIBINTL@

AM_CPPFLAGS = $(BINRELOC_CFLAGS) -DBR_PTHREAD=0

SUBDIRS = common freetype gui test rpg events net render io squirrel sqstdlib sqbinding

if HAVE_WXWIDGET_LIB
  SUBDIRS += tools	
endif

bin_PROGRAMS = scourge

scourge_SOURCES = \
	configlang.cpp\
	configlang.h\
	preferences.h\
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
	mondrian.cpp\
	mondrian.h\
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
	equip.cpp\
	equip.h\
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
	pceditor.cpp\
	pceditor.h\
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
	savegamedialog.cpp\
	savegamedialog.h\
	donatedialog.cpp\
	donatedialog.h\
	texteffect.cpp\
	texteffect.h\
	traindialog.cpp\
	traindialog.h\
	upload.cpp\
	upload.h\
	characterinfo.cpp\
	characterinfo.h\
	skillsview.cpp\
	skillsview.h

