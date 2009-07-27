/*
Copyright (C) 2009 - Derek John Evans 

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
*/
#include <stdlib.h>
#include <stdio.h>
#include <SDL.h>
#include "game.h"

static SDL_Surface *viewport;
static SDL_Surface *screen;
int oldTick=0; 
int done=0; 
static yeti_t yeti;
#define title (YETI_STR_TITLE " Cross-Platform Map Editor")

void InitFirstMap();
int check_editor_events();
void draw_setup(yeti_t *yeti);
void draw_editor(yeti_t yeti, SDL_Surface *viewport, SDL_Surface *screen);
void close_up();

static int drawing_loop()
{

  int currentTick;
  int waitTicks;
  
  currentTick=SDL_GetTicks();

  waitTicks=(oldTick+(YETI_VIEWPORT_INTERVAL/2))-currentTick;
  oldTick=currentTick;

  if(waitTicks < 0) {
      game_loop(&yeti);
  }

    draw_editor(yeti, viewport, screen);
    
  return check_editor_events();
}

int main(int argc, char *argv[])
{

Uint8* keys;  
framebuffer_t framebuffer;

  if ( SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER) < 0 )
  {
    printf("Unable to init SDL: %s\n", SDL_GetError());
    exit(1);
  }
  atexit(SDL_Quit);

  /* Set up our video surface to be large enough to hold a viewport, a 2d representation of our map,
     and to show our textures. */
  screen=SDL_SetVideoMode(800,600,16,SDL_HWSURFACE|SDL_DOUBLEBUF);
  if ( screen == NULL )
  {
    printf("Unable to set %d, %d 16-bit video: %s\n",800,
               600, SDL_GetError());
    exit(1);
  }
  
  SDL_WM_SetCaption(title, NULL);
  
    /* Set up our viewport window. */  
  viewport =SDL_CreateRGBSurfaceFrom((void *)framebuffer.pixels, YETI_VIEWPORT_WIDTH, YETI_VIEWPORT_HEIGHT,
  						16, YETI_VIEWPORT_WIDTH *2, 0x001f, 0x03e0, 0x7c00, 0);
						 
  yeti_init(&yeti, &framebuffer, &framebuffer, textures, palette, lua);
  nogame_init(&yeti);
  draw_setup(&yeti);
  InitFirstMap();
  while(done==0)
  {

    done = drawing_loop();

  }
  
  close_up();
  SDL_FreeSurface(viewport);
  SDL_FreeSurface(screen);
  return 0;
}









