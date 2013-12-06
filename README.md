Project Anarchy: 2D Toolset Sample (Alpha)
==========================================

![alt text](http://www.projectanarchy.com/sites/default/files/Project%20Anarchy%20Logo.png "Project Anarchy")

**IMPORTANT:** This is still very much in the alpha stage, so please be patient with us as we work out the kinks.

Although you can make 2D games with Project Anarchy out-of-the-box, it takes some custom code to get spritesheets working and doing basic
2D collision through LUA. This sample 2D toolset gives you a couple new entities and utilities to create 2D games entirely through LUA without
having to write any custom C++ code. There is still a lot of work left before this can be considered polished, but this is the first step
and we wanted to give you, the community, a first look at this so that you can provide us with feedback early on.

Features
--------

- Adds two new entities: Sprite and 2D Camera
- Automated sprite generation using [Shoebox][1]
- Runtime playback of spritesheets
- Collision detection and LUA callbacks

Dependencies
------------

* [Visual Studio 2010][6] - Needed for compiling and you can use [Visual Studio 2010 Express][5]
* [Python 2.7][4] - There are numerous helper scripts that are executed automatically in Visual Studio when you build, so you need to install this and put it in your PATH
* [PyTools][3] - Optional if you want to debug Python scripts
* [Shoebox][1] - Optional if you want to generate new spritesheets

Samples
-------

* Shooter

Compiling
---------

* [Visual Studio 2010][6]
* [PyTools][3] - Optional

To build on Windows, simply open Workspace\2D_Toolset_Win32_VS2010_DX9.sln and build either Debug DLL or Dev DLL
configurations. It will automatically copy the binaries to the Anarchy SDK folder.

* 2D_Toolset_Win32_VS2010_DX9_All.sln - Includes PyTools projects
* 2D_Toolset_Win32_VS2010_DX9_C++.sln / 2D_Toolset_Win32_VS2010_DX9_C#.sln - If you have Visual Studio Express, you need to build these two separately

Generating Spritesheets
-----------------------

We have a Python script Source\BuildSystem\spritesheet.py that can be used to generate a sprite sheet. Just pass in either a folder or

Credits
-------

Special thanks to Howard Day for the explosion effects!

TODO
----

Beta:

- Finish iOS and Android port
- Add 2D physics component using Havok Physics
- Optimzied rendering that uses actual mesh buffers
- Add blending modes for Sprite entity
- Add custom shader support for Sprite entity
- Add a SetDirection LUA call for setting orientation of sprite
- Convert the Sprite management LUA code over to C++ so that you can do SpriteManager:AddSprite
- Add a Clone LUA call to Sprite entity

Wishlist:

- Add a broadphase / sweep-and-prune implementation for collision detection
- Add support for using particle effects (as a child of the Sprite entity)
- Add a 2D manipulator in the editor
- Add pixel-perfect collision detection
- Add support for Box2D
- Add a transform rule that automatically creates the sprite sheet

Reference
---------

[1]: http://renderhjs.net/shoebox/
[2]: http://www.polycount.com/forum/showthread.php?t=91554&highlight=shoebox
[3]: http://pytools.codeplex.com/
[4]: http://www.python.org/download/releases/2.7.6/
[5]: http://www.visualstudio.com/en-us/downloads#d-2010-express
[6]: http://www.visualstudio.com/