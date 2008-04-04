------------------------------------------------------------------------
Tavern pack for S.C.O.U.R.G.E.
Claudio Canavese 2008 - cod@cod-web.net
------------------------------------------------------------------------
A collection of 3D objects for taverns to use in the game S.C.O.U.R.G.E.

------------------------------------------------------------------------
Installation
------------------------------------------------------------------------
Put the tavern folder in your scourge_data/objects folder
Put the images in the texture folder in your scourge_data/textures folder

------------------------------------------------------------------------
Block for map.cfg
------------------------------------------------------------------------
To test these object in game I suggest you to replace some common items
with these.
Health potion and liquid armor will let you see these models in the HQ,
near Uzudil.
The follwing block uses unique names for these object. Change them with 
the name of the object you are going to replace.

		[3ds_shape]
			name="DISH"
			path="/objects/tavern/dish.3ds"
			dimensions="1,1,0.1"
		[/3ds_shape]
		[3ds_shape]
			name="CHALICE"
			path="/objects/tavern/chalice.3ds"
			dimensions="0.5,0.5,1"
		[/3ds_shape]


------------------------------------------------------------------------
License
------------------------------------------------------------------------
This work is licensed under the Creative Commons Attribution 2.5 License. 
To view a copy of this license, visit http://creativecommons.org/licenses/by/2.5/
or send a letter to:

Creative Commons, 1
71 Second Street, Suite 300, San Francisco, 
California, 94105, USA.


------------------------------------------------------------------------
What is S.C.O.U.R.G.E.?
------------------------------------------------------------------------

S.C.O.U.R.G.E. is a cross platform, open source rogue-like game 
in the fine tradition of NetHack and Moria It sports a graphical 
front-end, similar to glHack or the Falcon's eye.
The design of the 3D UI is an attempt at the best of both worlds 
from old to new: It lets you rotate the view, zoom in/out, view special 
effects, etc with the feeling of the old-school isometric games like 
Exult or Woodward.

S.C.O.U.R.G.E. is Free Software and you can download it from the official website:
http://scourgeweb.org