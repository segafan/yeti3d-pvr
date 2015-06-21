
# Introduction #
If you are using Yeti3D to write a game for a console such as the Dreamcast or GP32, you may find it helpful to test changes to your game on your PC.  This isn't hard to do using SDL.  This code was adapted from the DirectDraw example by Derek J. Evans.

# The Example Code #
## Preliminary Stuff--Includes and Global Declarations ##
You'll want to include a few files at the top of your program.  SDL.h is definitely needed here.  The file ../../src/game.h is from the Yeti demo and will let you use the functions in /src/game.c. In your own game, you will probably want to replace game.h and game.c with your own code.
```
#include <stdlib.h>
#include <stdio.h>
#include <SDL.h>
#include "../../src/game.h"
```
Then, we will need to declare a yeti\_t struct, a framebuffer, an SDL surface for drawing, and a few other things here.  oldTick is used to keep track of time.
```
framebuffer_t framebuffer;  /* This is a buffer in memory where Yeti will draw each frame. */
yeti_t yeti;  /* Our main Yeti struct--this keeps all sorts of information that Yeti needs in order to draw the game. */
int oldTick=0; /* Used to help limit the framerate. */
int done=0;    /* Keeps track of whether or not we're still playing. */
SDL_Surface *screen; /* Our main video surface */
SDL_Event event;
```

## The Main Update-and-draw Loop ##
One way to keep the game running is to just repeatedly call a loop that gets things ready to draw and then runs your game loop. Something like this may be all you need:
```
void d3d_flip() 
{

  int currentTick; /* We use this to get the number of ticks since SDL started. */
  int waitTicks;   /* How many ticks to put the program to sleep for so we don't
                     run too fast.*/

  currentTick=SDL_GetTicks();

  waitTicks=(oldTick+(YETI_VIEWPORT_INTERVAL))-currentTick;/* Figure out how long to wait... */
  oldTick=currentTick;

  /* Only loop through the game loop and draw if enough
  time has passed since the last frame.

  The viewport_to_video function takes the frame we've drawm to
  our 15-bpp 'framebuffer' in memory, converts it to 16-bit,
  and throws it onto our actual video surface.  Look at yeti.c
  to see what is going on under the hood here.
  */
  if(waitTicks>0) {
  SDL_Delay(waitTicks);
 } /* Instead of SDL_Delay, we could just only draw the loop if waitTicks <= 0. */

    game_loop(&yeti); /* This is the main game loop defined in game.c.  You might have a different function that
                                          takes different arguments here. */
      SDL_LockSurface(screen); /* Lock the screen surface so nothing else tries to access it. */
      viewport_to_video(
        (rgb555_t*)screen->pixels,
        screen->pitch,
        &yeti.viewport,
        0xf800,
        0x07e0,
        0x001f);
      SDL_UnlockSurface(screen); /* All done, so unlock. */
        SDL_Flip(screen);   /* Display the scene on our screen. */

}

```

## Main program loop ##

Here's our main loop, which is where we set up SDL and then repeatedly call our loop/draw function.
```
int main(int argc, char *argv[])
{

Uint8* keys;  /* To keep track of keypresses later on. */

  /* Initialize SDL video. */
  if ( SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER) < 0 )
  {
    printf("Unable to init SDL: %s\n", SDL_GetError());
    exit(1);
  }
  atexit(SDL_Quit);

  /* Set up our video surface to have the same width and height as our viewport. */
  screen=SDL_SetVideoMode(YETI_VIEWPORT_WIDTH,YETI_VIEWPORT_HEIGHT,16,SDL_HWSURFACE|SDL_DOUBLEBUF);
  if ( screen == NULL )
  {
    printf("Unable to set %d, %d 16-bit video: %s\n",YETI_VIEWPORT_WIDTH,
               YETI_VIEWPORT_HEIGHT, SDL_GetError());
    exit(1);
  }
 
  /* Initiallize Yeti3D with our global yeti_t struct, framebuffer, etc. */
  yeti_init(&yeti, &framebuffer, &framebuffer, textures, palette, lua);
  game_init(&yeti);


  while(done==0) /* While we're not done playing, keep looping. */
  {

  d3d_flip();

    while ( SDL_PollEvent(&event) ) /* Here, if the user presses ESCAPE, we quit. */
    {
      if ( event.type == SDL_QUIT )  {  done = 1;  }

      if ( event.type == SDL_KEYDOWN )
      {
        if ( event.key.keysym.sym == SDLK_ESCAPE ) { done = 1; }

      }

    }
  /* Check the state of the keyboard keys. This could just as easilly be changed
  to use KOS controller code in place of the keyboard on Dreamcast, or probably
  SDL_Joystick would work although I've not tried that.
  A mouse might work for some of this, too. */

  keys=SDL_GetKeyState(NULL); 
 /* Fill in our yeti's keyboard struct with the values we got from polling the keyboard. */
  yeti.keyboard.up     = keys[SDLK_UP];
  yeti.keyboard.down   = keys[SDLK_DOWN];
  yeti.keyboard.left   = keys[SDLK_LEFT];
  yeti.keyboard.right  = keys[SDLK_RIGHT];
  yeti.keyboard.a      = keys[SDLK_RCTRL];
  yeti.keyboard.b      = keys[SDLK_SPACE];
  yeti.keyboard.l      = keys[SDLK_a];
  yeti.keyboard.r      = keys[SDLK_z];
  yeti.keyboard.select = keys[SDLK_ESCAPE];

  }

  return 0;
}
```