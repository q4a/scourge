What is this?
-------------

This is an example application that demonstrates how to reuse the Scourge map rendering "engine" (I hesitate to use the term...) in your own projects.

Here is what the files do in this directory:
--------------------------------------------

Makefile - This is a simple makefile I use to compile the example code. Since I didn't want to complicate things by introducing autotools, you will most likely have to edit the Makefile. If you're using unix (or cygwin, etc.) all you have to do is to edit the top of the file to point to the SDL and OpenGL settings on your machine. You can get this info by compiling scourge and copy/pasting the compiler's options here. Also, the Makefile assumes that you've compiled scourge already and that its object/.a files are in ../src.

main.cpp - This file contains the main() method; it's the entrypoint to the application. It sets up Graphics object to hide the SDL/GL initialization details, creates a Preferences object and finally runs the Game.

examplepreferences.h - This file subclasses Preferences. In Scourge, user pref-s are loaded/saved from a text file. In this header file, I only implemented the basics of what you need to display a simple map, but browsing the methods should give you an idea of other possibilities. Someday this will all be documented too... khmm.

graphics.h/cpp - This class encapsulates the graphics subsystem. The setVideoMode() method changes the SDL/OpenGL video mode, initializes all the low level detail that you don't really have to worry about. The mainLoop() method is the "heart" of your application that polls for input events (mouse, keyboard, etc.) and repaints the screen. This method calls Game::drawView() and Game::handleEvent() methods where you can do your application specific drawing and event handling. The rest of the methods in this class implement the MapAdapter interface, which is how the scourge map renders its graphics. Again, someday this interface will also be documented, but for now my hope is that the method names are descriptive enough. :-)

game.h/cpp - This is where you put your custom project code. Someday the other code in the above files will be better encapsulated inside the scourge libs so all you will need is this class. Even so, you should not have to edit the above files at all. In the constructor, you first create a Shapes object. This class loads and parses the scourge properties files and lets you access the 3d objects that the map displays (called a Shape object.) Once the shapes are loaded, the code creates a Map object which represents the map of a level (or mission in scourge.) Again, I didn't want to complicate things by loading an edited map, or generated a random one, so in Game::createMap() is just create a random jumble of objects. Hopefully this gives you the basics of how to put a shape on the map. The handleEvents() method forwards incoming events to the map (so it can be rotated, zoomed, moved) and the drawView() method renders the map on screen.

How to build the example app:
-----------------------------

1. Build the scourge code as usual.
2. Edit the Makefile and make sure the settings for SDL and OpenGL are correct for your machine.
3. Run make
4. If all goes well, the executable "main" is created. (main.exe on windows) Run it with the following command line args:

./main <true|false to use the stencil buffer (shadows)> <path to the data dir. eg.: /home/joe/scourge/data>

You can use the keyboard to move the map around, z/x to zoom in/out. You can also use the mouse to move the map by moving your pointer to the edge of the screen. To rotate the map, drag the mouse while holding the middle (wheel) button. You can use the wheel to zoom in/out.

Where to go from here:
----------------------

It's really up to you! The shapes/themes, etc. are customizable via the data/world/shapes.txt file. You can subclass the Rendered... classes to create items, creatures and projectiles. The map can also render effects via the startEffect() method (search constants.h for the effect types).

You can crete a UI by using including the src/gui/*.h files. Maybe I will write another example on how to do this.

On my end I will work to better encapsulate some of the low level details (graphics initialization) inside reusable libs. If you have any questions, don't hesitate to send me an email: cctorok@yahoo.com. I'd love to hear success/failure stories of your projects. If you make improvements to the scourge classes, I'd like to integrate them into the main branch as well.

9/9/05, Gabor

