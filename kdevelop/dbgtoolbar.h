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
class DbgToolbar;

#include <kapp.h>       // Needed for "Window" (Xlib/qt inclusion order is tricky)
#include <qframe.h>
#include <qlabel.h>

/**
  *@author John Birch
  */

// **************************************************************************
// **************************************************************************
// **************************************************************************

class DbgDocker : public QLabel
{
  Q_OBJECT

public:
  DbgDocker(DbgToolbar* toolBar, const QPixmap& pixmap);
  virtual ~DbgDocker()  {};
  virtual void mousePressEvent(QMouseEvent *e);

signals:
  void clicked();

private:
  DbgToolbar* toolBar_;
};

// **************************************************************************
// **************************************************************************
// **************************************************************************

class DbgToolbar : public QFrame
{
  Q_OBJECT

  public:
    DbgToolbar(DbgController* dbgController, CKDevelop* ckDevelop);
    virtual ~DbgToolbar();
//    virtual void mousePressEvent(QMouseEvent* e);

  private:
    void setAppIndicator(bool appIndicator);

  public slots:
    void slotDbgStatus(const QString&,int);
    void slotDock();
    void slotUndock();
    void slotIconifyAndDock();
    void slotActivateAndUndock();

  private slots:
    void slotDbgStop();
    void slotDbgKdevFocus();
    void slotDbgPrevFocus();

  private:
    CKDevelop*      ckDevelop_;
    DbgController*  dbgController_;
    Window          activeWindow_;
    DbgButton*      bKDevFocus_;
    DbgButton*      bPrevFocus_;
    bool            appIsActive_;
    bool            docked_;
    DbgDocker*      docker_;
};

#endif
