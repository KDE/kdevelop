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

#ifndef __K_TABZOOMFRAME_H__
#define __K_TABZOOMFRAME_H__

#include <qframe.h>

#include "ktabzoomposition.h"


class KTabZoomFramePrivate;

class KTabZoomFrame : public QWidget
{
  Q_OBJECT

public:

  KTabZoomFrame(QWidget *parent=0, KTabZoomPosition::Position pos=KTabZoomPosition::Left, const char *name=0);
  ~KTabZoomFrame();

  int addTab(QWidget *widget, const QString &title);
  void removeTab(int index);

  void setDockMode(bool docked);


public slots:

  void selected(int index);


signals:

  void closeClicked();
  void dockToggled(bool docked);
  void sizeChanged();

  
protected:

  void mousePressEvent(QMouseEvent *ev);
  void mouseReleaseEvent(QMouseEvent *ev);
  void mouseMoveEvent(QMouseEvent *ev);


private slots:

  void slotDockButtonToggled(bool toggle);


private:

  KTabZoomFramePrivate *d;

};


#endif
