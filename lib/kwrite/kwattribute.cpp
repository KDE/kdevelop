/*
   Copyright (C) 1998, 1999 Jochen Wilhelmy
                            digisnap@cs.tu-berlin.de

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#include "kwattribute.h"

Attribute::Attribute() : font(), fm(font) {
}

void Attribute::setFont(const QFont &f) {
  font = f;
  fm = FontMetrics(f);
//workaround for slow QFontMetrics::width(), QFont::fixedPitch() doesn't seem to work
  if ((fontWidth = fm.width('W')) != fm.width('i')) fontWidth = -1;
}

