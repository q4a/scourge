PROJECT (scourge)

INCLUDE (${CMAKE_ROOT}/Modules/FindOpenGL.cmake)
INCLUDE (${CMAKE_ROOT}/Modules/FindGLUT.cmake)
INCLUDE (${CMAKE_ROOT}/Modules/FindSDL.cmake)

INCLUDE_DIRECTORIES(${SDL_INCLUDE_DIR}/SDL ${OPENGL_INCLUDE_DIR} ${GLUT_INCLUDE_DIR})

#SET(CMAKE_VERBOSE_MAKEFILE "ON")

SET(CMAKE_CXX_FLAGS "-Wno-deprecated" )

ADD_EXECUTABLE (scourge WIN32 src/constants.cpp src/gui/window.cpp src/gui/widget.cpp src/gui/label.cpp src/gui/button.cpp src/gui/cardcontainer.cpp src/gui/draganddrop.cpp src/gui/scrollinglist.cpp src/gui/checkbox.cpp src/gui/multiplelabel.cpp src/rpg/rpgitem.cpp src/rpg/character.cpp src/rpg/monster.cpp src/events/event.cpp src/events/thirsthungerevent.cpp src/calendar.cpp src/date.cpp src/effect.cpp src/creature.cpp src/debugshape.cpp src/dungeongenerator.cpp src/glshape.cpp src/gltorch.cpp src/item.cpp src/location.cpp src/main.cpp src/mainmenu.cpp src/map.cpp src/Md2.cpp src/md2shape.cpp src/containergui.cpp src/scourge.cpp src/sdleventhandler.cpp src/sdlhandler.cpp src/sdlscreenview.cpp src/shape.cpp src/inventory.cpp src/shapepalette.cpp src/util.cpp src/text.cpp src/gllocator.cpp src/3ds.cpp src/3dsshape.cpp src/minimap.cpp src/optionsmenu.cpp src/userconfiguration.cpp)

TARGET_LINK_LIBRARIES(scourge ${SDL_LIBRARY} ${OPENGL_LIBRARIES} ${GLUT_LIBRARIES})

