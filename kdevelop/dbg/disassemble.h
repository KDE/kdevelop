/***************************************************************************
                         disassemble.h  -  description                              
                             -------------------                                         
    begin                : Sun Aug 8 1999                                           
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

#ifndef __disassemble_h__
#define __disassemble_h__

#include <keditcl.h>

class Breakpoint;
class QMouseEvent;

/***************************************************************************/
/***************************************************************************/
/***************************************************************************/

class Disassemble : public KEdit
{
  Q_OBJECT

public:
  Disassemble(KApplication* a, QWidget* parent, char* name=0);
  virtual ~Disassemble();

private:
  bool displayCurrent();

public slots:
  void slotDisassemble(char* buf);
  void slotActivate(bool activate);
  void slotShowStepInSource(const QString& filename, int lineno, const QString& address);
  void slotBPState(const Breakpoint* BP);

signals:
  void disassemble(const QString& start, const QString& end);

private:
  bool    active_;
  long    lowerBound_;
  long    upperBound_;
  QString currentAddress_;
};
/***************************************************************************/
/***************************************************************************/
/***************************************************************************/

#endif
