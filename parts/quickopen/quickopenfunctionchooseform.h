/*
 *  Copyright (C) 2004 Ahn, Duk J.(adjj22@kornet.net) (adjj1@hanmail.net)
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 *
 */

#ifndef QUICKOPENFUNCTIONCHOOSEFORM_H
#define QUICKOPENFUNCTIONCHOOSEFORM_H

#include "quickopenfunctionchooseformbase.h"

class QuickOpenFunctionChooseForm : public QuickOpenFunctionChooseFormBase
{
  Q_OBJECT

public:
  QuickOpenFunctionChooseForm(QWidget* parent = 0, const char* name = 0, bool modal = FALSE, Qt::WFlags fl = 0 );
  ~QuickOpenFunctionChooseForm();

public slots:
  virtual void slotArgsChange( int id );
  virtual void slotFileChange( int id );

};

#endif

