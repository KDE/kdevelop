/***************************************************************************
                      coutputwidget.cpp - the output window in KDevelop
                             -------------------                                         

    begin                : 5 Aug 1998                                        
    copyright            : (C) 1998 by Sandy Meier                         
    email                : smeier@rz.uni-potsdam.de                                     
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   * 
 *                                                                         *
 ***************************************************************************/
#include "coutputwidget.h"

COutputWidget::COutputWidget(QWidget* parent, const char* name) :
  KEdit(parent,name)
{
}

void COutputWidget::insertAtEnd(QString s)
{
  int row = (numLines() == 0)? 0 : numLines()-1;
  // correct workaround for QMultilineEdit
  //  the string inside could be NULL, and so QMultilineEdit fails
  int col = qstrlen(textLine(row));
  if (s.left(1) == "\n" && row == 0 && col == 0)
     s = QString(" ")+s;
  insertAt(s, row, col);
}
void COutputWidget::mouseReleaseEvent(QMouseEvent*){
  emit clicked();
}
void COutputWidget::keyPressEvent ( QKeyEvent* event){
  KEdit::keyPressEvent(event);
  emit keyPressed(event->ascii());
}
