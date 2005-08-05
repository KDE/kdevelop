/* This file is part of the KDE libraries
   Copyright (C) 2000 Max Judin <novaprint@mtu-net.ru>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

/*
   IMPORTANT Note: This file compiles also in Qt-only mode by using the NO_KDE2 precompiler definition
*/

#ifndef KDOCKWIDGET_PRIVATE_H
#define KDOCKWIDGET_PRIVATE_H

#include <qwidget.h>
#include <qpushbutton.h>

#ifndef NO_KDE2
#include <netwm_def.h>
#endif

class QFrame;
class KDockContainer;


/**
 * Like QSplitter but specially designed for dockwidgets stuff.
 * @internal
 *
 * @author Max Judin.
*/
class KDEUI_EXPORT KDockSplitter : public QWidget
{
  Q_OBJECT
public:
  KDockSplitter(QWidget *parent= 0, const char *name= 0, Orientation orient= Vertical, int pos= 50, bool highResolution=false);
  virtual ~KDockSplitter(){};

  void activate(QWidget *c0, QWidget *c1 = 0L);
  void deactivate();

  int separatorPos() const;
  void setSeparatorPos(int pos, bool do_resize = true);
  /**
   * For usage from outside.
   * If the splitter is in fixed position when called,
   * the value of @p pos will be saved and used when the splitter
   * is restored.
   * If @p do_resize is true, the size will be changed unless the splitter
   * is in fixed mode.
   */
  // ### please come up with a nicer name
  void setSeparatorPosX(int pos, bool do_resize=false);

  virtual bool eventFilter(QObject *, QEvent *);
  virtual bool event( QEvent * );

  QWidget* getFirst() const { return child0; }
  QWidget* getLast() const { return child1; }
  QWidget* getAnother( QWidget* ) const;
  void updateName();

  void setOpaqueResize(bool b=true);
  bool opaqueResize() const;

  void setKeepSize(bool b=true);
  bool keepSize() const;

  void setHighResolution(bool b=true);
  bool highResolution() const;

  void setForcedFixedWidth(KDockWidget *dw,int w);
  void setForcedFixedHeight(KDockWidget *dw,int h);
  void restoreFromForcedFixedSize(KDockWidget *dw);

  Orientation orientation(){return m_orientation;}

protected:
  friend class  KDockContainer;
  int checkValue( int ) const;
  int checkValueOverlapped( int ,QWidget*) const;
  virtual void resizeEvent(QResizeEvent *);
/*
protected slots:
  void delayedResize();*/

private:
  void setupMinMaxSize();
  QWidget *child0, *child1;
  Orientation m_orientation;
  bool initialised;
  QFrame* divider;
  int xpos, savedXPos;
  bool mOpaqueResize, mKeepSize, mHighResolution;
  int fixedWidth0,fixedWidth1;
  int fixedHeight0,fixedHeight1;
  bool m_dontRecalc;
};

/**
 * A mini-button usually placed in the dockpanel.
 * @internal
 *
 * @author Max Judin.
*/
class KDEUI_EXPORT KDockButton_Private : public QPushButton
{
  Q_OBJECT
public:
  KDockButton_Private( QWidget *parent=0, const char *name=0 );
  ~KDockButton_Private();

protected:
  virtual void drawButton( QPainter * );
  virtual void enterEvent( QEvent * );
  virtual void leaveEvent( QEvent * );

private:
  bool moveMouse;
};

/**
 * resizing enum
 **/



/**
 * additional KDockWidget stuff (private)
*/
class KDEUI_EXPORT KDockWidgetPrivate : public QObject
{
  Q_OBJECT
public:
  KDockWidgetPrivate();
  ~KDockWidgetPrivate();

public slots:
  /**
   * Especially used for Tab page docking. Switching the pages requires additional setFocus() for the embedded widget.
   */
  void slotFocusEmbeddedWidget(QWidget* w = 0L);

public:
 enum KDockWidgetResize
{ResizeLeft,ResizeTop,ResizeRight,ResizeBottom,ResizeBottomLeft,ResizeTopLeft,ResizeBottomRight,ResizeTopRight};

  int index;
  int splitPosInPercent;
  bool pendingFocusInEvent;
  bool blockHasUndockedSignal;
  bool pendingDtor;
  int forcedWidth;
  int forcedHeight;
  bool isContainer;

#ifndef NO_KDE2
  NET::WindowType windowType;
#endif

  QWidget *_parent;
  bool transient;

  QGuardedPtr<QWidget> container;

  QPoint resizePos;
  bool resizing;
  KDockWidgetResize resizeMode;
};

class KDEUI_EXPORT KDockWidgetHeaderPrivate
   : public QObject
{
public:
  KDockWidgetHeaderPrivate( QObject* parent )
        : QObject( parent )
  {
    forceCloseButtonHidden=false;
    toDesktopButton = 0;
    showToDesktopButton = true;
    topLevel = false;
    dummy=0;
  }
  KDockButton_Private* toDesktopButton;

  bool showToDesktopButton;
  bool topLevel;
  QPtrList<KDockButton_Private> btns;
  bool forceCloseButtonHidden;
  QWidget *dummy;
};

#endif
