

![http://lh3.ggpht.com/_6B2U-HqFFMc/Sm019gS_kpI/AAAAAAAAACQ/rzzXcX5L6lA/s400/YetiEdScreenshot.jpg](http://lh3.ggpht.com/_6B2U-HqFFMc/Sm019gS_kpI/AAAAAAAAACQ/rzzXcX5L6lA/s400/YetiEdScreenshot.jpg)
Adding a new entity to a map

---

# Introduction #

This is a new map editor for Yeti3D, which should compile for any system that
supports SDL.  There is nothing wrong with the other editor that is included,
except that it requires Borland C++ Builder 6 to compile it, and it doesn't seem fair to assume that everyone using Yeti3D has access to a commercial compiler, or a Windows PC to use the editor on.  You can find the source code for the new editor in the Subversion repository, in trunk/tools/new\_editor.


# Compiling #

To compile this editor for your system, you will need the following libraries:
  * SDL 1.2 (1.3 may or may not work--try if you like)
  * SDL\_image
  * SDL\_console

SDL\_Console is available as a Debian package, but it didn't work too well for me.
I suggest downloading from http://sdlconsole.sourceforge.net and compiling it
yourself if necessary.  I don't know if that is newer/older than the version
available from the Debian package repositories, but it works beautifully for me.

---

# Using the Editor #

![http://lh4.ggpht.com/_6B2U-HqFFMc/Sm01-ECOtOI/AAAAAAAAACU/ytRdEF1d5gQ/s400/YetiEdElements.jpg](http://lh4.ggpht.com/_6B2U-HqFFMc/Sm01-ECOtOI/AAAAAAAAACU/ytRdEF1d5gQ/s400/YetiEdElements.jpg)
Main elements of map editor

Buttons should be similar to the original Yeti3D editor, and I based some code
on things (and also copy + pasted some of the more portable code) found in the
Yeti3D editor by Derek J. Evans and the modifications by GAZIN Matthieu.

There are two edit modes.  The first mode we'll call "wall drawing," and the second is "selection."  In wall drawing mode, the left mouse button puts a wall in the cell you click on, and the right button clears the wall from that cell.  In selection mode, you can click or drag with the left mouse button to select one or more cells to edit.  Use Alt+M to switch between these two modes, or type editmode into the main console at the bottom right of the editor.

Type "help" or "h" into the editor's console to get a list of valid commands and
key combinations for editing your map.

To save your map, you can press Ctrl+S and type a filename when prompted.  Be sure that the folder you wish to save the file to exists, because the program will complain if it does not.  This will save the map, palette, and textures in one file.

To load your map later, you can press Ctrl+L and type in the filename when prompted.

To load/save map/palette/texture files individually, use the following commands in the console:
  * loadmap
  * savemap
  * loadpalette
  * loadtextures

(This page will be updated at some point in the future with more information on using the editor.)