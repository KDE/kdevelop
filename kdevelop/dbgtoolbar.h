/***************************************************************************
                          debugtoolbar.h  -  description
                             -------------------
    begin                : Thu Dec 23 1999
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
#ifndef DBGTOOLBAR_H
#define DBGTOOLBAR_H

class DbgController;
class CKDevelop;
class DbgButton;

#include <kapp.h>       // Needed for "Window" (Xlib/qt inclusion order is tricky)
#include <qframe.h>

/**
  *@author John Birch
  */

class DbgToolbar : public QFrame
{
  Q_OBJECT

  public:
    DbgToolbar(DbgController* dbgController, CKDevelop* ckDevelop);
    virtual ~DbgToolbar();

  public slots:
    void slotDbgStatus(const QString&,int);

  private slots:
    void slotDbgStop();
    void slotDbgKdevFocus();
    void slotDbgPrevFocus();

  private:
    CKDevelop*  ckDevelop_;
    Window      activeWindow_;
    DbgButton*  bKDevFocus_;
    DbgButton*  bPrevFocus_;
};

#endif
