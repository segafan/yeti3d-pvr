/*
pvr_sprite.c
Copyright (C) 2009 - Joshua Sutherland

Contains portions of code from Yeti3d's draw.c
Copyright (C) 2003 - Derek J. Evans

This file is part of the Dreamcast PVR rendering code for the
Yeti3D Portable Engine.

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

Original Yeti Source: (C) 2003 - Derek J. Evans <derek@theteahouse.com.au>
Prepared for public release: 10/24/2003 - Derek J. Evans <derek@theteahouse.com.au>
*/

#include <kos.h>
#include "../../src/yeti.h"

#define MAX_SPRITES 20

/* A lookup table of sorts for our sprites--
   given an index, we can look here to see if we have the sprite.
   This will be useful if we create a copy of the original sprite in memory
   in case we need to redraw the one we have store in pvr memory.  Also, we
   can easily access the copy on the pvr if we no longer need that sprite
   and wish to reclaim the memory. Using arrays has a few advantage over a
   linked list, but the one downside is that the size of the array isn't
   variable at runtime. */
static struct
{
  u8 sprite_count;
  pvr_ptr_t sprite_in_pvr[MAX_SPRITES];
  sprite_t sprite_in_ram[MAX_SPRITES];
}sprite_table;

/* A 32-byte long header. It doesn't have to be 32 bytes long, but it is. */
typedef struct
{
  u16 w; /* Width Yeti thinks the sprite is */
  u16 h; /* Height Yeti thinks the sprite is. */
  u16 tx_w; /* Width the PVR thinks the sprite is. */
  u16 tx_h; /* Height the PVR thinks the sprite is. */
  u16 mode; /* Determines how the sprite will be drawn. */
  u16 synch[10]; /* So we can make sure this is a pvr-ified sprite! */
  u16 idx; /* Texture index. */
} sprite_pvr_hdr;

/* We might want to just make a copy of the first 32 bytes of the sprite in case
  people 'load' and 'unload' compiled-in sprites rather than from files on disc,
  so we can restore the sprite.  No real need to make a copy of the whole sprite. */
static void log_sprite(sprite_t sprite, pvr_ptr_t txr, sprite_pvr_hdr *spr_hdr)
{
  if (sprite_table.sprite_count <= MAX_SPRITES)
  {
   u16 *src, *dst;
   int i;
    sprite_table.sprite_in_pvr[sprite_table.sprite_count] = txr;
    spr_hdr->idx = sprite_table.sprite_count;
    
    src=(u16 *)spr_hdr; dst = (u16 *)sprite;
    for (i=0; i < 16; i++)
    {
      dst[i] = src[i];
    }
    sprite_table.sprite_count++;
  } else {
    pvr_mem_free(txr);
  }
}

static u16 valid_synch[10] = 
{
  0,1,2,3,4,5,666,7,8,9
};

static void write_synch(sprite_pvr_hdr *spr_hdr)
{
  int i;
  for (i=0; i<10; i++)
    spr_hdr->synch[i] = valid_synch[i];
}

static int check_synch(sprite_pvr_hdr *spr_hdr)
{
  int i;
  for (i=0; i<10; i++)
  {
    if (!(spr_hdr->synch[i] == valid_synch[i]))
     return -1;
  }
  return 0;
}

/* Given integer k, return the next highest power of 2.
  
  This algorithm and code to find the next highest power of 2 
  came from:
  http://en.wikipedia.org/wiki/Power_of_two
  Accessed on Tuesday, June 30 2009 at 1:53am EST. 
  
  The only modifications were to change it from C++ to C, which was
  a trivial change to make.
  
  I believe the GPL is compatible with the Creative Commons license
  governing the use of the Wikipedia article from which I borrowed
  this code. */
#ifndef CHAR_BIT
#define CHAR_BIT 8
#endif
static int nexthigher(int k) {
int i;
        if (k == 0)
                return 1;
        k--;
        for (i=1; i<sizeof(int)*CHAR_BIT; i<<=1)
                k = k | k >> i;
        return k+1;
}

/* Draw the sprite to the screen--
   Sprites must be copied and indexed before this function will draw them.
   See sprite_to_texture(sprite_t sprite) below.
   Sprites can be drawn in 3 modes.
   
   Mode 0 is transparent/opaque.
   Mode 1 is transparent/translucent.
   Mode 2 is translucent, no full transparency.
   
   To perform this, we use sprites with 1-bit alpha.
   Mode 0 could theoretically be drawn in the punch-thru list if speed were a
   concern, but for the sake of simplicity we will use the transparent list
   (for now, at least).
   
   Mode 1 is the same as Mode 2, except you can see through the sprite.  We just
   set the alpha value on each vertex to a number less than 1.0f and away we go.
   
   Mode 2 is the same as Mode 1 in that the sprite is drawn semi-transparent, but
   the alpha value is ignored.  If you read in the function below, immediately after
   compiling the polygon header, we change one value before submitting the header.
   We set cxt.txr.alpha = PVR_TXRALPHA_DISABLE, overriding the normal value, which is
   PVR_TXRALPHA_DISABLE.  This allows us to keep only one copy of the sprite in memory
   on the PVR and still support all 3 sprite-drawing modes allowed for in the original
   Yeti3D code.
   
   */
void draw_sprite(yeti_t* yeti, vertex_t a, vertex_t b,
  YETI_ROM sprite_t sprite,
  int cl, int ct, int cr, int cb, int mode) 
{
  int uu, vv;
  sprite_pvr_hdr *spr_hdr;
  pvr_poly_cxt_t cxt;
  pvr_poly_hdr_t hdr;
  pvr_vertex_t vert;
  pvr_ptr_t txr;

  float zval[4];
  int x1, y1, u1, v1;
  int x2, y2, u2, v2; 
  x1 = f2i(a.sx); y1 = f2i(a.sy); u1 = a.u; v1 = a.v;
  x2 = f2i(b.sx); y2 = f2i(b.sy); u2 = b.u; v2 = b.v; 
  
  if ((x2 - x1) <= 0 || (y2 - y1) <=0) return;
  
    uu = f2i((u2 - u1) * reciprocal[x2 - x1]);
    vv = f2i((v2 - v1) * reciprocal[y2 - y1]);
    
  u1 = i2f(u1);
  v1 = i2f(v1);
  u2 = i2f(u2);
  v2 = i2f(v2);

  if (x1 < cl) {u1 += (cl - x1) * uu; x1 = cl;}
  if (y1 < ct) {v1 += (ct - y1) * vv; y1 = ct;}
  if (x2 > cr) {u2 -= (x2 - cr) * uu; x2 = cr;}
  if (y2 > cb) {v2 -= (y2 - cb) * uu; y2 = cb;}
  
  if ((x2 - x1) <= 0 || (y2 - y1) <=0) return;
   
  u1 = f2i(u1);
  v1 = f2i(v1);
  u2 = f2i(u2);
  v2 = f2i(v2);
  
  spr_hdr = (sprite_pvr_hdr *)sprite;
  if (check_synch(spr_hdr) < 0 ) return; /* We don't have a pvr-ified sprite. */
  
  /* Calculate Z values for all corners of the sprite. */
   float x = f2fl(a.x), y = f2fl(a.y), z = f2fl(a.z), w = 1.0f;
  		mat_trans_single4(x, y, z, w);				
    if (w == 1.0f) zval[0] = (0.5f * z) + 0.5f; else zval[0] = w;
    
    x = f2fl(b.x); y = f2fl(a.y); z = f2fl(a.z); w = 1.0f;
  		mat_trans_single4(x, y, z, w);				
    if (w == 1.0f) zval[1] = (0.5f * z) + 0.5f; else zval[1] = w;
    
    x = f2fl(a.x); y = f2fl(b.y); z = f2fl(b.z); w = 1.0f;
  		mat_trans_single4(x, y, z, w);				
    if (w == 1.0f) zval[2] = (0.5f * z) + 0.5f; else zval[2] = w;
    
    x = f2fl(b.x); y = f2fl(b.y); z = f2fl(b.z); w = 1.0f;
  		mat_trans_single4(x, y, z, w);				
    if (w == 1.0f) zval[3] = (0.5f * z) + 0.5f; else zval[3] = w;  
   
  
  /* Values should be good.  Let's find our sprite texture and begin drawing. */
  txr = sprite_table.sprite_in_pvr[spr_hdr->idx];
  
  pvr_poly_cxt_txr(&cxt, PVR_LIST_TR_POLY, PVR_TXRFMT_ARGB1555, spr_hdr->tx_w, spr_hdr->tx_h, txr, PVR_FILTER_TRILINEAR1);
  
  if (mode == 2) cxt.txr.alpha = PVR_TXRALPHA_DISABLE; /* Sprites in mode2 have no txr alpha, but are alpha blended.
  							Fortunately for us, the PVR allows this easily! */
  pvr_poly_compile(&hdr, &cxt);
  pvr_list_prim(PVR_LIST_TR_POLY, &hdr, sizeof(hdr));
  vert.oargb = 0;
  
  vert.argb = (mode) ? PVR_PACK_COLOR(0.35f, 1.0f, 1.0f, 1.0f) : PVR_PACK_COLOR(1.0f, 1.0f, 1.0f, 1.0f);
 
  /* Top left */  
  vert.x = x1;
  vert.y = y1;
  vert.z = 1.0f/zval[0];
  vert.u = (u1 > 0)? ((float)u1) / spr_hdr->tx_w : 0.0f;
  vert.v = (v1 > 0)? ((float)v1) / spr_hdr->tx_h : 0.0f;
  vert.flags = PVR_CMD_VERTEX;
  pvr_list_prim(PVR_LIST_TR_POLY, &vert, sizeof(vert));;
  
  /*Top right */
  vert.x = x2;
  vert.z = 1.0f/zval[1];
  vert.u = ((float)u2) / spr_hdr->tx_w;
  pvr_list_prim(PVR_LIST_TR_POLY, &vert, sizeof(vert));
  
  /* Bottom left */
  vert.x = x1;
  vert.y = y2;
  vert.z = 1.0f/zval[2];
  vert.u = (u1 > 0)? ((float)u1) / spr_hdr->tx_w : 0.0f;
  vert.v = ((float)v2) / spr_hdr->tx_h;
  pvr_list_prim(PVR_LIST_TR_POLY, &vert, sizeof(vert));
  
  /*Bottom right */
  vert.x = x2;
  vert.z = 1.0f/zval[3];
  vert.u = ((float)u2) / spr_hdr->tx_w;
  vert.flags = PVR_CMD_VERTEX_EOL;
  pvr_list_prim(PVR_LIST_TR_POLY, &vert, sizeof(vert));
}


/* Load a sprite in main memory to the PVR as a texture, altering both a bit:
   A texture's width and height must each be a power of 2.
   Yeti stores a sprite in BGR555 format.  The PVR does not allow this, so we need
   to convert the pixel data to the correct format, which will be ARGB1555, to allow
   for the punch-thru alpha in modes 0 and 1. */
sprite_t sprite_to_texture(sprite_t sprite)
{
  pvr_ptr_t sprite_in_pvr;
  int w, h, tx_w, tx_h;
  sprite_pvr_hdr *spr_hdr;
  sprite_t tmp;
  sprite_t src, dst;
  int i, j;
  
  w = sprite[0]; h = sprite[1];
  
  if (w*h*2 < sizeof(sprite_pvr_hdr)) return sprite;
  
  tx_w = nexthigher(w); tx_h = nexthigher(h); /* must be powers of 2. */
  
  /* Allocate what we need to allocate */
  spr_hdr = malloc(sizeof(sprite_pvr_hdr));
  sprite_in_pvr = pvr_mem_malloc(tx_w * tx_h * 2);
  tmp = malloc(tx_w * tx_h * 2);
    
  /* Clear the tmp space to all zeroes. */
  for (i=0; i< tx_w * tx_h; i++)
  {
    tmp[i] = 0x0000;
  }
  
  /* Manipulate our sprite's pixel data. */
  src = sprite; dst = tmp;
  src+=2; /* skip over w and h, which we have. */
  for (j=0; j< h; j++)
  {
    for (i=0; i<w; i++)
    {
      u16 color = src[0];
      u16 r, g, b;
      if (color)
      {
        r = RGB_RED(color); g = RGB_GREEN(color); b = RGB_BLUE(color);
        dst[i] = ( (1<<15)|(r<<10)|(g<<5)|(b) );
      }
      src++;
    }
    dst +=tx_w;
  }
  /* Put together our sprite header. */
  spr_hdr->w = w;
  spr_hdr->h = h;
  spr_hdr->tx_w = tx_w;
  spr_hdr->tx_h = tx_h;
  spr_hdr->mode = 0;
  write_synch(spr_hdr);
  
  /* Copy pixel data to the PVR */
  pvr_txr_load_ex((void *)tmp, sprite_in_pvr, tx_w, tx_h, PVR_TXRLOAD_16BPP);
  
  /* Log the sprite, etc. */
 log_sprite(sprite, sprite_in_pvr, spr_hdr);
 
  /* Clean up after ourselves. */ 
  free(spr_hdr);
  free(tmp);
  return sprite;
}

