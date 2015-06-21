
# Introduction #


In order to use the PVR to render Yeti3D, you need to set up the Dreamcast hardware.  There are some simple functions to do most of the work for you.  Below is some code for a simple game from [/platform/pvr/main.c](http://code.google.com/p/yeti3d-pvr/source/browse/trunk/platform/pvr/main.c), which is the example program for the PVR renderer for Yeti3D.

# The Example Code #


First, you need to include the necessary files. The file [../../src/game.h](http://code.google.com/p/yeti3d-pvr/source/browse/trunk/src/game.c) is from the Yeti demo and will let you use the functions in [/src/game.c](http://code.google.com/p/yeti3d-pvr/source/browse/trunk/src/game.c).  In your own game, you will probably want to replace game.h and game.c with your own code.
## Preliminary Stuff--Includes and Global Declarations ##
```
#include <kos.h>
#include "../../src/game.h"
#include "pvr_sprite.h"
#include "pvr_texture.h"
```

Declare a yeti struct, and a framebuffer.  Even though we do not actually use a framebuffer for drawing, Yeti expects one.
```
framebuffer_t framebuffer;
yeti_t yeti;
```
## Update the Keyboard ##
You will need to check some sort of input device in order to fill in Yeti's internal keyboard struct.  Using MAPLE\_FOREACH\_BEGIN / MAPLE\_FOREACH\_END is a bit lazy and probably not the best idea for use in a real game, but here's how you might fill in the keyboard\_t struct using a standard Dreamcast controller.
```
void keyboard_update(keyboard_t* kb)
{
 MAPLE_FOREACH_BEGIN(MAPLE_FUNC_CONTROLLER, cont_state_t, st)
  
  kb->up     = (st->buttons & CONT_DPAD_UP);
  kb->down   = (st->buttons & CONT_DPAD_DOWN);
  kb->left   = (st->buttons & CONT_DPAD_LEFT);
  kb->right  = (st->buttons & CONT_DPAD_RIGHT);
  kb->a      = (st->buttons & CONT_A);
  kb->b      = (st->buttons & CONT_B);
  kb->l      = (st->ltrig > 90) ? 1 : 0;
  kb->r      = (st->rtrig > 90) ? 1 : 0;
  
  if (st->buttons & CONT_START) arch_menu();
  
 MAPLE_FOREACH_END()
}

```

## The Main 'Update and Draw' Loop ##
One way to keep the game running is to just repeatedly call a loop that gets the PVR ready to draw and then runs your game loop.  Something like this may be all you need:
```
void IdleFunc()
{
  static int MarkTime;
  
  if ((int)(MarkTime - timer_ms_gettime64()) < 0)
  {
    MarkTime = timer_ms_gettime64() + YETI_VIEWPORT_INTERVAL;
   
  /* Check the controller for input. */
    keyboard_update(&yeti.keyboard);
  
 /* Wait for the pvr to be ready for a new frame, and then tell it we're ready to start drawing. */  
    pvr_wait_ready();
    pvr_scene_begin();
    
   /*  Just run your game loop in between "pvr_scene_begin();" and "pvr_scene_finish();"
        and the rest is taken care of.  You don't have to worry about the currently open display list
       or anything like that. */
    
    game_loop(&yeti);
    
    /* Done drawing! */
    pvr_scene_finish();
    
  }
}
```

## Load Some Sprites ##
We'll just use the default blue guy character and fireball bullet for our sprites.  In your own game, you could use
something else.
```
void LoadSprites()
{
  sprite_to_texture(spr_00); /* Upload spr_00 to the PVR. */
  sprite_to_texture(spr_01);
  sprite_to_texture(spr_02);
  sprite_to_texture(spr_03);
  sprite_to_texture(spr_ball1);
}
```

## Main program loop ##
Here's our main loop, which is where we set up the PVR and then repeatedly call our IdleFunc.
```
int main(int argc, char **argv)
{

 int done = 0;
 
  yeti_pvr_init();  /* Initializes the PVR, set up vertex buffers, etc... call this first! */
  
  yeti_pvr_def_pal(); /* Use the palette defined in the original Yeti source code. */
  yeti_pvr_def_tex(); /* Use the textures defined in the original Yeti source code. */
  LoadSprites();
        
  yeti_init(&yeti, &framebuffer, &framebuffer, textures, palette, lua);
  game_init(&yeti);
  
  while(done==0) /* While we're not done playing, keep looping. */
  {
    IdleFunc();
  }  
  
return 0;
}

```