/***************************************************************************
                          phperrorview.h  -  description
                             -------------------
    begin                : Sat May 26 2001
    copyright            : (C) 2001 by 
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

#ifndef PHPERRORVIEW_H
#define PHPERRORVIEW_H

#include <qwidget.h>
#include <qlistbox.h>
#include <qintdict.h>
#include <qstring.h>

class PHPSupportPart;

class ErrorItem {
 public:
  QString filename;
  int line;
};

/**
  *@author 
  */

class PHPErrorView : public QListBox  {
   Q_OBJECT
public: 
	PHPErrorView(PHPSupportPart *part);
	~PHPErrorView();
	void parse(QString& phpOutput);
	
 signals:
	  void fileSelected(const QString& fileName,int lineNumber);
	  
	  private slots:
	    void slotItemSelected(int);
	  
	  
 private:
	  QIntDict<ErrorItem> errorDict;
};

#endif
