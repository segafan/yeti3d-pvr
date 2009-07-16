/*
md2.c
Copyright (C) Joshua Sutherland

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
#include <string.h>
#include <stdio.h>
#include "md2.h"

/******************************************************************************/

#undef CODE_IN_IWRAM 
#define CODE_IN_IWRAM 

/* These are in model.c */

void CODE_IN_IWRAM md2_tmap(vertex_t* p, int n, YETI_ROM u16* texture, int textsize, framebuffer_t* dst);

void CODE_IN_IWRAM md2_clipped_poly(yeti_t* yeti, polyclip_t src, int n, u16* skin);

void CODE_IN_IWRAM md2_unclipped_poly(yeti_t* yeti, polyclip_t p, int n, u16* skin);

/******************************************************************************/

/* For a similar function, see the find_frame function from KamKa's Quake 2 Model Loader for Dreamcast,
   available at boob.co.uk in the Developer Tools section. */
int md2_get_frame(entity_t *e, char *frame_name)
{
   int i;
   frame_t* f;
   model_t *model = e->visual.data;
   for (i = 0; i < model->numFrames; i++)
   {
     f = (frame_t*)((int)model + model->offsetFrames + model->frameSize * i);
     if(!strcmp(frame_name, f->name))
       return i;
   }
   return 0;/* Shouldn't get here unless we're asked for a frame that doesn't exist. */
}
/******************************************************************************/

typedef struct
{
  short next;
  short triangle;
} bucket_node_t;

void md2_draw2(entity_t* e)
{
  int i, j, bid;
  polyclip_t p;
  int nnodes = 0;
  static short buckets[YETI_TRIANGLE_BUCKET_MAX];
  static bucket_node_t nodes[1000];
  static vertex_t verts[1000];
  md2_info_t *md2_info = e->tag;
  matrx_t m;
  yeti_t* yeti = e->yeti;
  model_t* model = e->visual.data;
  
  int frame_range = md2_info->end_frm - md2_info->st_frm;
  frame_t* f = (frame_t*)((int)model + model->offsetFrames + model->frameSize * (md2_info->st_frm +((md2_info->cur_frm++/2) % frame_range)));
  triangle_t* t = (triangle_t*)((int)model + model->offsetTriangles);
  textureCoordinate_t* tc = (textureCoordinate_t*)((int)model + model->offsetTexCoords);
  
  matrix_rotate_object(m, f2i(e->r), f2i(e->p), f2i(e->t)-512);

  for (i = 0; i < model->numVertices; i++)
  {
    int u = (f->vertices[i].vertex[0] * f->scale[0] + f->translate[0]) * 10;
    int w = (f->vertices[i].vertex[1] * f->scale[1] + f->translate[1]) * 10;
    int v = (f->vertices[i].vertex[2] * f->scale[2] + f->translate[2]) * 10 - 128;
    int x = f2i(m[0][0] * u + m[0][1] * v + m[0][2] * w) + e->x - yeti->camera->x;
    int y = f2i(m[1][0] * u + m[1][1] * v + m[1][2] * w) + e->z - yeti->camera->z;
    int z = f2i(m[2][0] * u + m[2][1] * v + m[2][2] * w) + e->y - yeti->camera->y;

    verts[i].x = f2i(yeti->m[0][0] * x + yeti->m[0][1] * y + yeti->m[0][2] * z);
    verts[i].y = yeti->is2d ? y : f2i(yeti->m[1][0] * x + yeti->m[1][1] * y + yeti->m[1][2] * z);
    verts[i].z = f2i(yeti->m[2][0] * x + yeti->m[2][1] * y + yeti->m[2][2] * z);

    vertex_project(&verts[i]);
  }
  for (i = 0; i < model->numTriangles; i++)
  {    
    bid = ((e->y - yeti->camera->y - verts[t[i].vertexIndices[0]].z) >> 6) +
      (YETI_TRIANGLE_BUCKET_MAX>>1);

    bid = CLAMP(bid, 0, YETI_TRIANGLE_BUCKET_MAX - 1);

    nodes[nnodes].next = buckets[bid];
    nodes[nnodes].triangle = i;
    buckets[bid] = nnodes++;
  }
  for (bid = 0; bid < YETI_TRIANGLE_BUCKET_MAX; bid++)
  {
    for (i = buckets[bid]; i; i = nodes[i].next)
    {
      for (j = 0; j < 3; j++)
      {
        p[j] = &verts[t[i].vertexIndices[j]];
        p[j]->u = (int)tc[t[i].textureIndices[j]].s * i2f(256) / model->skinWidth;
        p[j]->v = (int)tc[t[i].textureIndices[j]].t * i2f(256) / model->skinHeight;
      }
      md2_unclipped_poly(yeti, p, 3, (u16 *)md2_info->skin);
    }
    buckets[bid] = 0;
  }
}



