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

#ifndef DOCK_MOVE_MANAGER
#define DOCK_MOVE_MANAGER

#include <qobject.h>
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

class QTimer;
class QWidget;

class DockMoveManager: public QObject
{Q_OBJECT
friend class DockManager;

private:
  DockMoveManager(QWidget *widget);
  ~DockMoveManager();

  void doMove(bool in_hotspot_static=false, bool dynamic = false, bool dontmove=false);
  void movePause(){ pauseMove = true; }
  void moveContinue(){ pauseMove = false; }
  void setGeometry(int x, int y, int w, int h);

  /**
   * Resizes the resizer. rw is width and rh is height
   */
  void resize(int rw, int rh) {setGeometry(xp, yp, rw, rh);};

  /**
   * Returns global x coordinate of a mouse.
   */
   int mouseX() {return rx;};

  /**
   * Returns global y coordinate of a mouse.
   */
  int mouseY() {return ry;};
  
  /**
   * Returns global x coordinate.
   */
  int x() {return xp;};

  /**
   * Returns global y coordinate.
   */
  int y() {return yp;};

  /**
   * Returns width.
   */
  int width() {return w;};

  /**
   * Returns height.
   */
  int height() {return h;};
  /**
   * Calling this slot will stop the process of moving. It is
   * equal as if user releases the mouse button.
   */
  void stop ();
  /**
   * Internal - draws rectangle on the screen
   */
  void drawRectangle (int x, int y, int w, int h);

  /**
   * Internal - deletes last rectangle, if there is one.
   */
  void deleteLastRectangle ();


private slots:
  void doMoveInternal();

private:
  bool pauseMove;
  int xp, yp, w, h;
  int ox, oy, ow, oh;
  int orig_x, orig_y, orig_w, orig_h;
  bool noLast;
  bool working;

  QWidget *widget;
  QTimer *timer;

  int rx, ry, sx, sy;
  int offX, offY;

  /* X-stuff */
  Window root;
  GC rootgc;
  int scr;
  XEvent ev;
  unsigned int active_button;
};

#endif



