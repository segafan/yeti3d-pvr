CREDITS:
Derek J. Evans - Yeti3D engine and map editor
Gazin Mattieu(ThunderZ) - GP32 port of Yeti3D engine and modified map editor
Vortexx - Dreamcast port of Yeti3D engine
Joshua Sutherland (OneThirty8) - SDL example, Dreamcast PVR port and example
Sam Lantinga - Simple Directmedia Layer


In the OpenGL demo and the DirectDraw demo, the controls are slightly different
than in this SDL version.  This was because SDLK_RCTRL didn't seem to work on
OS X when I compiled a version for the Macintosh, and I haven't touched that code
since 2004.  So, if you're compiling for Windows or Dreamcast, you should be able
to change it back to the Control button for firing a bullet.

Controls are mapped as follows:
SDL:
UP      = move forward
DOWN = move backward
LEFT    = turn left
RIGHT  = turn right
ENTER  = fire a bullet
SPACE = jump
A         = look up
Z         = look down
ESC     = quit

Dreamcast PVR example:
DPAD UP     = move forward
DPAD DOWN   = move backward
DPAD LEFT   = turn left
DPAD RIGHT  = turn right
A           = fire a bullet
B           = jump
L TRIGGER   = look up
R TRIGGER   = look down
START       = quit

The map editor source code I included isn't the original version by Derek J. Evans, but
it is mostly his code.  It's essentially the modified version that comes with the GP32
source by Gazin Matthieu (aka ThunderZ).

In a few of the files, I had to change quad_t to Quad_t because of a conflict I encountered
when compiling on my Mac, and matrix_t has been changed to matrx_t by Vortexx because of a
conflict with KallistiOS on the Dreamcast.  The reason I used the ThunderZ's map editor is
so that you can load your own texture and palette files when editing a map.

I've noticed a bit of a glitch in the editor - when you go to change a floor, wall, or ceiling
texture, you'll often get an annoying popup (only when using the default textures) saying
"Division by Zero."  I find that this is easilly resolved by loading a new texture & palette file.

I'll be including a Dreamcast makefile for the SDL example here as well, just to show that you
can use SDL on the DC for rendering your worlds.  The SDL version I have is from 2004 and makes
use of Yeti's viewport_to_video function, which converts the 15-bit viewport to match your actual
display mode (16-bit in our case).  This works well on Windows, but I've found that function slows
things up a bit on the Dreamcast.

Please note that the DC version of the SDL demo requires a keyboard just like the Windows and Mac
versions.

I intend at some point to include some functions for loading sprites and textures from bitmap files
or some such convenient format.  Until then, sprites are stored as an array of unsigned short integers.
The first two values are the width and the height, and the rest of the array is the pixel data, stored
in BGR555 format.  Textures are 8-bit palleted images, with each palette entry being three unsigned
characters with a value for red, green, and blue.
