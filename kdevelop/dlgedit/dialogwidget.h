/***************************************************************************
                          dialogwidget.h  -  description
                             -------------------
    begin                : Thu Jan 20 2000
    copyright            : (C) 2000 by 
    email                : 
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef DIALOGWIDGET_H
#define DIALOGWIDGET_H

#include <qwidget.h>

/**
  *@author 
  */

class DialogWidget : public QWidget  {
   Q_OBJECT
public: 
	DialogWidget(QWidget *parent=0, const char *name=0);
	~DialogWidget();
	QString fileName() { return filename;}
	void openDialog(QString filename);

 private:
	QString filename;
};

#endif
