/*
    Modify from KTolBoxManager
    novaprint@mtu-net.ru
    Judin Max
**********************************/

/*
    This file is part of the KDE libraries
    Copyright (C) 1998 Sven Radej (radej@kde.org)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#include <qcursor.h>
#include <qapplication.h>
#include <qwindowdefs.h>
#include <qwidget.h>
#include <qtimer.h>
#include <qrect.h>
#include <kwm.h>

#include "dockmovemanager.h"

DockMoveManager::DockMoveManager(QWidget *_widget) : QObject ()
{
  XGCValues gv;

  working=false;
  noLast=true;
  widget = _widget;

  scr = qt_xscreen();
  root = qt_xrootwin();

  gv.function = GXxor;
  gv.line_width = 0;
  gv.foreground = WhitePixel(qt_xdisplay(), scr)^BlackPixel(qt_xdisplay(), scr);
  gv.subwindow_mode = IncludeInferiors;
  long mask = GCForeground | GCFunction | GCLineWidth | GCSubwindowMode;
  rootgc = XCreateGC(qt_xdisplay(), qt_xrootwin(), mask, &gv);

  timer = new QTimer(this);
}

DockMoveManager::~DockMoveManager()
{
  stop();
}

void DockMoveManager::doMove (bool hot_static, bool _dynamic, bool dontmove)
{
  if (working) return;

  Window wroot, wchild;
  int trash;
  
  working=true;

  QRect rr = KWM::geometry(widget->winId(), true);
  QPoint p(rr.topLeft());

  offX = QCursor::pos().x() - p.x();
  offY = QCursor::pos().y() - p.y();
  
  xp = p.x();
  yp = p.y();
  w = rr.width();
  h = rr.height();

  orig_x = p.x();
  orig_y = p.y();
  orig_w = w;
  orig_h = h;

  XQueryPointer( qt_xdisplay(), qt_xrootwin(), &wroot, &wchild,
                 &sx, &sy, &trash, &trash, &active_button);
  
  rx = sx;
  ry = sy;
  
  xp=sx-offX;
  yp=sy-offY;

  QApplication::setOverrideCursor(QCursor(sizeAllCursor));
  connect (timer, SIGNAL(timeout()), this, SLOT (doMoveInternal()));

  drawRectangle(xp, yp, w, h);

  pauseMove = false;
  timer->start(0);
}

void DockMoveManager::doMoveInternal()
{
  if ( pauseMove ) return;

  Window wroot, wchild;
  int trash;
  unsigned int buttons;

  XQueryPointer( qt_xdisplay(), qt_xrootwin(), &wroot, &wchild,
                 &rx, &ry, &trash, &trash, &buttons );
  
  if (buttons != active_button)
  {
    stop();
    return;
  }
  
  if (rx == sx && ry == sy) return;

  sx=rx;
  sy=ry;

  xp=rx-offX;
  yp=ry-offY;

  deleteLastRectangle();
  drawRectangle(xp, yp, w, h);

  XFlush(qt_xdisplay());
  XSync(qt_xdisplay(), False);
}

void DockMoveManager::stop ()
{
  if (!working) return;
  
  timer->stop();
  disconnect (timer, SIGNAL(timeout()));
  QApplication::restoreOverrideCursor();

  deleteLastRectangle();
  XFlush(qt_xdisplay());
  QApplication::restoreOverrideCursor();

  QPoint p(xp, yp);
  if (widget->parent() != 0) p=widget->parentWidget()->mapFromGlobal(p);

  widget->move(p);

  working = false;
}

void DockMoveManager::setGeometry (int _x, int _y, int _w, int _h)
{
  if (!working) return;
  xp=_x;
  yp=_y;
  w=_w;
  h=_h;
  deleteLastRectangle();
  drawRectangle( _x, _y, _w, _h);
}

void DockMoveManager::drawRectangle(int _x, int _y, int _w, int _h)
{
  ox = _x;
  oy = _y;
  ow = _w;
  oh = _h;

  XDrawRectangle(qt_xdisplay(), root, rootgc, _x, _y, _w, _h);
  if (_w > 2)
    _w -= 2;
  if (_h > 2)
    _h -= 2;
  XDrawRectangle(qt_xdisplay(), root, rootgc, _x+1, _y+1, _w, _h);
  if (_w > 2)
    _w -= 2;
  if (_h > 2)
    _h -= 2;
  XDrawRectangle(qt_xdisplay(), root, rootgc, _x+2, _y+2, _w, _h);
  
  noLast = false;
}

void DockMoveManager::deleteLastRectangle()
{
  if (noLast) return;
  
  XDrawRectangle(qt_xdisplay(), root, rootgc, ox, oy, ow, oh);
  if (ow > 2)
    ow -= 2;
  if (oh > 2)
    oh -= 2;
  XDrawRectangle(qt_xdisplay(), root, rootgc, ox+1, oy+1, ow, oh);
  if (ow > 2)
    ow -= 2;
  if (oh > 2)
    oh -= 2;
  XDrawRectangle(qt_xdisplay(), root, rootgc, ox+2, oy+2, ow, oh);

  noLast = true;
}

