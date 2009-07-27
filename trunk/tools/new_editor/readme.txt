This is a new map editor for Yeti3D, which should compile for any system that
supports SDL.  There is nothing wrong with the other editor that is included,
but it requires Borland C++ Builder 6, and I don't want to assume that everyone
using Yeti3D has access to a commercial compiler, or a Windows PC to use the
editor on.

I intend these tools to be cross-platform, so that no matter what OS you choose
to use, you can compile and use the tools.  I will compile and test the editor on
Debian Linux and Windows (Win2k Professional, probably Windows98 SE, and maybe XP).
Being cross-platform means that I don't have a nice GUI library to use, so some
things you're used to doing with a button or menu might require a console command.
I will do my best to make these make sense and not require too much typing.

Buttons should be similar to the original Yeti3D editor, and I based some code
on things (and also copy + pasted some of the more portable code) found in the
Yeti3D editor by Derek J. Evans and the modifications by GAZIN Matthieu.

ConsoleFont.gif is from the example program for the SDL Console library.

Console command handling stuff is based on the SDL Console example. Commandline
argument parsing uses the split.c file by Clemens Wacha, also from the SDL Console
example.

SDL_Console is available as a Debian package, but it didn't work too well for me.
I suggest downloading from http://sdlconsole.sourceforge.net and compiling it 
yourself if necessary.  I don't know if that is newer/older than the version
available for Debian, but it works beautifully for me.

Type "help" or "h" into the editor's console to get a list of valid commands and
key combinations for editing your map.
--Josh Sutherland

Things that are planned for the future but are not yet implemented:

Loading individual textures from bitmap (or jpeg, etc) files.

On-the fly recalculation of palette when new textures are added (will use the
  NeuQuant algorithm for this).
  
Saving of palette/texture files (will only be useful when the above are implemented).

Check/warn before overwriting files.  Right now, the program assumes that you know what
you are doing.

Colored lighting.  This will have to be added to the engine itself, and it is not hard
  to do (I did it once years ago but lost my source code).  There are switches in the
  cell_t structure for 3 lighting tables.

Exporting C files. This also shouldn't be hard but it's only necessary when making a game
  for a system such as the GBA that requires everything be compiled into the game binary.
