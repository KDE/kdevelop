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

class CKDevelop;
class DbgButton;
class DbgController;
class DbgToolbar;
class KWinModule;
class KDockWindow;

#include <ksystemtray.h>
#include <kwin.h>         // needed for WId :(

#include <qframe.h>


/**
  *@author John Birch
  */

// **************************************************************************
// **************************************************************************
// **************************************************************************

class DbgDocker : public KSystemTray
{
  Q_OBJECT

public:
  DbgDocker(QWidget* parent, DbgToolbar* toolBar, const QPixmap& pixmap);
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

  private:
    void setAppIndicator(bool appIndicator);

  public slots:
    void slotDbgStatus(const QString&,int);
    void slotDock();
    void slotUndock();
    void slotIconifyAndDock();
    void slotActivateAndUndock();

  private slots:
    void slotDbgKdevFocus();
    void slotDbgPrevFocus();

  private:
    CKDevelop*      ckDevelop_;
    DbgController*  dbgController_;
    WId             activeWindow_;
    KWinModule*     winModule_;
    DbgButton*      bKDevFocus_;
    DbgButton*      bPrevFocus_;
    bool            appIsActive_;
    bool            docked_;
    DbgDocker*      docker_;
    KDockWindow*    dockWindow_;
};

#endif
