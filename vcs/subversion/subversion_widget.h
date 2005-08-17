/* Copyright (C) 2003
	 Mickael Marchand <marchand@kde.org>

	 This program is free software; you can redistribute it and/or
	 modify it under the terms of the GNU General Public
	 License as published by the Free Software Foundation; either
	 version 2 of the License, or (at your option) any later version.

	 This program is distributed in the hope that it will be useful,
	 but WITHOUT ANY WARRANTY; without even the implied warranty of
	 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
	 General Public License for more details.

	 You should have received a copy of the GNU General Public License
	 along with this program; see the file COPYING.  If not, write to
	 the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
	 Boston, MA 02111-1307, USA.
	 */

#ifndef __SUBVERSION_WIDGET_H__
#define __SUBVERSION_WIDGET_H__

#include <q3textedit.h>

class subversionPart;

class subversionWidget : public Q3TextEdit
{
  Q_OBJECT
    
public:
		  
  subversionWidget(subversionPart *part, QWidget *parent, const char* name);
  ~subversionWidget();

private:
	subversionPart *m_part;

};

#endif
