/***************************************************************************
                          memview.h  -  description                              
                             -------------------                                         
    begin                : Tue Oct 5 1999                                           
    copyright            : (C) 1999 by John Birch                         
    email                : jb.nz@writeme.com                                     
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   * 
 *                                                                         *
 ***************************************************************************/


/**
  *@author John Birch
  */

#ifndef MEMVIEW_H
#define MEMVIEW_H

#include <kwizard.h>

class KLined;
class QMultiLineEdit;

class MemoryView : public KDialog
{
  Q_OBJECT

public:
  MemoryView(QWidget *parent, const char *name);
  ~MemoryView();

signals:
  void disassemble(const QString& start, const QString& end);
  void memoryDump(const QString& start, const QString& end);
  void registers();
  void libraries();

public slots:
  void slotRawGDBMemoryView(char* buf);

private slots:
  void slotDisassemble();
  void slotMemoryDump();

private:
  KLined*         start_;
  KLined*         end_;
  QMultiLineEdit* output_;
};

#endif
