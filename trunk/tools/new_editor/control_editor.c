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

/* I am not always the best at giving filenames to source modules.  There is also
  some control-related stuff in draw_editor.c, although this file should have all
  if the file loading/saving, map editing, etc.  The stuff in draw_editor.c should
  mostly be related to drawing the GUI elements and updating the GUI, and handling
  mouse input.
*/

extern yeti_t *sasquatch;
void UpdateTexStrip();
void deactivate_all_cells();

extern int tex_updated;
extern int texture_base;
extern int texture_selected;
extern int num_tex;
extern int cell_active[YETI_MAP_WIDTH][YETI_MAP_WIDTH];
extern ConsoleInformation *edit_console;

static u8 map_name[32];
static u8 map_auth[32];
static u8 map_desc[64];

/* From the comments in SDL_Console example. */
typedef struct {
  char* commandname;
  void (*myfunc)(ConsoleInformation *console, int argc, char* argv[]);
  char* description;
} command_t;

/* This seems silly to define it like this, but it makes it easy for me to keep
  the 'help' thing complete. */
typedef struct{
  char *key;
  char *desc;
} hotkey_desc;

static hotkey_desc hotkey_table[] = {
  {"Esc", "Hide main console window"},
  {"Alt+C", "Alternate console/map edit control (Hide/show console)"},
  {"Alt+M", "Toggle mouse control between wall drawing/selection mode"},
  {"T", "Scroll textures left (shows next texture in texture list)."},
  {"Ctrl+T","Scroll textures up (shows next 4 textures)."},
  {"Shift+T","Select next texture from textures in window."},
  {"Ctrl+E", "View/hide entities (sets camera to original position as well)."},
  {"Ctrl+Q", "Exit the program"},
  {NULL,NULL}
};

static hotkey_desc mapedit_table[] ={
  {"F", "Fill selected cell(s) with walls"},
  {"G", "Clear walls from selected cell(s)"},
  {"L", "Toggle lightswitch in selected cell(s)"},
  {"U", "Raise floor in selected cell(s)"},
  {"J", "Lower floor in selected cell(s)"},
  {"I", "Raise ceiling in selected cell(s)"},
  {"K", "Lower ceiling in selected cell(s)"},
  {"Insert", "Increase floor texture id in selected cell(s)"},
  {"Delete", "Decrease floor texture id in selected cell(s)"},
  {"Home", "Increase wall texture id in selected cell(s)"},
  {"End", "Decrease wall texture id in selected cell(s)"},
  {"PgUp", "Increase ceiling texture id in selected cell(s)"},
  {"PgDn", "Decrease ceiling texture id in selected cell(s)"},
  {"E", "Place an entity in currently selected cell(s)."},
  {NULL,NULL}
};

typedef struct
{
  rom_map_t map;
  palette_t palette;
  texture_t textures[YETI_TEXTURE_MAX];
} project_t;

/****************************************************************************************************************

Performed when program starts

****************************************************************************************************************/

void InitFirstMap()
{
  strcpy((char *)map_name,(char *)e1m1.name);
  strcpy((char *)map_auth, (char *)e1m1.auth);
  strcpy((char *)map_desc, (char *)e1m1.desc);
}

/****************************************************************************************************************

Load/Save commands

****************************************************************************************************************/
void SaveMap(ConsoleInformation *console, int argc, char* argv[])
{
 
   if(argc > 1)
   {
     rom_map_t *e1m2 = (rom_map_t *)malloc(sizeof(rom_map_t));
      
      if (e1m2)
      {
          strcpy((char *)e1m2->name, (char *)map_name);
          strcpy((char *)e1m2->auth, (char *)map_auth);
          strcpy((char *)e1m2->desc, (char *)map_desc);
        yeti_save_map(sasquatch, e1m2);
        yeti_save_file((void *)e1m2,sizeof(rom_map_t), argv[1]);
	free(e1m2);
	CON_Out(console, "%s: Map saved.", argv[1]);
      }else{
        CON_Out(console, "%s: this file name/path appears to be invalid.", argv[1]);
      }
   }else{
      CON_Out(console, "usage: %s <mapfile>", argv[0]);
   }
}

void LoadMap(ConsoleInformation *console, int argc, char* argv[])
{
   if(argc > 1)
   {
      rom_map_t *e1m2 = (rom_map_t *)yeti_load_file(argv[1]);
      if (e1m2)
      {
        yeti_load_map(sasquatch, e1m2);
	  strcpy((char *)map_name, (char *)e1m2->name);
          strcpy((char *)map_auth, (char *)e1m2->auth);
          strcpy((char *)map_desc, (char *)e1m2->desc);
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
	free(newtex);
	UpdateTexStrip();
	tex_updated = 1;
	texture_base = 0;
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

void SaveProject(ConsoleInformation *console, int argc, char* argv[])
{

   if(argc == 1)
   {
     project_t *project = (project_t *)malloc(sizeof(project_t));
       if (project)
       {
        /* Save the map. */
          strcpy((char *)project->map.name, (char *)map_name);
          strcpy((char *)project->map.auth, (char *)map_auth);
          strcpy((char *)project->map.desc, (char *)map_desc);
          yeti_save_map(sasquatch, &project->map);
	  
	/* Save the palette. */
	memcpy(&project->palette, sasquatch->palette, 256*3);
	
	/* Save the textures. */
	memcpy(&project->textures, sasquatch->textures, 64*64*YETI_TEXTURE_MAX);
	
	if (yeti_save_file((void *)project,sizeof(project_t), argv[0]) > 0)
	{
	  CON_Out(console, "Project saved to: %s", argv[0]);
  	  CON_DrawConsole(console);
          SDL_Flip(SDL_GetVideoSurface());
          SDL_Delay(750);
	
	} else {
	  CON_Out(console, "Error saving project: %s may not be a valid filepath", argv[0]);
  	  CON_DrawConsole(console);
          SDL_Flip(SDL_GetVideoSurface());
          SDL_Delay(2000);
	}
	free(project);
       
       }
   }   
}

void LoadProject(ConsoleInformation *console, int argc, char* argv[])
{
int a, i;
   if(argc == 1)
   {
     project_t *project = (project_t *)yeti_load_file(argv[0]);
       if (project)
       {
         lut_t *luaptr = sasquatch->lighting[0];
        /* Load the map. */
          strcpy( (char *)map_name, (char *)project->map.name);
          strcpy( (char *)map_auth, (char *)project->map.auth);
          strcpy( (char *)map_desc, (char *)project->map.desc);
          yeti_load_map(sasquatch, &project->map);
	  
	/* Load the palette. */
	memcpy( sasquatch->palette, &project->palette, 256*3);
	
	/* Recalculate the lighting table. */
	for (a = 0; a < 64; a++)
         {
            for (i = 0; i < 256; i++)
            {
               luaptr[a][i] = RGB555(palette[i][0]*a/16,palette[i][1]*a/16,palette[i][2]*a/16);
            }
         }
	
	/* Save the textures. */
	memcpy( sasquatch->textures,&project->textures, 64*64*YETI_TEXTURE_MAX);
	  
	free(project);
	UpdateTexStrip();
	tex_updated = 1;
	texture_base = 0;
       
       }else{
        CON_Out(console, "Error loading project: %s may not be a valid filepath", argv[0]);
  	CON_DrawConsole(console);
        SDL_Flip(SDL_GetVideoSurface());
        SDL_Delay(2000);
       }
   }
}
/****************************************************************************************************************

Map edit commands.

****************************************************************************************************************/
void NameMap(ConsoleInformation *console, int argc, char* argv[])
{
   if ((argc > 1) && (argc < 5))
   {
     strcpy((char *)map_name, argv[1]);
      if (argc > 2) strcpy((char *)map_auth, argv[2]);
      if (argc > 3) strcpy((char *)map_desc, argv[3]);
   } else {
     CON_Out(console, "usage: %s <Map Name> [<Map Author>] [<Map Description>]", argv[0]);
   }
}

void MapName(ConsoleInformation *console, int argc, char* argv[])
{
  CON_NewLineConsole(console);
  CON_Out(console, "Map Name: %s", map_name);
  CON_Out(console, "Author: %s", map_auth);
  CON_Out(console, "Description: %s", map_desc);
  CON_NewLineConsole(console);
  
}

extern int EditMouseMode;

void MouseMode(ConsoleInformation *console, int argc, char* argv[])
{
   if (argc > 1)
   {
     if(!strcmp(argv[1], "wall"))
     {
       EditMouseMode = 0;
     } else if(!strcmp(argv[1], "selection"))
     {
       EditMouseMode = 1;
     } else {
       CON_Out(console, "usage: %s <wall/selection>", argv[0]);
       return;
     }
   } else {
       EditMouseMode ^= 1;
   }
   
    deactivate_all_cells();
    CON_NewLineConsole(console);
    if (EditMouseMode)CON_Out(console, "%s: mouse action set to selection mode", argv[0]);
    else CON_Out(console, "%s: mouse action set to wall drawing mode.", argv[0]);
    CON_NewLineConsole(console);
}
/****************************************************************************************************************

General/Other commands.

****************************************************************************************************************/
void ClearScreen(ConsoleInformation *console, int argc, char* argv[])
{
  Clear_History(console);
  CON_UpdateConsole(console);
}

static int ent_switch = 0;
void ToggleEntities()
{
  
  ent_switch ^=1;
  
  yeti_save_map(sasquatch, &e1m1);
  if (ent_switch)
  {
    yeti_init(sasquatch, sasquatch->viewport.front, sasquatch->viewport.back,
      sasquatch->textures, sasquatch->palette, sasquatch->lighting[0]);
    game_init(sasquatch);
  }else{
    yeti_init(sasquatch, sasquatch->viewport.front, sasquatch->viewport.back,
      sasquatch->textures, sasquatch->palette, sasquatch->lighting[0]);
    nogame_init(sasquatch);
  }
}

void ShowHelp(ConsoleInformation *console, int argc, char* argv[]);

static command_t cmd_table[] = {
  { "loadmap", LoadMap, "Load a previously saved map file from <filename>."  },
  { "savemap", SaveMap, "Save a map file to <filename>."},
  { "loadtextures", LoadTextures, "Load textures from <filename>." },
  { "loadpalette", LoadPalette, "Load texture palette from <filename>." },
  { "mapname", MapName, "Display the name, author, decription of the current map." },
  { "namemap", NameMap, "Set the name[, author, description] of the current map."},
  { "mousemode", MouseMode, "Toggle mousemode between <wall | selection>."},
  { "editmode", MouseMode, "Toggle mousemode between <wall | selection>."},
  { "clearscreen", ClearScreen, "Clear the console window history." },
  { "cls", ClearScreen, "Clear the console window history." },
  { "help", ShowHelp, "Show this help message." },
  { "h", ShowHelp, "Show this help message." },
  { NULL, NULL }
};

void ShowHelp(ConsoleInformation *console, int argc, char* argv[])
{
  command_t* cmd;
  hotkey_desc *hotkey;
  CON_NewLineConsole(console);
  CON_Out(console, "Valid typed commands:");
  for (cmd = cmd_table; cmd->commandname; cmd++)
  {
      CON_Out(console, "%s: %s",cmd->commandname, cmd->description);
  }
  
  CON_NewLineConsole(console);
  CON_Out(console, "Valid Map edit key commands:");
  for (hotkey = mapedit_table; hotkey->desc; hotkey++)
  {
      CON_Out(console, "%s: %s", hotkey->key, hotkey->desc);
  }
  
  CON_NewLineConsole(console);
  CON_Out(console, "Valid hotkey commands:");
  for (hotkey = hotkey_table; hotkey->desc; hotkey++)
  {
      CON_Out(console, "%s: %s", hotkey->key, hotkey->desc);
  }
  CON_NewLineConsole(console);
  CON_Out(console, "PgUp/PgDn to scroll through the list.");
}
/****************************************************************************************************************

The thing that processes the commands.

****************************************************************************************************************/
void Command_Handler(ConsoleInformation *console, char* command)
{
  int argc;
  char* argv[128];
  char* linecopy;
  command_t* cmd;
  
  linecopy = strdup(command);
  argc = splitline(argv, (sizeof argv)/(sizeof argv[0]), linecopy);
  if(!argc) {
    free(linecopy);
    return;
  }
		
 /* From the comments in SDL_Console example. */		
  for (cmd = cmd_table; cmd->commandname; cmd++) {
    if(!strcmp(cmd->commandname, argv[0])) {

      cmd->myfunc(console, argc, argv);
      return;
    }
  }
  
  CON_Out(console, "Invalid command entered.");

}

static int EID;
ConsoleInformation *EID_Console = NULL;
ConsoleInformation *LoadSave_Console = NULL;
void guard_behaviour(entity_t* e);

void EID_Handler(ConsoleInformation *console, char* command)
{
  int argc;
  char* argv[128];
  char* linecopy;
  int x,y;
  
  linecopy = strdup(command);
  argc = splitline(argv, (sizeof argv)/(sizeof argv[0]), linecopy);
  if(!argc) {
    CON_Out(console, "Invalid value entered.");
    CON_Out(console, "Valid values are 0 - 255.");
    CON_DrawConsole(console);
    SDL_Flip(SDL_GetVideoSurface());
    SDL_Delay(750);
    CON_Hide(console);
    free(linecopy);
    if (!(edit_console->Visible == CON_CLOSED))
      CON_Topmost(edit_console);
    return;
  }
  if(argc < 1) {
    CON_Out(console, "No value entered.");
    CON_Out(console, "Valid values are 0 - 255.");
    CON_DrawConsole(console);
    SDL_Flip(SDL_GetVideoSurface());
    SDL_Delay(750);
    CON_Hide(console);
    free(linecopy);
    if (!(edit_console->Visible == CON_CLOSED))
      CON_Topmost(edit_console);
    return;
  }
  
  EID = atoi(argv[0]);
  
  if( (EID < 0 ) || (EID > 255) )
  {
    CON_Out(console, "Invalid value entered.");
    CON_Out(console, "Valid values are 0 - 255.");
    CON_DrawConsole(console);
    SDL_Flip(SDL_GetVideoSurface());
    EID = 0;
    SDL_Delay(750);
    if (!(edit_console->Visible == CON_CLOSED))
      CON_Topmost(edit_console);
    CON_Hide(console);
    return;
  }
  
  for (y=0; y<YETI_MAP_HEIGHT; y++){
    for (x=0; x<YETI_MAP_HEIGHT; x++){
      if (cell_active[y][x]){
        sasquatch->cells[y][x].ent = EID;
	CON_Out(console, "EID %d placed @ cell[y=%d][x=%d]", EID,y,x);
	if (ent_switch){
	  entity_t *e = yeti_entity(sasquatch, i2f(x), i2f(y), i2f(1), guard_behaviour);
	  e->swi |=ENTITY_SWI_NO_COLLISION_RESPONSE;
	}
       }
    }
  }
  if (!(edit_console->Visible == CON_CLOSED))
      CON_Topmost(edit_console);
  CON_Hide(console);
}

void Save_Handler(ConsoleInformation *console, char* command)
{
  int argc;
  char* argv[128];
  char* linecopy;
  
  linecopy = strdup(command);
  argc = splitline(argv, (sizeof argv)/(sizeof argv[0]), linecopy);
  if(!argc) {
    CON_Out(console, "No Filename entered.");
    CON_DrawConsole(console);
    SDL_Flip(SDL_GetVideoSurface());
    SDL_Delay(750);
    CON_Hide(console);
    free(linecopy);
    if (!(edit_console->Visible == CON_CLOSED))
      CON_Topmost(edit_console);
    return;
  }
  
  SaveProject(console, argc, argv);
    
  if (!(edit_console->Visible == CON_CLOSED))
      CON_Topmost(edit_console);
  CON_Hide(console);
}

void Load_Handler(ConsoleInformation *console, char* command)
{
    int argc;
  char* argv[128];
  char* linecopy;
  
  linecopy = strdup(command);
  argc = splitline(argv, (sizeof argv)/(sizeof argv[0]), linecopy);
  if(!argc) {
    CON_Out(console, "No Filename entered.");
    CON_DrawConsole(console);
    SDL_Flip(SDL_GetVideoSurface());
    SDL_Delay(750);
    CON_Hide(console);
    free(linecopy);
    if (!(edit_console->Visible == CON_CLOSED))
      CON_Topmost(edit_console);
    return;
  }
  
  LoadProject(console, argc, argv);
    
  if (!(edit_console->Visible == CON_CLOSED))
      CON_Topmost(edit_console);
  CON_Hide(console);
}
/****************************************************************************************************************

Mapedit functions.

****************************************************************************************************************/
void cell_block(cell_t *cell, int issolid)
{
  if (issolid)
  {
    if (!CELL_IS_SOLID(cell))
    {
      cell->tos = cell->top;
      cell->bos = cell->bot;
      cell->top = cell->bot = 0;
    }
  }
  else
  {
    if (CELL_IS_SOLID(cell))
    {
      cell->top = cell->tos;
      cell->bot = cell->bos;      
    }
  }
}
/****************************************************************************************************************

Mapedit key commands.

****************************************************************************************************************/

SDL_Event * Edit_Events(SDL_Event *event)
{
int y, x, Key = 0;
u8 *keys;
  if(event->type == SDL_KEYDOWN) {
    switch (event->key.keysym.sym) {
     
      case SDLK_f:
         for (y=0; y<YETI_MAP_HEIGHT; y++){
	   for (x=0; x<YETI_MAP_HEIGHT; x++){
	     if (cell_active[y][x]){
	       cell_block(&sasquatch->cells[y][x], 1);
	     }
	   }
	 }
	 Key = 1;
      break;
      
      case SDLK_g:
         for (y=0; y<YETI_MAP_HEIGHT; y++){
	   for (x=0; x<YETI_MAP_HEIGHT; x++){
	     if (cell_active[y][x]){
	       cell_block(&sasquatch->cells[y][x], 0);
	     }
	   }
	 }
	 Key = 1;
      break;
      
      case SDLK_l:
       /*  if (event->key.keysym.mod & KMOD_CTRL)
        {
	    if (LoadSave_Console == NULL)
	    {
	      SDL_Rect rect;
	      SDL_Surface *Screen = SDL_GetVideoSurface();
	      rect.x = 400;
	      rect.y = 150;
	      rect.w = 200;
	      rect.h = 150;
              LoadSave_Console = CON_Init("ConsoleFont.gif", Screen, 100, rect);
	    }
	    CON_SetExecuteFunction(LoadSave_Console, Load_Handler);
	    CON_SetPrompt(LoadSave_Console, "Load Filename: ");
	    Clear_History(LoadSave_Console);
            CON_UpdateConsole(LoadSave_Console);
	    CON_Show(LoadSave_Console);
            CON_Topmost(LoadSave_Console);
	  
       } else  */{ 
         for (y=0; y<YETI_MAP_HEIGHT; y++){
	   for (x=0; x<YETI_MAP_HEIGHT; x++){
	     if (cell_active[y][x]){
	       sasquatch->cells[y][x].swi ^= CELL_SWI_LIGHT;
	     }
	   }
	 }
	 Key = 1;
       }
      break;
      
      case SDLK_u:
         for (y=0; y<YETI_MAP_HEIGHT; y++){
	   for (x=0; x<YETI_MAP_HEIGHT; x++){
	     if (cell_active[y][x]){
	       sasquatch->cells[y][x].bot += 64;
	     }
	   }
	 }
	 Key = 1;
      break;
      
      case SDLK_j:
         for (y=0; y<YETI_MAP_HEIGHT; y++){
	   for (x=0; x<YETI_MAP_HEIGHT; x++){
	     if (cell_active[y][x]){
	       sasquatch->cells[y][x].bot -= 64;
	     }
	   }
	 }
	 Key = 1;
      break;
      
      case SDLK_i:
         for (y=0; y<YETI_MAP_HEIGHT; y++){
	   for (x=0; x<YETI_MAP_HEIGHT; x++){
	     if (cell_active[y][x]){
	       sasquatch->cells[y][x].top += 64;
	     }
	   }
	 }
	 Key = 1;
      break;
      
      case SDLK_k:
         for (y=0; y<YETI_MAP_HEIGHT; y++){
	   for (x=0; x<YETI_MAP_HEIGHT; x++){
	     if (cell_active[y][x]){
	       sasquatch->cells[y][x].top -= 64;
	     }
	   }
	 }
	 Key = 1;
      break;
      case  SDLK_INSERT:
         texture_base = (texture_base-1)%num_tex;
	 if (texture_base < 0) texture_base+=num_tex;
         for (y=0; y<YETI_MAP_HEIGHT; y++){
	   for (x=0; x<YETI_MAP_HEIGHT; x++){
	     if (cell_active[y][x]){
	       sasquatch->cells[y][x].btx = ((texture_selected)%12)+ texture_base;
	     }
	   }
	 }
	 Key = 1;
      break;
      case  SDLK_DELETE:
         texture_base = (texture_base+1)%num_tex;
         for (y=0; y<YETI_MAP_HEIGHT; y++){
	   for (x=0; x<YETI_MAP_HEIGHT; x++){
	     if (cell_active[y][x]){
	       sasquatch->cells[y][x].btx = ((texture_selected)%12)+ texture_base;
	     }
	   }
	 }
	 Key = 1;
      break;
     
      case  SDLK_HOME:
         texture_base = (texture_base-1)%num_tex;
	 if (texture_base < 0) texture_base+=num_tex;
         for (y=0; y<YETI_MAP_HEIGHT; y++){
	   for (x=0; x<YETI_MAP_HEIGHT; x++){
	     if (cell_active[y][x]){
	       sasquatch->cells[y][x].wtx = ((texture_selected)%12)+ texture_base;
	     }
	   }
	 }
	 Key = 1;
	 break;
	 
      case  SDLK_END: 
      texture_base = (texture_base+1)%num_tex;
         for (y=0; y<YETI_MAP_HEIGHT; y++){
	   for (x=0; x<YETI_MAP_HEIGHT; x++){
	     if (cell_active[y][x]){
	       sasquatch->cells[y][x].wtx = ((texture_selected)%12)+ texture_base;
	     }
	   }
	 }
	 Key = 1;
      break;
      
      case  SDLK_PAGEUP: 
         texture_base = (texture_base-1)%num_tex;
	 if (texture_base < 0) texture_base+=num_tex;
         for (y=0; y<YETI_MAP_HEIGHT; y++){
	   for (x=0; x<YETI_MAP_HEIGHT; x++){
	     if (cell_active[y][x]){
	       sasquatch->cells[y][x].ttx = ((texture_selected)%12)+ texture_base;
	     }
	   }
	 }
	 Key = 1;
	 break;   
	    
      case  SDLK_PAGEDOWN: 
      texture_base = (texture_base+1)%num_tex;
         for (y=0; y<YETI_MAP_HEIGHT; y++){
	   for (x=0; x<YETI_MAP_HEIGHT; x++){
	     if (cell_active[y][x]){
	       sasquatch->cells[y][x].ttx = ((texture_selected)%12)+ texture_base;
	     }
	   }
	 }
	 Key = 1;
      break;
       
      case SDLK_t:
        if (event->key.keysym.mod & KMOD_SHIFT)
	{
	 texture_selected = (texture_selected+1)%12;
	 tex_updated++;
	 return NULL;
	}
        else if (event->key.keysym.mod & KMOD_CTRL)
	{
	 texture_base = (texture_base+4)%num_tex;
	 tex_updated++;
	 return NULL;
	}else{
         texture_base = (texture_base+1)%num_tex;
	 tex_updated++;
	 return NULL;
	}
      break;
      
      case SDLK_e:
        if (event->key.keysym.mod & KMOD_CTRL)
	{
	  ToggleEntities();
	}else{
          if (EID_Console == NULL)
	  {
	    SDL_Rect rect;
	    SDL_Surface *Screen = SDL_GetVideoSurface();
	    rect.x = 400;
	    rect.y = 150;
	    rect.w = 200;
	    rect.h = 150;
            EID_Console = CON_Init("ConsoleFont.gif", Screen, 100, rect);
  	    CON_SetExecuteFunction(EID_Console, EID_Handler);
	    CON_SetPrompt(EID_Console, "Entity ID: ");
	  }
	  Clear_History(EID_Console);
          CON_UpdateConsole(EID_Console);
	  CON_Show(EID_Console);
          CON_Topmost(EID_Console);
	  }
      break;
      
      case SDLK_m:
         if (event->key.keysym.mod & KMOD_ALT)
	 {
	   EditMouseMode ^= 1;
	   deactivate_all_cells();
	 }
      break;
      
/*       case SDLK_s:
      #ifdef devel_editor
          if (event->key.keysym.mod & KMOD_ALT)
	  {
	     SDL_SaveBMP(SDL_GetVideoSurface(), "/root/YetiEdScreenshot.bmp");
	  }else
      #endif 
          if (event->key.keysym.mod & KMOD_CTRL)
	  {
	    if (LoadSave_Console == NULL)
	    {
	      SDL_Rect rect;
	      SDL_Surface *Screen = SDL_GetVideoSurface();
	      rect.x = 400;
	      rect.y = 150;
	      rect.w = 200;
	      rect.h = 150;
              LoadSave_Console = CON_Init("ConsoleFont.gif", Screen, 100, rect);
	    }
	    CON_SetExecuteFunction(LoadSave_Console, Save_Handler);
	    CON_SetPrompt(LoadSave_Console, "Save Filename: ");
	    Clear_History(LoadSave_Console);
            CON_UpdateConsole(LoadSave_Console);
	    CON_Show(LoadSave_Console);
            CON_Topmost(LoadSave_Console);
	  }
      break; */
      
      default:
      break;
    }
  }
  
  if (Key)
  {
    tex_updated = 1;
    yeti_default_lighting(sasquatch);
    return NULL;
  }
  
  keys=SDL_GetKeyState(NULL);
  sasquatch->keyboard.up     = keys[SDLK_UP];
  sasquatch->keyboard.down   = keys[SDLK_DOWN];
  sasquatch->keyboard.left   = keys[SDLK_LEFT];
  sasquatch->keyboard.right  = keys[SDLK_RIGHT];
  sasquatch->keyboard.a      = keys[SDLK_RCTRL];
  sasquatch->keyboard.b      = keys[SDLK_SPACE];
  sasquatch->keyboard.l      = keys[SDLK_a];
  sasquatch->keyboard.r      = keys[SDLK_z];
  
 return event; 
}

SDL_Event * File_Events(SDL_Event *event)
{

  if(event->type == SDL_KEYDOWN) {
    switch (event->key.keysym.sym) {
          case SDLK_s:
      #ifdef devel_editor
          if (event->key.keysym.mod & KMOD_ALT)
	  {
	     SDL_SaveBMP(SDL_GetVideoSurface(), "/root/YetiEdScreenshot.bmp");
	     return NULL;
	  }else
      #endif 
          if (event->key.keysym.mod & KMOD_CTRL)
	  {
	    if (LoadSave_Console == NULL)
	    {
	      SDL_Rect rect;
	      SDL_Surface *Screen = SDL_GetVideoSurface();
	      rect.x = 400;
	      rect.y = 150;
	      rect.w = 200;
	      rect.h = 150;
              LoadSave_Console = CON_Init("ConsoleFont.gif", Screen, 100, rect);
	    }
	    CON_SetExecuteFunction(LoadSave_Console, Save_Handler);
	    CON_SetPrompt(LoadSave_Console, "Save Filename: ");
	    Clear_History(LoadSave_Console);
            CON_UpdateConsole(LoadSave_Console);
	    CON_Show(LoadSave_Console);
            CON_Topmost(LoadSave_Console);
	    return NULL;
	  }
	 
      break;
      
      case SDLK_l:
        if (event->key.keysym.mod & KMOD_CTRL)
        {
	    if (LoadSave_Console == NULL)
	    {
	      SDL_Rect rect;
	      SDL_Surface *Screen = SDL_GetVideoSurface();
	      rect.x = 400;
	      rect.y = 150;
	      rect.w = 200;
	      rect.h = 150;
              LoadSave_Console = CON_Init("ConsoleFont.gif", Screen, 100, rect);
	    }
	    CON_SetExecuteFunction(LoadSave_Console, Load_Handler);
	    CON_SetPrompt(LoadSave_Console, "Load Filename: ");
	    Clear_History(LoadSave_Console);
            CON_UpdateConsole(LoadSave_Console);
	    CON_Show(LoadSave_Console);
            CON_Topmost(LoadSave_Console);
	    return NULL;	  
       }
       
      break;
      
      default:
      break;
    
    }
  }
  return event;
}






