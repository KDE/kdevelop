/***************************************************************************
                          framestack.h  -  description                              
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

#ifndef FRAMESTACK_H
#define FRAMESTACK_H

#include <qlistbox.h>
#include <qstrlist.h>

/**
  *@author John Birch
  */

class FrameStack : public QListBox
{
  Q_OBJECT

public:

  FrameStack( QWidget * parent=0, const char * name=0, WFlags f=0 );
	virtual ~FrameStack();
  void parseGDBBacktraceList(char* str);
  QString getFrameParams(int frame);
  QString getFrameName(int frame);
	
public slots:
  void slotHighlighted(int index);
  void slotSelectFrame(int frame);

signals:
  void selectFrame(int selectFrame);

private:
  int currentFrame_;
  QStrList* currentList_;
};

#endif
