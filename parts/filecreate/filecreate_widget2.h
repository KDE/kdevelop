/***************************************************************************
 *   Copyright (C) 2003 by Julian Rockey                                   *
 *   linux@jrockey.com                                                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#ifndef __FILECREATE_WIDGET2_H__
#define __FILECREATE_WIDGET2_H__


#include <qtable.h>
#include <qstring.h>
#include <qmap.h>

//#include <klistview.h>

#include "filecreate_typechooser.h"

class KIconLoader;
class KDevProject;
class FileCreatePart;

class FileCreateFileType;

class FileCreateWidget2 : public QTable, public FileCreateTypeChooser
{
  Q_OBJECT
    
public:
		  
  FileCreateWidget2(FileCreatePart *part);
  virtual ~FileCreateWidget2();

  virtual void refresh();

private:
  virtual void setRow(int row, FileCreateFileType * filetype);
  virtual void empty();
  virtual void setDefaultColumnWidths();

  KIconLoader * m_iconLoader;
  QMap<int,FileCreateFileType*> typeForRow;
  FileCreateFileType * m_selected;

private slots:
  virtual void slotCellSelected(int row, int col);
  virtual void slotDoSelection();
  
};


#endif
