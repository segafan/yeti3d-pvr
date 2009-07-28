/*
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

*/
#include <stdlib.h>
#include <stdio.h>
#include <SDL.h>
#include <SDL_console.h>
#include "split.h"
#include "../../src/yeti.h"
#include "game.h"
#include "../../src/extra.h"

static SDL_Surface *Map2D, *TexCanvas, *TexStrip;
static SDL_Rect viewrect, maprect, texrect, conrect;
static Uint32 solid_cell;
static Uint32 camera_cell;

yeti_t *sasquatch;
int tex_updated = 0;
int texture_base = 0;
int texture_selected = 0;
int num_tex = YETI_TEXTURE_MAX;

ConsoleInformation *edit_console;
extern ConsoleInformation *EID_Console;
extern ConsoleInformation *LoadSave_Console;

void Command_Handler(ConsoleInformation *console, char* command);
SDL_Event * Edit_Events(SDL_Event *event);
SDL_Event * File_Events(SDL_Event *event);
void cell_block(cell_t *cell, int issolid);

int cell_active[YETI_MAP_HEIGHT][YETI_MAP_WIDTH];

void deactivate_all_cells()
{
 int x, y;
 for (y=0; y<64; y++)
  for (x=0; x<64; x++)
    cell_active[y][x] = 0;
}

#define EDIT_MODE_WALL 0
#define EDIT_MODE_SELECT 1
int EditMouseMode;
typedef struct
{
  int dragging;
  int button;
  int startx, starty;
  int endx, endy;
  SDL_Rect rect; 
}EditMouse_t;

static EditMouse_t EditMouse;

static void MouseButtonDown()
{
  int mousex, mousey;
  u8 MouseButtons = SDL_GetMouseState(&mousex, &mousey);
 
 /* Check for map clicks */
  if ( (mousex > maprect.x) && (mousex < maprect.x + maprect.w)
    && (mousey > maprect.y) && (mousey < maprect.y + maprect.h))
  {
    mousex -= maprect.x; mousey -= maprect.y;
    if (EditMouseMode)
    {
       if (MouseButtons & SDL_BUTTON_RMASK)
       {
	 deactivate_all_cells();
	 EditMouse.button = SDL_BUTTON_RMASK;
       }else if (MouseButtons & SDL_BUTTON_LMASK){
      
         deactivate_all_cells();
	 cell_active[YETI_MAP_HEIGHT - (mousey/7)-1][mousex/7] = 1;
	 EditMouse.startx = mousex + maprect.x; EditMouse.starty = mousey + maprect.y;
         EditMouse.dragging = 1;       
         EditMouse.button = SDL_BUTTON_LMASK;
       }
     } else {
        if (MouseButtons & SDL_BUTTON_LMASK)
	{
	  cell_block(&sasquatch->cells[YETI_MAP_HEIGHT - (mousey/7)-1][mousex/7], 1);
	}else if (MouseButtons & SDL_BUTTON_RMASK)
	{
	  cell_block(&sasquatch->cells[YETI_MAP_HEIGHT - (mousey/7)-1][mousex/7], 0);
	}
     }
  } else
 /* Check for texture clicks */
  if ( (mousex > texrect.x) && (mousex < texrect.x + texrect.w)
    && (mousey > texrect.y) && (mousey < texrect.y + texrect.h))
  {
  
        int i, j;
        int start_x = 12;
        int start_y = 12;
	int x, y;
	mousex -= texrect.x; mousey -= texrect.y;
	
          for (i=0; i<3; i++)
          {
           SDL_Rect cur_rect;
           cur_rect.x = start_x;
           cur_rect.y = (i * 74)+start_y;
           cur_rect.w = cur_rect.h = 64;

             for (j=0; j<4; j++)
             {
	       if ( (mousex > cur_rect.x) && (mousex < cur_rect.x + cur_rect.w)
    			&& (mousey > cur_rect.y) && (mousey < cur_rect.y + cur_rect.h))
	       {
		texture_selected = (i*4)+j;
		tex_updated++;
		if (MouseButtons & SDL_BUTTON_LMASK)
		{
		  for(y=0; y<YETI_MAP_HEIGHT; y++)
		  {
		    for(x=0; x<YETI_MAP_WIDTH; x++)
		    {
		      if (cell_active[y][x])
		        sasquatch->cells[y][x].btx = (texture_base + texture_selected)%num_tex;
		    }
		  }
		} else if (MouseButtons & SDL_BUTTON_RMASK)
		{
		  for(y=0; y<YETI_MAP_HEIGHT; y++)
		  {
		    for(x=0; x<YETI_MAP_WIDTH; x++)
		    {
		      if (cell_active[y][x])
		        sasquatch->cells[y][x].ttx = (texture_base + texture_selected)%num_tex;
		    }
		  }
		}else if (MouseButtons & SDL_BUTTON_MMASK)
		{
		  for(y=0; y<YETI_MAP_HEIGHT; y++)
		  {
		    for(x=0; x<YETI_MAP_WIDTH; x++)
		    {
		      if (cell_active[y][x])
		        sasquatch->cells[y][x].wtx = (texture_base + texture_selected)%num_tex;
		    }
		  }
		}
		return;
	       }
               cur_rect.x +=77;
             }
          } 
	
    
    }
}

static void MouseButtonUp()
{
 
  if (EditMouse.button == SDL_BUTTON_LMASK)
  {
    int x, y;
    EditMouse.dragging = 0;
    EditMouse.button = 0;
     
    /* Check for map selection */
    if ((EditMouse.startx > maprect.x) && (EditMouse.startx < maprect.x + maprect.w)
    && (EditMouse.starty > maprect.y) && (EditMouse.starty < maprect.y + maprect.h))
    {
    if (EditMouse.rect.y < 0) EditMouse.rect.y = 0;
    if (EditMouse.rect.y > 7*(YETI_MAP_HEIGHT-1)) EditMouse.rect.y = 7*(YETI_MAP_HEIGHT - 1);
    if (EditMouse.rect.x < 0) EditMouse.rect.x = 0;
    if (EditMouse.rect.x > 7*(YETI_MAP_WIDTH-1)) EditMouse.rect.x = 7*(YETI_MAP_WIDTH - 1);
      for (y=EditMouse.rect.y; y<EditMouse.rect.y + EditMouse.rect.h; y++){
       for (x=EditMouse.rect.x; x<EditMouse.rect.x + EditMouse.rect.w; x++){
        cell_active[YETI_MAP_HEIGHT - (y/7) -1][x/7] = 1;
       }
      }
    } /* if (EditMouse.startx > ... ... ... maprect.y + maprect.h) */
    
     
  } /* if (!(MouseButtons & SDL_BUTTON_LMASK)) */
  
}

/* We only drag on the map, so we could handle most of the drag-to-activate stuff here... */
static void DragMouse()
{
  u8 MouseButtons = SDL_GetMouseState(&EditMouse.endx, &EditMouse.endy);
  if (EditMouse.dragging == 0) return;
  
  if (!(MouseButtons & SDL_BUTTON_LMASK))
  {
   EditMouse.dragging = 0;
   return;
  } 

   if ((EditMouse.startx > maprect.x) && (EditMouse.startx < maprect.x + maprect.w)
    && (EditMouse.starty > maprect.y) && (EditMouse.starty < maprect.y + maprect.h))
    {
       EditMouse.endx -= maprect.x; EditMouse.endy -= maprect.y;
       if (EditMouse.endx < 0) EditMouse.endx = 0;
       if (EditMouse.endx > maprect.w) EditMouse.endx = maprect.w;
       if (EditMouse.endy < 0) EditMouse.endy = 0;
       if (EditMouse.endy > maprect.h) EditMouse.endy = maprect.h;
       
       if (EditMouse.startx - maprect.x < EditMouse.endx)
       {
         EditMouse.rect.x = EditMouse.startx - maprect.x; EditMouse.rect.w = EditMouse.endx - (EditMouse.startx - maprect.x);
       }else{
         EditMouse.rect.x = EditMouse.endx; EditMouse.rect.w = (EditMouse.startx - maprect.x) - EditMouse.endx;
       }
       
       if (EditMouse.starty - maprect.y < EditMouse.endy)
       {
         EditMouse.rect.y = EditMouse.starty - maprect.y; EditMouse.rect.h = EditMouse.endy - (EditMouse.starty - maprect.y);
       }else{
         EditMouse.rect.y = EditMouse.endy; EditMouse.rect.h = (EditMouse.starty - maprect.y) - EditMouse.endy;
       }
       
    } /* If (EditMouse.startx > .... ) */
}

static void update_Map2D(yeti_t yeti)
{
  int x, y;
  SDL_Rect cell_rect;
  cell_t cell;
  static int camera_throb1 = 0, camera_throb2 = 30;
  Uint32 current_cell_color;
  
  camera_cell = SDL_MapRGB(Map2D->format, camera_throb1+=10&255,camera_throb2+=10&255,0);
  DragMouse();
  SDL_FillRect(Map2D, NULL, SDL_MapRGB(Map2D->format, 0,0,255));
  if (EditMouse.dragging)
  {SDL_FillRect(Map2D, &EditMouse.rect, SDL_MapRGB(Map2D->format, 255,255,255));}
  
  for (y=0; y<YETI_MAP_HEIGHT; y++)
  {
    cell_rect.y = y*7 + 1;
    cell_rect.h = cell_rect.w = 5;
    for (x=0; x<YETI_MAP_WIDTH; x++)
    {
      cell = yeti.cells[YETI_MAP_HEIGHT - y -1][x];
      cell_rect.x = x*7 + 1;
      
      if (cell_active[YETI_MAP_HEIGHT - y -1][x])
      {
        cell_rect.x -=1; cell_rect.y-=1; cell_rect.w +=2; cell_rect.h +=2;
        SDL_FillRect (Map2D, &cell_rect, 0xffff);
        cell_rect.x +=1; cell_rect.y+=1; cell_rect.w -=2; cell_rect.h -=2;
      }
      if (CELL_IS_SOLID(&cell))
      {
        SDL_FillRect(Map2D, &cell_rect, solid_cell);
      } else {
        int s;
        s = (cell.top - cell.bot) >> 4;
	  current_cell_color = SDL_MapRGB(Map2D->format,s,s,s);
        SDL_FillRect (Map2D, &cell_rect, current_cell_color);
      }
      if (cell.ent)
      {
        cell_rect.x +=2; cell_rect.y +=2; cell_rect.w = cell_rect.h = 1;
	SDL_FillRect (Map2D, &cell_rect, SDL_MapRGB(Map2D->format,0,255,0));
	cell_rect.x -=2; cell_rect.y -=2; cell_rect.w = cell_rect.h = 5;
      }
    }
  }
  
   x = f2i(yeti.camera->x);
  y = f2i(yeti.camera->y);
  cell = yeti.cells[y][x];
  cell_rect.x = x*7 + 1; cell_rect.y = (YETI_MAP_HEIGHT - y -1)*7 + 1;
  cell_rect.w = cell_rect.h = 5;
  SDL_FillRect(Map2D, &cell_rect, camera_cell); 
 
}


void MakeTexStrip()
{
  int i;
  u8 *rawpix = (u8 *)sasquatch->textures;
  color_t *texpal = sasquatch->palette;
  color_t *strippix;
  TexStrip = SDL_CreateRGBSurface (SDL_SWSURFACE, 64, 64*YETI_TEXTURE_MAX, 24,0x0000ff, 0x00ff00, 0xff0000, 0);
  strippix = (color_t *)TexStrip->pixels;
  for(i=0; i < 64*64*YETI_TEXTURE_MAX; i++)
  {
    strippix[i][0] = texpal[rawpix[i]][0];
    strippix[i][1] = texpal[rawpix[i]][1];
    strippix[i][2] = texpal[rawpix[i]][2];
  }
  
}

void UpdateTexStrip()
{
  int i;
  u8 *rawpix = (u8 *)sasquatch->textures;
  color_t *texpal = sasquatch->palette;
  color_t *strippix;
  strippix = (color_t *)TexStrip->pixels;
  for(i=0; i < 64*64*YETI_TEXTURE_MAX; i++)
  {
    strippix[i][0] = texpal[rawpix[i]][0];
    strippix[i][1] = texpal[rawpix[i]][1];
    strippix[i][2] = texpal[rawpix[i]][2];
  }
  
}


void Draw_TexCanvas()
{
  int tx_cur = texture_base;
  int i, j;
  int start_x = 12;
  int start_y = 12;
  SDL_Rect hilight;
  SDL_Rect ctr;
  ctr.x = 0; /* Current texture--offset into TexStrip. X is always 0.*/
  
  SDL_FillRect(TexCanvas, NULL, 0);
  
  hilight.w = hilight.h = 70;
  hilight.x = (start_x - 3) + ((texture_selected%4) * 77);
  hilight.y = (start_y - 3) + ((texture_selected/4) * 74);
  
  SDL_FillRect(TexCanvas, &hilight, SDL_MapRGB(TexCanvas->format, 255,255,0));
  hilight.x+=2; hilight.w-=4;
  hilight.y+=2; hilight.h-=4;
  SDL_FillRect(TexCanvas, &hilight, 0);
  
  ctr.w = ctr.h = 64;
  for (i=0; i<3; i++)
  {
    SDL_Rect cur_rect;
    cur_rect.x = start_x;
    cur_rect.y = (i * 74)+start_y;
    cur_rect.w = cur_rect.h = 64;

    for (j=0; j<4; j++)
    {
         ctr.y = tx_cur * 64;
         SDL_BlitSurface(TexStrip, &ctr, TexCanvas, &cur_rect);
	 tx_cur = (tx_cur+1)%num_tex; cur_rect.x +=77;
    }
  }
  
  tex_updated = 0;
}

void draw_editor(yeti_t yeti, SDL_Surface *viewport, SDL_Surface *screen)
{
  SDL_BlitSurface(viewport, NULL, screen, &viewrect);
  update_Map2D(yeti);
  SDL_BlitSurface(Map2D, NULL, screen, &maprect);

  if(!(edit_console->Visible == CON_OPEN))
  {
    SDL_FillRect(screen, &conrect, SDL_MapRGB(screen->format, 64,0,0));
  }
    
  CON_DrawConsole(edit_console);
  
  SDL_BlitSurface(TexCanvas, NULL, screen, &texrect);
  
  if (tex_updated)
  {
    Draw_TexCanvas();
    SDL_BlitSurface(TexCanvas, NULL, screen, &texrect);
  }
  CON_DrawConsole(EID_Console);
  CON_DrawConsole(LoadSave_Console);
  SDL_Flip(screen);
  
  SDL_FillRect(viewport, NULL, 0);
}

void draw_setup(yeti_t *yeti)
{
 SDL_Surface *Screen = SDL_GetVideoSurface();
 
 sasquatch = yeti;
 deactivate_all_cells();
 
  SDL_FillRect(Screen, NULL, SDL_MapRGB(Screen->format, 64,0,0));
  
/* Establish the areas to draw the different UI elements */					
  viewrect.x = 10;
  viewrect.y = 10;
  viewrect.w = 320;
  viewrect.h = 240;				

/* Set up our console. */
  conrect.x = 340;
  conrect.y = 470;
  conrect.w = 450;
  conrect.h = 120;
  
  edit_console = CON_Init("ConsoleFont.gif", Screen, 100, conrect);
  CON_SetExecuteFunction(edit_console, Command_Handler);
  CON_Background(edit_console, "blueguy.bmp", 420,10);
  CON_Out(edit_console, "Welcome to the Yeti3D Map Editor!");
  CON_Out(edit_console, "Copyright (C) 2009 - Joshua Sutherland");
  CON_Out(edit_console, "Built with and for the Yeti3D Portable Engine");
  CON_Out(edit_console, "Copyright (C) 2003 - Derek John Evans");

  CON_SetPrompt(edit_console, "Type command: ");
  CON_Show(edit_console);
  CON_Topmost(edit_console);
  SDL_EnableKeyRepeat(250,30);
  
/* Set up the 2D map */
  maprect.x = 340;
  maprect.y = 10;
  maprect.w = maprect.h = 64 * 7;
  Map2D = SDL_CreateRGBSurface (SDL_SWSURFACE, 64*7, 64*7, 16,0xf800, 0x7e0, 0x1f, 0);

  SDL_FillRect(Map2D, NULL, SDL_MapRGB(Map2D->format, 0,0,255));
  solid_cell = SDL_MapRGB(Map2D->format, 192,0,0);
  
/* Set up the display area for textures */
  texrect.x = 10;
  texrect.y = 260;
  texrect.w = 320;
  texrect.h = 240;
  TexCanvas = SDL_CreateRGBSurface (SDL_SWSURFACE, 320, 240, 24,0xff0000, 0x00ff00, 0x0000ff, 0);
  SDL_FillRect(TexCanvas, &texrect, SDL_MapRGB(TexCanvas->format, 0,0,0));
  MakeTexStrip();
  Draw_TexCanvas(yeti);
  SDL_BlitSurface(TexCanvas, NULL, Screen, &texrect);
  
  EditMouseMode = EditMouse.dragging = 0;
  	
}

void close_up()
{
  CON_Destroy(edit_console);
  CON_Destroy(EID_Console);
  SDL_FreeSurface(Map2D);
  SDL_FreeSurface(TexCanvas);
  SDL_FreeSurface(TexStrip);
}

int check_editor_events()
{
SDL_Event event;
int done = 0;
u8 *keys;

 while ( SDL_PollEvent(&event) )
 {
    if(!File_Events(&event))
			continue;
    if(!CON_Events(&event))
			continue;
    if(!Edit_Events(&event))
			continue;
  switch (event.type)
  {
    case SDL_QUIT:
      printf("Exiting Yeti3D Editor!\n"); done = 1;
    break;
    
    case SDL_MOUSEBUTTONDOWN:
    MouseButtonDown();
    break;
    
    case SDL_MOUSEBUTTONUP:
    MouseButtonUp();
    break;
    
    case SDL_KEYDOWN:
    switch (event.key.keysym.sym)
    {
      case SDLK_c:
         if (event.key.keysym.mod & KMOD_ALT)
	 {
	   if (edit_console->Visible == CON_OPEN)
	   {
	     CON_Topmost(NULL);
	     CON_Hide(edit_console);

	   } else {
	     CON_Topmost(edit_console);
	     CON_Show(edit_console);
	   }
	 }
      break;
      case SDLK_q:
	 if (event.key.keysym.mod & KMOD_CTRL)
	 {
	   done = 1;
	 }
      break;
      default:
      break;
    } /* switch (event.key.keysym.sym) */
    
    break; /* case SDL_KEYDOWN: */
    
    default:
    break;
  } /* switch (event.type) */

 } /* while ( SDL_PollEvent(&event) ) */
 
 
 return done;
}


/****************************************************************************************************************
  ***************************************************************************************************************
  ***************************************************************************************************************

   End of anything useful.  Below is a copy of what I moved to control_editor.c (which is probably different now). 

  ***************************************************************************************************************
  ***************************************************************************************************************
  ***************************************************************************************************************/
#if 0
void SaveMap(ConsoleInformation *console, int argc, char* argv[])
{
   if(argc > 1)
   {
      e1m2 = (rom_map_t *)malloc(sizeof(rom_map_t));
      
      if (e1m2)
      {
        yeti_save_map(sasquatch, e1m2);
        yeti_save_file((void *)e1m2,sizeof(rom_map_t), argv[1]);
	free(e1m2);
	CON_Out(console, "%s: Map saved.", argv[1]);
      }else{
        CON_Out(console, "%s: this file name/path appears to be invalid.", argv[1]);
      }
   }else{
      CON_Out(console, "usage: %s <mapfile> (<texturefile>) (<palettefile>)", argv[0]);
   }
}

void LoadMap(ConsoleInformation *console, int argc, char* argv[])
{
   if(argc > 1)
   {
      e1m2 = (rom_map_t *)yeti_load_file(argv[1]);
      if (e1m2)
      {
        yeti_load_map(sasquatch, e1m2);
	free(e1m2);
	CON_Out(console, "%s: Map loaded.", argv[1]);
      }else{
        CON_Out(console, "%s: this file name/path appears to be invalid.", argv[1]);
      }
   }else{
      CON_Out(console, "usage: %s <mapfile>", argv[0]);
   }
}

void LoadTextures(ConsoleInformation *console, int argc, char* argv[])
{
   if(argc > 1)
   {
    texture_t *newtex = (texture_t *)yeti_load_file(argv[1]);
      if (newtex)
      {
        memcpy(sasquatch->textures, newtex, 64*64*YETI_TEXTURE_MAX);
	
	UpdateTexStrip();
	tex_updated = 1;
	texture_base = 0;
	free(newtex);
	CON_Out(console, "%s loaded. Remember to load the palette as well!", argv[1]);
      }else{
        CON_Out(console, "%s: this file name/path appears to be invalid.", argv[1]);
      }
      
   }else{
      CON_Out(console, "usage: %s <texturefile>", argv[0]);
   }
}

/*Lua calculation is taken from GAZIN Matthieu's modified version of the original
   Yeti3D map editor. */
#define min(a, b) ((a < b) ? (a) : (b))
u16 RGB555 (int r, int g, int b)
{
	int	r1 = (min (r, 255) * 31) / 255;
	int	g1 = (min (g, 255) * 31) / 255;
	int	b1 = (min (b, 255) * 31) / 255;

	return (r1 )|(g1 << 5)|(b1 << 10);
}

void LoadPalette(ConsoleInformation *console, int argc, char* argv[])
{
int a,i;
   if(argc > 1)
   {
    palette_t *newpal = (palette_t *)yeti_load_file(argv[1]);
      if (newpal)
      {
        memcpy(sasquatch->palette, newpal, 256*3);
	lut_t *luaptr = sasquatch->lighting[0];
         for (a = 0; a < 64; a++)
         {
            for (i = 0; i < 256; i++)
            {
               luaptr[a][i] = RGB555(palette[i][0]*a/16,palette[i][1]*a/16,palette[i][2]*a/16);
            }
         }
	
	free(newpal);
	UpdateTexStrip();
	tex_updated = 1;
	CON_Out(console, "%s loaded. Remember to load the textures as well!", argv[1]);
      }else{
        CON_Out(console, "%s: this file name/path appears to be invalid.", argv[1]);
      }
      
   }else{
      CON_Out(console, "usage: %s <palettefile>", argv[0]);
   }
}

void Command_Handler(ConsoleInformation *console, char* command)
{
	int argc;
	char* argv[128];
	char* linecopy;

	linecopy = strdup(command);
	argc = splitline(argv, (sizeof argv)/(sizeof argv[0]), linecopy);
	if(!argc) {
		free(linecopy);
		return;
	}


       if(!strcmp(argv[0], "loadmap"))
		LoadMap(console, argc, argv);
       else if(!strcmp(argv[0], "savemap"))
		SaveMap(console, argc, argv);
       else if(!strcmp(argv[0], "loadtextures"))
		LoadTextures(console, argc, argv);
       else if(!strcmp(argv[0], "loadpalette"))
		LoadPalette(console, argc, argv);
       else if(!strcmp(argv[0], "cls"))
                {Clear_History(console); CON_UpdateConsole(console);}
       else CON_Out(console, "Invalid command entered.");
}

#endif
