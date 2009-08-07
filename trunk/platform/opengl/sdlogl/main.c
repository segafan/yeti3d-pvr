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

#include <SDL.h>
#include <SDL_opengl.h>

#include "../../../src/game.h"

framebuffer_t framebuffer;
yeti_t yeti;

//GLfloat fogColor[4] = {0.0, 0.5, 0.0, 1};
GLfloat fogColor[4] = {0.5, 0.5, 0.5, 1};
unsigned texture_lists[255];
u8 texture[64][64][3];
float yeti_to_gl = 1.0 / (256 * 64);

int done = 0;

void draw_texture(yeti_t* yeti, polyclip_t src, int n, int tid)
{
  int i, x, y;

  if (texture_lists[tid])
  {
    glBindTexture(GL_TEXTURE_2D, texture_lists[tid]);
  }
  else
  {
    glGenTextures(1, &texture_lists[tid]);
    glBindTexture(GL_TEXTURE_2D, texture_lists[tid]);

    for (y = 0; y < 64; y++)
    {
      for (x = 0; x < 64; x++)
      {        
        i = textures[tid][y][x];
        texture[y][x][0] = palette[i][0];
        texture[y][x][1] = palette[i][1];
        texture[y][x][2] = palette[i][2];
      }      
    }
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); //LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);      
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 64, 64, 0, GL_RGB, GL_UNSIGNED_BYTE, texture);        
  }
  
  glBegin(n == 4 ? GL_QUADS : GL_POLYGON);
  
  for (i = n; i--;)
  {
    vertex_t* v = src[i];
    float c = 1.0 * v->l / i2f(63);
      
    glColor3f(c, c, c);
    glTexCoord2f(v->u * yeti_to_gl, v->v * yeti_to_gl);
    glVertex3f(f2fl(v->x), f2fl(v->y), -f2fl(v->z));
  }
  
  glEnd();
}

void keyboard_update(keyboard_t *keyboard)
{
SDL_Event event;  
u8 *keys;

    while ( SDL_PollEvent(&event) ) // Here, if the user presses ESCAPE, we quit.
    {
      if ( event.type == SDL_QUIT )  {  done = 1;  }

      if ( event.type == SDL_KEYDOWN )
      {
        if ( event.key.keysym.sym == SDLK_ESCAPE ) { done = 1; }

      }

    }

  keys=SDL_GetKeyState(NULL);
  keyboard->up     = keys[SDLK_UP];
  keyboard->down   = keys[SDLK_DOWN];
  keyboard->left   = keys[SDLK_LEFT];
  keyboard->right  = keys[SDLK_RIGHT];
  keyboard->a      = keys[SDLK_RCTRL]; /* Uncomment the line below if RCTRL doesn't do anything on your system. */
/*  keyboard->a      = keys[SDLK_RETURN]; */
  keyboard->b      = keys[SDLK_SPACE];
  keyboard->l      = keys[SDLK_a];
  keyboard->r      = keys[SDLK_z];

}
 
void IdleFunc()
{
  static unsigned MarkTime;
  
  if ((int)(MarkTime - SDL_GetTicks()) < 0)
  {
    MarkTime = SDL_GetTicks() + YETI_VIEWPORT_INTERVAL;

    keyboard_update(&yeti.keyboard);
   
    glClear(GL_DEPTH_BUFFER_BIT);
         
    game_loop(&yeti);
    
    glFlush();
    SDL_GL_SwapBuffers();
  }
}
  
void yeti_gl_init(GLsizei w, GLsizei h)
{
  if (!h) return;
  glViewport(0, 0, w, h);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(85.0, 1.0 * w / h, 0.1, 40.0);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

        
  glEnable(GL_TEXTURE_2D);
  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_FOG);
  glFogi (GL_FOG_MODE, GL_LINEAR);
  glFogi(GL_FOG_START, 0);
  glFogi(GL_FOG_END, 20);
  glFogfv (GL_FOG_COLOR, fogColor);
  glFogf(GL_FOG_DENSITY, 0.05);               

  glClearColor(fogColor[0], fogColor[1], fogColor[2], fogColor[3]);        
}


int main()
{
  int width = 640;
  int height = 480;
  SDL_Surface* screen;

  if ( SDL_Init(SDL_INIT_VIDEO) != 0 ) {
	printf("Unable to initialize SDL: %s\n", SDL_GetError());
	return 1;
  }
 
  SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 ); // *new*
 
  screen = SDL_SetVideoMode( YETI_VIEWPORT_WIDTH, YETI_VIEWPORT_HEIGHT, 16, SDL_OPENGL );

  SDL_WM_SetCaption("Yeti3D OpenGL/SDL Demo", NULL); // Write something a bit more interesting than "SDL App" on our window.

  palette_overbright(palette, palette, i2fdiv2(5));
    
  yeti_init(&yeti, &framebuffer, &framebuffer, textures, palette, lua);
  game_init(&yeti);
  
  yeti_gl_init(YETI_VIEWPORT_WIDTH, YETI_VIEWPORT_HEIGHT);
  
  while(!done)
  {
    IdleFunc();
  }

return 0;
}
//---------------------------------------------------------------------------

 
