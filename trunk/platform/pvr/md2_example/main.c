/*
Example using the Dreamcast PVR rendering code:
Copyright (C) 2009 - Joshua Sutherland

Based on the Yeti3D OpenGL example (../opengl/main.c.)
Copyright (C) 2003 - Derek John Evans

Contains code adapted from KGL's gldraw.c
 (c)2001 Dan Potter
*/
/*
Original Yeti3D code and OpenGL example
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
*/

#include <kos.h>
#include <jpeg/jpeg.h>
#include "game.h"
#include "pvr_sprite.h"
#include "pvr_texture.h"
#include "md2.h"
#include "extra.h"

framebuffer_t framebuffer;
yeti_t yeti;

skin_t *skin;
model_t *testmd2;
/* Updates the internal yeti keyboard.
  TODO:
  Just check one controller in case multiple controllers are plugged in.
    I was lazy and just copied the MAPLE_FOREACH thing in because it was easy.
  Check the analog stick as well.
*/
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
 
void IdleFunc()
{
  static int MarkTime;
  
  if ((int)(MarkTime - timer_ms_gettime64()) < 0)
  {
    MarkTime = timer_ms_gettime64() + YETI_VIEWPORT_INTERVAL;

    keyboard_update(&yeti.keyboard);
    
    pvr_wait_ready();
    pvr_scene_begin();
    
   /*  Just run your game loop in between
      "pvr_scene_begin();" and "pvr_scene_finish();" and the rest (sumbitting polygons to
      the right display list and all that) is taken care of. */    
    
    game_loop(&yeti);
    
    /* Done drawing! */
    pvr_scene_finish();
    
  }
}

/* Load a skin to the PVR. This assumes that the skin we have is 256 * 256. */
skin_t *IMG_Load(const char *fn)
{
  skin_t *dest;
  kos_img_t *rv;
  
  rv = malloc(sizeof(kos_img_t));
  jpeg_to_img(fn, 1, rv);
  
  dest = (skin_t *)pvr_mem_malloc(256 * 256 * 2);
  pvr_txr_load_ex(rv->data, (pvr_ptr_t)dest, 256, 256, PVR_TXRLOAD_16BPP);
  kos_img_free(rv, 1);
  
  return dest;
}

int main(int argc, char **argv)
{

 int done = 0;

  yeti_pvr_init();  /* Initialize the PVR, set up vertex buffers, etc... call this first! */
  
  yeti_pvr_def_pal(); /* Use the palette defined in the original Yeti source code. */
  yeti_pvr_def_tex(); /* Use the textures defined in the original Yeti source code. */
  sprite_to_texture(spr_ball1); /* We'll still use the fireball sprite for our bullet. */
  
  testmd2 = (model_t *)yeti_load_file("/cd/test.md2");
  if (testmd2->magic == 844121161)
  {
    skin = IMG_Load("/cd/test.jpg");
  } 
       
  yeti_init(&yeti, &framebuffer, &framebuffer, textures, palette, lua);
  game_init(&yeti);

  while(done==0) /* While we're not done playing, keep looping. */
  {
    IdleFunc();
  }  
  
return 0;
}
//---------------------------------------------------------------------------

 
