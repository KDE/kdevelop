/***************************************************************************
 *   Copyright (C) 2001-2003                                               *
 *   The KDevelop Team                                                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef __K_TABWIDGET_H__
#define __K_TABWIDGET_H__

#include <qtabwidget.h>
#include <qtabbar.h>
#include <qevent.h>

class QPopupMenu;

class KTabBar : public QTabBar
{
  Q_OBJECT

public:
  KTabBar(QWidget *parent=0, const char *name=0);
signals:
  void closeWindow(const QWidget *);
  void closeOthers(QWidget *);
protected slots:
  virtual void mousePressEvent(QMouseEvent *e);  
  void closeWindowSlot();
  void closeOthersSlot();
private:
  QPopupMenu *m_pPopupMenu;
  QWidget *m_pPage;
};

class KDevTabWidget : public QTabWidget
{
  Q_OBJECT
  
public:
  KDevTabWidget(QWidget *parent=0, const char *name=0);
signals:
  void closeWindow(const QWidget *);
  void closeOthers(QWidget *);
private:
  KTabBar *m_pTabBar;
};

#endif
