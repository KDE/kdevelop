/***************************************************************************
 *   Copyright (C) 2002 by Jakob Simon-Gaarde                              *
 *   jsgaarde@tdcspace.dk                                                  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _CARET_H_
#define _CARET_H_

#include <stdio.h>
class Caret
{
  public:
  int      m_row;
  int      m_idx;
            Caret();
            Caret(const int row, const int idx);
            Caret(const Caret& value);
  bool      operator<(const Caret& compare);
  bool      operator>(const Caret& compare);
  bool      operator<=(const Caret& compare);
  bool      operator>=(const Caret& compare);
  bool      operator==(const Caret& compare);
  bool      operator!=(const Caret& compare);
  Caret     operator=(const Caret& value);
  Caret     operator+(const Caret& value);
  Caret     operator-(const Caret& value);

};

#endif

