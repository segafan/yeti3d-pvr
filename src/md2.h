/*
md2.h
Copyright (C) 2009 - Joshua Sutherland 

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

#ifndef __MD2_H__
#define __MD2_H__

#include "yeti.h"
#include "model.h"

#ifdef __cplusplus
extern "C"{
#endif

/* md2_info_t holds important information about the model for this particular entity.
  It doesn't give us the actual md2 model, but gives us information about which animation
  frame we should draw. */
typedef struct
{
  int st_frm; /* First frame of our current animation. */
  int end_frm;  /* Last frame of our current animation. */
  int cur_frm; /* The current frame to draw. */
  skin_t *skin; /* The entity's skin--D.J.E's code only allows for one skin, period! */ 
} md2_info_t;

/* md2_get_frame(entity_t *e, char *frame_name) will scan through the frames of the entity's model and
  return the frame number of the frame with that name. */
int md2_get_frame(entity_t *e, char *frame_name);

/* This is not the version included in the original Yeti3d code by Derek Evans, but it can be
  used directly in place of his md2_draw to draw animated models better. */
void md2_draw2(entity_t* e);

#ifdef __cplusplus
};
#endif

#endif /* __MD2_H_ */
