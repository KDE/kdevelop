/***************************************************************************
                          cfinddoctextdlg.h  -  description                              
                             -------------------                                         
    begin                : Thu Feb 25 1999                                           
    copyright            : (C) 1999 by Ralf Nolden
    email                : Ralf.Nolden@post.rwth-aachen.de

    Adapted from finddlg.h  and finddlg.cpp of khelp
    Copyright (C) 1998 Martin Jones (mjones@kde.org)
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   * 
 *                                                                         *
 ***************************************************************************/


#ifndef CFINDDOCTEXTDLG_H
#define CFINDDOCTEXTDLG_H

#include <kiconloader.h>
#include <qdialog.h>
#include <klocale.h>

/**
  *@author 
  */
class CFindDocTextDlg : public QDialog  {
   Q_OBJECT
public: 
	CFindDocTextDlg(QWidget *parent=0, const char *name=0);
	~CFindDocTextDlg();

public slots:
  void slotTextChanged( const char *t );
  void slotClose();
  void slotFind();

signals:
  void signalFind(QString);

private:
  QString text;
  KIconLoader* icon_loader;
};

#endif





