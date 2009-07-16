/*
Copyright (C) 2003 - Derek John Evans 

This file is part of Yeti3D Portable Engine

Yeti3D is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

Original Source: 2003 - Derek J. Evans <derek@theteahouse.com.au>
Prepared for public release: 10/24/2003 - Derek J. Evans <derek@theteahouse.com.au>
*/

/*
** Name: Yeti3D
** Desc: Portable GameBoy Advanced 3D Engine
** Auth: Derek J. Evans <derek@theteahouse.com.au>
**
** Copyright (C) 2003 Derek J. Evans. All Rights Reserved.
**
** YY  YY EEEEEE TTTTTT IIIIII 33333  DDDDD
** YY  YY EE       TT     II       33 DD  DD
**  YYYY  EEEE     TT     II     333  DD  DD
**   YY   EE       TT     II       33 DD  DD
**   YY   EEEEEE   TT   IIIIII 33333  DDDDD
**
**
**  This demo using SDL was made by OneThirty8, but is basically a
**  modification of the DirectDraw demo by Derek J. Evans.  The idea here
**  is to do basically the same thing, but also to be portable to
**  Mac OSX and Linux.  Surprisingly, this runs fairly well on the
**  Dreamcast without using the 3D hardware.
**
*/
#include <stdlib.h>
#include <stdio.h>
#include <SDL.h>
#include <SDL_image.h>
#include "model.h"
#include "game.h"
#include "extra.h"

framebuffer_t framebuffer; //gives us somewhere to draw our stuff.
yeti_t yeti;
int oldTick=0; // Used to help limit the framerate.
int done=0;    // Keeps track of whether or not we're still playing.
SDL_Surface *screen; // our video surface
SDL_Event event;
model_t *testmd2;
skin_t skin;

#define title (YETI_STR_TITLE " SDL Demo " YETI_STR_VERSION " - " YETI_STR_COPYRIGHT)

void d3d_flip() // checks whether we're ready to draw a frame, and if so it draws it.
{

  int currentTick; //We use this to get the number of ticks since SDL started.
  int waitTicks;   /*How many ticks to put the program to sleep for so we don't
                     run too fast.*/

 //This little framerate limiter is taken from Jasper Berlijn's flxplay.
  currentTick=SDL_GetTicks();

 /*If this runs too slow for your liking, comment out the 3 lines following this comment.
  I'm slowing it down here simply because the bad guys were a bit to quick for me
  on my PC.  =P  */
#ifndef _arch_dreamcast
     oldTick+=YETI_VIEWPORT_INTERVAL;
#endif
  waitTicks=(oldTick+(YETI_VIEWPORT_INTERVAL))-currentTick;//...figure out how long to wait...
  oldTick=currentTick;

  /* Only loop through the game loop and draw if enough
  time has passed since the last frame.

  The viewport_to_video function takes the frame we've drawm to
  our 15-bpp 'framebuffer' in memory, converts it to 16-bit,
  and throws it onto our actual video surface.  Look at yeti.c
  to see what is going on under the hood here.
  */
  if(waitTicks>0) {
  SDL_Delay(waitTicks);}

    game_loop(&yeti);
      SDL_LockSurface(screen);
      viewport_to_video(
        (rgb555_t*)screen->pixels,
        screen->pitch,
        &yeti.viewport,
        0xf800,
        0x07e0,
        0x001f);
      SDL_UnlockSurface(screen);
        SDL_Flip(screen);

}


int main(int argc, char *argv[])
{

Uint8* keys;  // to keep track of keypresses later on.
SDL_Surface *temp_skin;
SDL_Surface *convert_skin;
int i, j;
u16 *skinsrc;

  //Initialize SDL video.
  if ( SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER) < 0 )
  {
    printf("Unable to init SDL: %s\n", SDL_GetError());
    exit(1);
  }
  atexit(SDL_Quit);

  testmd2 = (model_t *)yeti_load_file("test.md2");
  printf("Opened test.md2 successfully.\r\n");
  temp_skin = IMG_Load("test.jpg");
  
  // set up our video surface to have the same width and height as our viewport
  screen=SDL_SetVideoMode(YETI_VIEWPORT_WIDTH,YETI_VIEWPORT_HEIGHT,16,SDL_HWSURFACE|SDL_DOUBLEBUF);
  if ( screen == NULL )
  {
    printf("Unable to set %d, %d 16-bit video: %s\n",YETI_VIEWPORT_WIDTH,
               YETI_VIEWPORT_HEIGHT, SDL_GetError());
    exit(1);
  }
  convert_skin = SDL_CreateRGBSurface (SDL_SWSURFACE, 256, 256, 16, 
			31, 31 << 5, 31 << 10, 1<<15);
  SDL_BlitSurface(temp_skin, NULL, convert_skin, NULL);
  skinsrc = (u16 *)convert_skin->pixels;
  for (i=0; i< 256; i++)
  {
    for (j=0; j< 256; j++)
      skin[i][j] = skinsrc[j];
     skinsrc+=256;
  }
  SDL_FreeSurface(temp_skin);
  SDL_FreeSurface(convert_skin);
#ifndef _arch_dreamcast //SDL on the Dreamcast doesn't use a Window manager, so...
  SDL_WM_SetCaption(title, NULL); // Write something a bit more interesting than "SDL App" on our window.
#endif
  yeti_init(&yeti, &framebuffer, &framebuffer, textures, palette, lua);

  game_init(&yeti);



  while(done==0) // While we're not done playing, keep looping.
  {

  d3d_flip();
    
    while ( SDL_PollEvent(&event) ) // Here, if the user presses ESCAPE, we quit.
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
  yeti.keyboard.up     = keys[SDLK_UP];
  yeti.keyboard.down   = keys[SDLK_DOWN];
  yeti.keyboard.left   = keys[SDLK_LEFT];
  yeti.keyboard.right  = keys[SDLK_RIGHT];
  yeti.keyboard.a      = keys[SDLK_RCTRL]; /* Uncomment the line below if RCTRL doesn't do anything on your system. */
/*  yeti.keyboard.a      = keys[SDLK_RETURN]; */
  yeti.keyboard.b      = keys[SDLK_SPACE];
  yeti.keyboard.l      = keys[SDLK_a];
  yeti.keyboard.r      = keys[SDLK_z];
  yeti.keyboard.select = keys[SDLK_RETURN]; /* Use this to change the model animation? */

  }

  return 0;
}









