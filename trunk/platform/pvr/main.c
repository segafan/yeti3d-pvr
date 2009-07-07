/*
Dreamcast PVR rendering code:
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
#include <plx/matrix.h>
#include "../../src/game.h"
#include "pvr_sprite.h"

framebuffer_t framebuffer;
yeti_t yeti;

/* Declaring the vertex buffers like this I learned from the DC video code in 
   Yabuse. It's a great resource to look at to figure out the PVR DMA stuff,
   which was necessary here since Yeti triggers the drawing routines in such a
   way that it's difficult to switch display lists when we need to. */
   
static uint8 op_vbuf[1024 * 512] __attribute__((aligned(32)));
static uint8 tr_vbuf[1024 * 256] __attribute__((aligned(32)));

pvr_ptr_t texture[YETI_TEXTURE_MAX];

static float yeti_to_gl = 1.0 / (256 * 64); /* From the OpenGL example. */

static float check_bounds(float start, float l_or_t, float r_or_b)
{
  float retval = start;
  if (l_or_t < start < r_or_b) retval = start;
  if (start < l_or_t) retval =l_or_t;
  if (start > r_or_b) retval = r_or_b;
  
  return retval;
}
/* My understanding of this method of sorting vertices for polygon rendering came
   from KGL, but since Yeti only allows a maximum of 16 vertices for a polygon,
   this might be much simpler and faster than sorting vertices each time
   we try to render a polygon--we have a list of vertex orders that should
   work every time! I do need to optimize the code that uses this table a bit
   to make it as simple and efficient as I had intended, but the idea here
   is that since we have a maximum of 16 verticies per polygon, we already know
   in which order to submit them. */
typedef int vertex_order_t;

int orders[16*16] ={
/* Number of vertices , {order} */
/* 1  */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* Unused--just here to make this easier. */
/* 2  */ 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* Unused--polygons need at least 3 vertices. */
/* 3  */ 0, 1, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/* 4 */  0, 1, 3, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
/* 5 */  0, 1, 4, 2, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/* 6 */  0, 1, 5, 2, 4, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/* 7 */  0, 1, 6, 2, 5, 3, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/* 8 */  0, 1, 7, 2, 6, 3, 5, 4, 0, 0, 0, 0, 0, 0, 0, 0,
/* 9 */  0, 1, 8, 2, 7, 3, 6, 4, 5, 0, 0, 0, 0, 0, 0, 0,
/* 10 */ 0, 1, 9, 2, 8, 3, 7, 4, 6, 5, 0, 0, 0, 0, 0, 0,
/* 11 */ 0, 1,10, 2, 9, 3, 8, 4, 7, 5, 6, 0, 0, 0, 0, 0,
/* 12 */ 0, 1,11, 2,10, 3, 9, 4, 8, 5, 7, 6, 0, 0, 0, 0,
/* 13 */ 0, 1,12, 2,11, 3,10, 4, 9, 5, 8, 6, 7, 0, 0, 0,
/* 14 */ 0, 1,13, 2,12, 3,11, 4,10, 5, 9, 6, 8, 7, 0, 0,
/* 15 */ 0, 1,14, 2,13, 3,12, 4,11, 5,10, 6, 9, 7, 8, 0,
/* 16 */ 0, 1,15, 2,14, 3,13, 4,12, 5,11, 6,10, 7, 9, 8
};

void CODE_IN_IWRAM draw_clipped_poly(yeti_t* yeti, polyclip_t src, int n, int tid)
{
  int i;
  polygon_t p;
  pvr_poly_cxt_t cxt;
  pvr_poly_hdr_t hdr;
  pvr_vertex_t vert;
  int order[16];

/* If we do not have at least 3 vertices, we do not have a polygon. */ 
  if (n < 3)
    return;
  
  pvr_poly_cxt_txr(&cxt, PVR_LIST_OP_POLY, PVR_TXRFMT_PAL8BPP, 64, 64, texture[tid], PVR_FILTER_TRILINEAR1);
  pvr_poly_compile(&hdr, &cxt);
  pvr_list_prim(PVR_LIST_OP_POLY, &hdr, sizeof(hdr));
  vert.oargb = 0;
  
 for (i=n; i--;)
   order[n-1-i] = orders[ (16* (n-1)) + i];
 
 for (i=n; i--;)
 {
   int j = order[i];
   /* Stolen from KGL to get the z value of the vertices. */
   float x = f2fl(src[j]->x), y = f2fl(src[j]->y), z = f2fl(src[j]->z), w = 1.0f;
		mat_trans_single4(x, y, z, w);				
    if (w == 1.0f)
      p[i].z = fl2f((0.5f * z) + 0.5f);
   else
      p[i].z = fl2f(w);
   
   p[i].x = src[j]->sx;
   p[i].y = src[j]->sy;
   p[i].u = src[j]->u;
   p[i].v = src[j]->v;
   p[i].l = src[j]->l;
    
 }

  if (
    tid == YETI_TEXTURE_SKY ||
    tid == YETI_TEXTURE_WINDOW ||
    tid == YETI_TEXTURE_WATER ||
    tid == YETI_TEXTURE_LAVA)
  {
    for (i = n; i--;) p[i].l = i2f(32);
  }

/* Draw the polygon */
  for (i=n; i--;) 
  {
    float c = 1.0 * p[i].l / i2f(63);
    
    vert.argb = PVR_PACK_COLOR(1.0f, c, c, c);
    
    /* We should probably check the values of vert.x and vert.y against the viewport. */
    vert.x = check_bounds(f2fl(p[i].x), YETI_VIEWPORT_X1, YETI_VIEWPORT_X2);
    vert.y = check_bounds(f2fl(p[i].y), YETI_VIEWPORT_Y1, YETI_VIEWPORT_Y2);
    vert.z = 1.0 / f2fl(p[i].z);
    vert.u = p[i].u * yeti_to_gl;
    vert.v = p[i].v * yeti_to_gl;
    
    vert.flags = (i) ? PVR_CMD_VERTEX : PVR_CMD_VERTEX_EOL ;
    
    pvr_list_prim(PVR_LIST_OP_POLY, &vert, sizeof(vert));
  }
}

#ifdef __PATCH_DRAW_TEXTURE__
/* This is disabled in the makefile--near-Z clipping either has no effect,
  I'm doing it improperly, or we just need a more accurate clipping algorithm?
  Define -D__PATCH_DRAW_TEXTURE__ in the makefile to try this code out.
*/
void CODE_IN_IWRAM draw_texture(yeti_t* yeti, polyclip_t p, int n, int tid)
{
  int i;

  for (i = n; i--;)
  {
    if (
       (p[i]->x + p[i]->z) < 0 ||
       (p[i]->z - p[i]->x) < 0 ||
       (p[i]->y + p[i]->z) < 0 ||
       (p[i]->z - p[i]->y) < 0 ||
       (p[i]->z < 512)   ) /* Punting here a little bit... Should probably just use floating point on DC.*/
    {
      polyclip_t a, b;

      for (i = n; i--; p[i]->d = p[i]->x + p[i]->z); n = polygon_clip(yeti, a, p, n); // Left
      for (i = n; i--; a[i]->d = a[i]->z - a[i]->x); n = polygon_clip(yeti, b, a, n); // Right
      for (i = n; i--; b[i]->d = b[i]->y + b[i]->z); n = polygon_clip(yeti, a, b, n); // Top
      for (i = n; i--; a[i]->d = a[i]->z - a[i]->y); n = polygon_clip(yeti, b, a, n); // Bottom
      for (i = n; i--; b[i]->d = b[i]->z          ); n = polygon_clip(yeti, a, b, n); // Near plane

      p = a;
      
      break;
    }
  }
  if (n > 2) draw_clipped_poly(yeti, p, n, tid);
}
#endif
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
    
   /* The cool thing here is that we don't have to worry about which display list
      to open/close, which display list is open, or any of that when using PVR DMA.
      We submit everything with pvr_list_prim instead of pvr_prim and things get
      submitted in the right order automatically. */    
    
    game_loop(&yeti);
    
    /* Done drawing! */
    pvr_scene_finish();
    
  }
}

#define PACK_ARGB8888(a,r,g,b) ( ((a & 0xFF) << 24) | ((r & 0xFF) << 16) | ((g & 0xFF) << 8) | (b & 0xFF) )
/* Thanks to BlackAura for the above macro and information on using paletted textures! */
void LoadPalette()
{
 int i;
 for (i=0; i<256; i++)
 {
  pvr_set_pal_entry(i, PACK_ARGB8888(255, palette[i][0], palette[i][1], palette[i][2]));
 }
}

void LoadTextures()
{
 int i;
  for (i=0; i< YETI_TEXTURE_MAX; i++)
  {
    texture[i] = pvr_mem_malloc(64*64);
    pvr_txr_load_ex(textures[i], texture[i], 64, 64, PVR_TXRLOAD_8BPP);
  }

}

void LoadSprites()
{
  sprite_to_texture(spr_00);
  sprite_to_texture(spr_01);
  sprite_to_texture(spr_02);
  sprite_to_texture(spr_03);
  sprite_to_texture(spr_ball1);
}

pvr_init_params_t params = {
	/* Enable opaque and translucent polygons with size 16 */
	{ PVR_BINSIZE_16, PVR_BINSIZE_0, PVR_BINSIZE_16, PVR_BINSIZE_0, PVR_BINSIZE_0 },

	/* Vertex buffer size */
	384*1024,
	
	1, /* Enable PVR DMA */
	0 /* No fsaa */
};

int main(int argc, char **argv)
{

 int done = 0;
 
  /* vid_set_mode(DM_320x240, PM_RGB565); */ /* To use this display size, you must
  					change values in viewports.h! */
  
  pvr_init(&params); /* Initialize the pvr using the params specified above. */
  
  pvr_set_vertbuf(PVR_LIST_OP_POLY, op_vbuf, 1024 * 512);
  pvr_set_vertbuf(PVR_LIST_TR_POLY, tr_vbuf, 1024 * 256);
  
  pvr_set_pal_format(PVR_PAL_ARGB8888); /* Set up the 8bpp palette format. */
  
  palette_overbright(palette, palette, i2fdiv2(5)); /* The Yeti OpenGL example does this... */
    
  LoadPalette();
  LoadTextures();
  LoadSprites();
  
  /* Try to make a stripped-down version of the plx_mat3d stuff.
      We don't need all of libparallax (or even all of the mat3d.c stuff..).
      Maybe merge in some of the stuff up above (that came from KGL) and move it
      to a new module with the GL-like stuff in it? */
      
  plx_mat3d_init();
  plx_mat3d_mode(PLX_MAT_PROJECTION);
  plx_mat3d_identity();
  plx_mat3d_perspective(85.0f, 640.0f / 480.0f, 0.1f, 40.0f);
  plx_mat3d_mode(PLX_MAT_MODELVIEW);
  plx_mat3d_identity();
    
  yeti_init(&yeti, &framebuffer, &framebuffer, textures, palette, lua);
  game_init(&yeti);
  
  while(done==0) /* While we're not done playing, keep looping. */
  {
    IdleFunc();
  }  
  
return 0;
}
//---------------------------------------------------------------------------

 
