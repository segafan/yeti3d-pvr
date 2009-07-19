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
*/
#include <malloc.h>
#include "game.h"
#include "md2.h"
/******************************************************************************/

YETI_ROM s8 cube[] =
{
  1,              // Number of vertex lists (Animation Frames)
  8,              // Number of verices per list.
  -100,   100,  100, // Vertex X, Y, Z. Range (-120..120)
   100,   100,  100,
   100,   100, -100,
  -100,   100, -100,
  -100,  -100,  100,
   100,  -100,  100,
   100,  -100, -100,
  -100,  -100, -100,
  6,              // Number of faces in the polygon mesh.
                  // Each face is (NPoints, VID, U, V) + texture id.
  4, 0, 0, 63, 1, 63, 63, 2, 63, 0, 3, 0, 0, 8,
  4, 3, 0, 63, 2, 63, 63, 6, 63, 0, 7, 0, 0, 8,
  4, 2, 0, 63, 1, 63, 63, 5, 63, 0, 6, 0, 0, 8,
  4, 0, 0, 63, 4, 63, 63, 5, 63, 0, 1, 0, 0, 8,
  4, 0, 0, 63, 3, 63, 63, 7, 63, 0, 4, 0, 0, 8,
  4, 4, 0, 63, 7, 63, 63, 6, 63, 0, 5, 0, 0, 8
};

/******************************************************************************/
extern model_t *testmd2;

int friction(int a, int b)
{
  return a - f2i((b = a * b) < 0 ? b - FIXCEIL : b + FIXCEIL);
}

void entity_friction(entity_t* e, int amount)
{
  e->xx = friction(e->xx, amount);
  e->yy = friction(e->yy, amount);
}

void entity_move_forward(entity_t* e)
{
  e->xx += fixsin(f2i(e->t)) >> 4;
  e->yy += fixcos(f2i(e->t)) >> 4;
}

void entity_move_backwards(entity_t* e)
{
  e->xx -= fixsin(f2i(e->t)) >> 4;
  e->yy -= fixcos(f2i(e->t)) >> 4;
}

void entity_turn_right(entity_t* e)
{
  e->tt += i2f(25);
}

void entity_turn_left(entity_t* e)
{
  e->tt -= i2f(25);
}

void entity_turn_towards(entity_t* e, int x, int y)
{
  int x1 = x - (e->x + fixsin(f2i(e->t + i2f(100))));
  int y1 = y - (e->y + fixcos(f2i(e->t + i2f(100))));
  int x2 = x - (e->x + fixsin(f2i(e->t - i2f(100))));
  int y2 = y - (e->y + fixcos(f2i(e->t - i2f(100))));

  if ((x1 * x1 + y1 * y1) < (x2 * x2 + y2 * y2))
  {
    entity_turn_right(e);
  }
  else
  {
    entity_turn_left(e);
  }
}

void entity_kill(entity_t* e)
{
  e->ontick = 0;
  e->onhit  = 0;
  e->visual.data = 0;
  e->radius = 0;
}

/******************************************************************************/

void entity_default(entity_t* e, int isjumping, int iscrawling)
{
  yeti_t* yeti = (yeti_t*)e->yeti;

  e->xx = friction(e->xx, 30);
  e->yy = friction(e->yy, 30);
  e->tt = friction(e->tt, 80);

  e->r = friction(e->r, 20);
  e->p = friction(e->p, 20);

  {
    cell_t* cell = &yeti->cells[f2i(e->y)][f2i(e->x)];
    int bot = cell->bot + (iscrawling ? yeti->game.crawling : yeti->game.walking);
    int top = cell->top - yeti->game.ceiling;

    if (e->z <= bot)
    {
      e->z  -= ((e->z - bot) >> 2);
      e->zz -= (e->zz >> 2);
      if (isjumping && e->z < top)
      {
        e->z = bot;
        e->zz += yeti->game.jumping;
      }
    }
    else
    {
      e->zz += yeti->game.gravity;
    }

    if (e->z >= top)
    {
      e->z -= ((e->z - top) >> 2);
    }
  }
  if (e->life < 0) entity_kill(e);
}

void guard_pain(entity_t*e);

void bullet_collision(entity_t* e1, entity_t* e2)
{
  yeti_t* yeti = (yeti_t*)e1->yeti;

  if (e2 != yeti->camera)
  {
    /* Use e->tag for info on skin and md2 frames. */
    md2_info_t *md2_info = e2->tag;
    md2_info->st_frm = md2_get_frame(e2, "pain101");
    md2_info->end_frm = md2_get_frame(e2, "pain104");
    entity_move_backwards(e2);
    md2_info->cur_frm = 0;
    e2->ontick = guard_pain;
    e2->life -= 10;
    entity_kill(e1);
     
    
  }
}

void bullet_behaviour(entity_t* e)
{
  e->onhit = bullet_collision;
  if (!e->xx || !e->yy)
  {
    entity_kill(e);
  }
}

void shoot_bullet(entity_t* e)
{
  yeti_t* yeti = (yeti_t*)e->yeti;

  entity_t* b = YETI_BULLET(yeti);

  b->t  = e->t;
  b->p  = e->p;
  b->xx = (fixsin(f2i( e->t))) + (e->xx);
  b->yy = (fixcos(f2i( e->t))) + (e->yy);
  b->zz = (fixsin(f2i(-e->p))) + (e->zz);
  b->x  = e->x;
  b->y  = e->y;
  b->z  = e->z;
  b->visual.data = spr_ball1;
  b->visual.width = 1;
  b->visual.height = 1;
  b->visual.mode = 1;
  b->radius = 16;
  b->ontick = bullet_behaviour;
}

void camera_behaviour(entity_t* e)
{
  yeti_t* yeti = (yeti_t *)e->yeti;

  if (yeti->keyboard.l) e->p -= i2f(20);
  if (yeti->keyboard.r) e->p += i2f(20);
  if (yeti->keyboard.left) entity_turn_left(e);
  if (yeti->keyboard.right) entity_turn_right(e);
  if (yeti->keyboard.up) entity_move_forward(e);
  if (yeti->keyboard.down) entity_move_backwards(e);
  entity_default(e, yeti->keyboard.b, FALSE);

  if (yeti->keyboard.a && !yeti->keyboard._a)
  {
    shoot_bullet(e);
  }
}

void guard_attack(entity_t* e1);

void guard_collision(entity_t* e1, entity_t* e2)
{
  yeti_t *yeti = (yeti_t*)e1->yeti;
   /* Use e->tag for info on skin and md2 frames. */
  if (e2==yeti->camera)
  {
    md2_info_t *md2_info = e1->tag;
    md2_info->st_frm = md2_get_frame(e1, "punch01");
    md2_info->end_frm = md2_get_frame(e1, "punch10");
    e1->ontick = guard_attack;
  }
}

void guard_behaviour(entity_t* e)
{
  yeti_t* yeti = (yeti_t *)e->yeti;
 /* Use e->tag for info on skin and md2 frames. */
  md2_info_t *md2_info = e->tag;
  e->visual.data   = testmd2;
 
  e->visual.width  = 8;
  e->visual.height = 12;
  e->radius = 200;
  e->onhit = guard_collision;
  e->ondraw = md2_draw2;
  entity_turn_towards(e, yeti->camera->x, yeti->camera->y);
  
  md2_info->st_frm = md2_get_frame(e, "run1");
  md2_info->end_frm = md2_get_frame(e, "run6");

  entity_move_forward(e);
  e->xx = friction(e->xx, 50);
  e->yy = friction(e->yy, 50);

  entity_default(e, FALSE, FALSE);
}

void guard_pain(entity_t*e)
{
  md2_info_t *md2_info = e->tag;
  /* Use e->tag for info on skin and md2 frames. */
  if (md2_info->cur_frm / 2 > md2_info->end_frm - md2_info->st_frm)
  {
    e->ontick = guard_behaviour;
    md2_info->st_frm = md2_get_frame(e, "run1");
    md2_info->end_frm = md2_get_frame(e, "run6");
  }

  e->ondraw = md2_draw2;
  entity_move_backwards(e);
  e->xx = friction(e->xx, 50);
  e->yy = friction(e->yy, 50);

  entity_default(e, FALSE, FALSE);
}

void guard_attack(entity_t* e)
{
  yeti_t* yeti = (yeti_t *)e->yeti;
  md2_info_t *md2_info = e->tag;
  /* Use e->tag for info on skin and md2 frames. */
  if (md2_info->cur_frm / 2 > md2_info->end_frm - md2_info->st_frm)
  {
    e->ontick = guard_behaviour;
    md2_info->st_frm = md2_get_frame(e, "run1");
    md2_info->end_frm = md2_get_frame(e, "run6");
  }
  e->visual.width  = 8;
  e->visual.height = 12;
  e->radius = 200;
  e->onhit = guard_collision;
  e->ondraw = md2_draw2;
  entity_turn_towards(e, yeti->camera->x, yeti->camera->y);
  entity_move_forward(e);
  e->xx = friction(e->xx, 75);
  e->yy = friction(e->yy, 75);

  entity_default(e, FALSE, FALSE);

}
extern skin_t *skin;

void game_init(yeti_t* yeti)
{
  int x, y;

  yeti->game.gravity = -16;      // Gravity force.
  yeti->game.jumping = 80;       // Jumping force.
  yeti->game.crawling = i2f(1);  // Crawling height.
  yeti->game.walking = 400;      // Walking height.
  yeti->game.ceiling = 200;      // Crouch distance.

  yeti_load_map(yeti, &e1m1);

  //yeti->overlay = spr_ball1;
  yeti->camera->ontick = camera_behaviour;

#ifndef __YETI_EDITOR__

  for (y = 0; y < YETI_MAP_HEIGHT; y++)
  {
    for (x = 0; x < YETI_MAP_WIDTH; x++)
    {
      if (yeti->cells[y][x].ent == 2)
      {
        md2_info_t *md2_info = malloc(sizeof(md2_info_t));
	entity_t *e;
	md2_info->skin = skin;
        e = yeti_entity(yeti, i2f(x), i2f(y), i2f(1), guard_behaviour);
	e->tag = md2_info;
      }
    }
  }
#endif
}


void game_draw(yeti_t* yeti)
{
  yeti_draw(yeti);
}

void game_tick(yeti_t* yeti)
{
  yeti_tick(yeti);
}

void game_loop(yeti_t* yeti)
{
  game_tick(yeti);
  game_draw(yeti);
}

