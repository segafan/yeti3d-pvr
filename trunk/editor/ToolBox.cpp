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


#pragma hdrstop

#include "ToolBox.h"

//---------------------------------------------------------------------------

#pragma package(smart_init)

TRect RectNormalize(TRect* ARect)
{
  return Rect(
    Min((int)ARect->Left, ARect->Right),
    Min((int)ARect->Top , ARect->Bottom),
    Max((int)ARect->Left, ARect->Right),
    Max((int)ARect->Top , ARect->Bottom)
    );
}

TRect RectInflate(TRect* ARect, int X, int Y)
{
  return Rect(
    ARect->Left - X,
    ARect->Top - Y,
    ARect->Right + X,
    ARect->Bottom + Y
    );
}
