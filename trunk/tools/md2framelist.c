/* Simple md2 frame lister. 
   To compile: gcc -o md2framelist md2framelist.c*/
/*
Contains code from Yeti3D.

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

#include <stdlib.h>
#include <stdio.h>

typedef struct 
{ 
   int magic; 
   int version; 
   int skinWidth; 
   int skinHeight; 
   int frameSize; 
   int numSkins; 
   int numVertices; 
   int numTexCoords; 
   int numTriangles; 
   int numGlCommands; 
   int numFrames; 
   int offsetSkins; 
   int offsetTexCoords; 
   int offsetTriangles; 
   int offsetFrames; 
   int offsetGlCommands;
   int offsetEnd; 
} model_t;

typedef struct
{
   char vertex[3];
   char lightNormalIndex;
} triangleVertex_t;

typedef struct
{
   float scale[3];
   float translate[3];
   char name[16];
   triangleVertex_t vertices[1];
} frame_t;

void* yeti_load_file(char* filename)
{
  void* r = 0;
  FILE* fp = fopen(filename, "rb");

  if (fp)
  {
    if (!fseek(fp, 0, SEEK_END))
    {
      int n = ftell(fp);
      if ((r = malloc(n)) != 0)
      {
        fseek(fp, 0, SEEK_SET);
        fread(r, 1, n, fp);
      }
    }
    fclose(fp);
  }
  return r;
}



int main(int argc, char *argv[])
{
  model_t *model;
  frame_t* f;
  int i;
  
  if (argc < 2)
  {
    printf("usage: md2framelist [name_of_file].md2\n");
    printf("  To direct to a file, use: md2framelist [name_of_file].md2 >>"
    "[name_of_file.txt\n");
    return -1; 
  }
  model = (model_t *)yeti_load_file(argv[1]);
  
  for (i=0; i<model->numFrames; i++)
  {
    f = (frame_t*)((int)model + model->offsetFrames + model->frameSize * i);
    printf("Frame no.: %d Frame name: %s \n", i, f->name);
  }
  
  return 0;
}
