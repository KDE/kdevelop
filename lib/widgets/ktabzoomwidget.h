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

#ifndef __K_TABZOOM_WIDGET_H__
#define __K_TABZOOM_WIDGET_H__

#include <qwidget.h>

#include <kmultitabbar.h>

class KTabZoomWidgetPrivate;
class KConfig;

class KTabZoomWidget : public QWidget
{
  Q_OBJECT

public:

  KTabZoomWidget(QWidget *parent=0, KMultiTabBar::KMultiTabBarPosition pos=KMultiTabBar::Left, const char *name=0);
  ~KTabZoomWidget();

  void setStyle( KMultiTabBar::KMultiTabBarStyle style );

  void addTab( const QPixmap& pixmap, QWidget *widget, const QString &tab, const QString &toolTip = QString::null);
  void removeTab(QWidget *widget);

  void addContent(QWidget *widget);

  void raiseWidget(QWidget *widget);
  void lowerWidget(QWidget *widget);

  void lowerAllWidgets();

  void saveSettings(KConfig *config);
  void loadSettings(KConfig *config);

  bool isRaised() const;
  bool isDocked() const;
  bool hasFocus() const;
  bool isEmpty() const; //indicates whether widget has tabs

  uint count() const;
  int indexOf(QWidget *widget) const;

  QWidget *at(int i) const;
  QWidget *current() const;

signals:
  // emitted when a tab gets added / deleted
  void tabsChanged();

public slots:

  void setDockMode(bool docked);
  void setFocus();

private slots:

  void selected(int index);
  void unselected();

  void widgetDeleted();

  void adjustStrut();
  void unsetButtons();


protected:

  virtual void resizeEvent(QResizeEvent *ev);


private:

  void calculateGeometry();

  KTabZoomWidgetPrivate *d;

};


#endif
