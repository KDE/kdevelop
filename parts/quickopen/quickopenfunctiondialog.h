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
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 *
 */

#ifndef QUICKOPENFUNCTIONDLG_H
#define QUICKOPENFUNCTIONDLG_H

#include <codemodel.h>

#include "quickopendialog.h"
#include "quickopen_part.h"

class KCompletion;
class QuickOpenPart;

class QuickOpenFunctionDialog : public QuickOpenDialog
{
  Q_OBJECT

public:
  QuickOpenFunctionDialog( QuickOpenPart* part, QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
  ~QuickOpenFunctionDialog();
  
  void gotoFile( QString name );
  
  FunctionList* matchingFuncList(){ return funcList; };
  QuickOpenPart* part(){ return m_part; };
  
public slots:
  virtual void slotExecuted(QListBoxItem*);//itemList executed, returnPressed
  virtual void executed(QListBoxItem*);	//
  virtual void slotReturnPressed();			//buttonOk clicked, nameEdit returnPressed
  virtual void slotTextChanged(const QString & text);

protected:
    void fillFunctions();
    int spaces;
    FunctionList *m_functionDefList;
    QStringList *m_functionStrList;
    FunctionList *funcList;
	
};

#endif

