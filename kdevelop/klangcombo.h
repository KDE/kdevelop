/*
 * klangcombo.h - A combobox to select a language
 *
 * Copyright (c) 1998 Matthias Hoelzer (hoelzer@physik.uni-wuerzburg.de)
 *
 * Requires the Qt widget libraries, available at no cost at
 * http://www.troll.no/
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */


#ifndef __KLANGCOMBO_H__
#define __KLANGCOMBO_H__


#include <qcombo.h>


class KLanguageCombo : public QComboBox
{
  Q_OBJECT

public:

  KLanguageCombo(QWidget *parent=0, const char *name=0);
  ~KLanguageCombo();

  void insertLanguage(const char *lang);  

  QString tag(const char *lang);   
  QString language(const char *lang);
  
};


#endif

