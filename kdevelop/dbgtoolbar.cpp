/***************************************************************************
                          dbgtoolbar.cpp  -  description
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

#include "dbgtoolbar.h"
#include "./dbg/dbgcontroller.h"

#include "ckdevelop.h"

#include <kdockwindow.h>
#include <klocale.h>
#include <kpopupmenu.h>
#include <kstddirs.h>
#include <kwin.h>
#include <kwinmodule.h>

#include <qapp.h>
#include <qframe.h>
#include <qlayout.h>
#include <qpainter.h>
#include <qpushbutton.h>
#include <qtooltip.h>
#include <qcursor.h>

// **************************************************************************
// **************************************************************************
// **************************************************************************

// Implements a floating toolbar for the debugger.

// Unfortunately, I couldn't get the KToolBar to work nicely when it
// was floating, so I was forced to write these classes. I'm not sure whether
// I didn't try hard enough or ... and I've forgotten what the problems were
// now.

// The problem with using this is that it will not dock as a normal toolbar.
// I'm not convince that this is a real problem though.

// So, if you can get it to work as a KToolBar, and it works well when the
// app is running, then all these classes can be removed.

// This code is very specific to the internal debugger in kdevelop.

// **************************************************************************
// **************************************************************************
// **************************************************************************

// This just allows the user to click on the toolbar and drag it somewhere else.
// I would have preferred to use normal decoration on the toolbar and removed
// the iconify, close, etc buttons from the window title but again I kept running
// into problems. Instead, I used no decoration and this class. Also this looks
// similar to the KToolBar floating style.
class DbgMoveHandle : public QFrame
{
public:
  DbgMoveHandle(DbgToolbar* parent=0, const char * name=0, WFlags f=0);
  virtual ~DbgMoveHandle();

  virtual void mousePressEvent(QMouseEvent *e);
  virtual void mouseReleaseEvent( QMouseEvent *e);
  virtual void mouseMoveEvent(QMouseEvent *e);

private:
  DbgToolbar* toolBar_;
  QPoint      offset_;
  bool        moving_;
};

// **************************************************************************

DbgMoveHandle::DbgMoveHandle(DbgToolbar* toolBar,
                              const char * name, WFlags f) :
  QFrame(toolBar, name, f),
  toolBar_(toolBar),
  offset_(QPoint(0,0)),
  moving_(false)
{
  setFrameStyle( QFrame::Panel|QFrame::Raised);
  setFixedHeight( 12 );
}

// **************************************************************************

DbgMoveHandle::~DbgMoveHandle()
{
}

// **************************************************************************

void DbgMoveHandle::mousePressEvent(QMouseEvent *e)
{
  QFrame::mousePressEvent(e);
  if (moving_)
    return;

  if (e->button() == RightButton)
  {
    KPopupMenu* menu = new KPopupMenu( this );
    menu->setTitle("Debug Toolbar");
    menu->insertItem(i18n("Dock to Panel"),                     toolBar_, SLOT(slotDock()));
    menu->insertItem(i18n("Dock to Panel and Iconify KDevelop"),toolBar_, SLOT(slotIconifyAndDock()));
    menu->popup(e->globalPos());
    return;
  }

  moving_ = true;
  offset_ = toolBar_->pos() - e->globalPos();
  setFrameStyle( QFrame::Panel|QFrame::Sunken);
  QApplication::setOverrideCursor(QCursor(sizeAllCursor));
  setPalette(QPalette(colorGroup().background()));
  repaint();
}

// **************************************************************************

void DbgMoveHandle::mouseReleaseEvent(QMouseEvent *e)
{
  QFrame::mouseReleaseEvent(e);
  moving_ = false;
  offset_ = QPoint(0,0);
  setFrameStyle( QFrame::Panel|QFrame::Raised);
  QApplication::restoreOverrideCursor();
  setPalette(QPalette(colorGroup().background()));
  repaint();
}

// **************************************************************************

void DbgMoveHandle::mouseMoveEvent(QMouseEvent *e)
{
  QFrame::mouseMoveEvent(e);
  if (!moving_)
    return;

  toolBar_->move(e->globalPos() + offset_);
}

// **************************************************************************
// **************************************************************************
// **************************************************************************

// This class adds text _and_ a pixmap to a button. Why doesn't QPushButton
// support that? It only allowed text _or_ pixmap.
class DbgButton : public QPushButton
{
public:
  DbgButton(const QPixmap& pixmap, const QString& text, DbgToolbar* parent);
  virtual ~DbgButton()  {};
  void drawButtonLabel(QPainter *painter);
  QSize sizeHint() const;

private:
  QPixmap     pixmap_;
};

// **************************************************************************

DbgButton::DbgButton(const QPixmap& pixmap, const QString& text, DbgToolbar* parent) :
  QPushButton(parent),
  pixmap_(pixmap)
{
  setText(text);
}

// **************************************************************************

void DbgButton::drawButtonLabel(QPainter *painter)
{
  // We always have a pixmap (today...)
  // Centre it if there's no text
  int x = ((text() ? height() : width()) - pixmap_.width()) / 2;
  int y = (height() - pixmap_.height()) / 2;
  painter->drawPixmap(x, y, pixmap_);

  if (text())
  {
    painter->setPen(colorGroup().text());
    painter->drawText( height()+2, 0, width()-(height()+2), height(), AlignLeft|AlignVCenter, text());
  }
}

// **************************************************************************

QSize DbgButton::sizeHint() const
{
  QString s(text());
  if (s.isEmpty())
    return pixmap_.size();

#if (QT_VERSION < 300)
  return QPushButton::sizeHint();
#else
  return QSize(QPushButton::sizeHint().width() + pixmap_.size().width(),
                QPushButton::sizeHint().height());
#endif
}

// **************************************************************************
// **************************************************************************
// **************************************************************************

DbgDocker::DbgDocker(QWidget* parent, DbgToolbar* toolBar, const QPixmap& pixmap) :
  KSystemTray(parent, "DbgDocker"),
  toolBar_(toolBar)
{
  setPixmap(pixmap);
  QToolTip::add( this, i18n("KDevelop debugger: Click to execute one line of code (\"step\")") );
}

// **************************************************************************

void DbgDocker::mousePressEvent(QMouseEvent *e)
{
  if (!rect().contains( e->pos()))
	  return;
	
  switch (e->button())
  {
    case LeftButton:
    {
      // Not really a click, but it'll hold for the time being !!!
      emit clicked();
      break;
    }

    case RightButton:
    {
      KPopupMenu* menu = new KPopupMenu( this );
      menu->setTitle("Debug Toolbar");
      menu->insertItem(i18n("Activate"),                        toolBar_, SLOT(slotUndock()));
      menu->insertItem(i18n("Activate (KDevelop gets focus)"),  toolBar_, SLOT(slotActivateAndUndock()));
      menu->popup(e->globalPos());
      break;
    }

    default:
      break;
  }
}

// **************************************************************************
// **************************************************************************
// **************************************************************************

DbgToolbar::DbgToolbar(DbgController* dbgController, CKDevelop* parent) :
  QFrame(0, "DbgToolbar"),
  ckDevelop_(parent),
  dbgController_(dbgController),
  activeWindow_(0),
  winModule_(0),
  bKDevFocus_(0),
  bPrevFocus_(0),
  appIsActive_(false),
  docked_(false),
  docker_(0),
  dockWindow_(new KDockWindow(parent))
{
  setFrameStyle( QFrame::Box | QFrame::Raised );
  setFocusPolicy(NoFocus);
  setLineWidth(1);
  setMidLineWidth(2);

  winModule_  = new KWinModule(this);
  docker_ = new DbgDocker(ckDevelop_, this, BarIcon("dbgnext"));
  connect(docker_, SIGNAL(clicked()),  dbgController_,  SLOT(slotStepOver()));

  // Must have noFocus set so that we can see what window was active.
  // see slotDbgKdevFocus() for more comments
  // I do not want the user to be able to "close" this widget. If we have any
  // decoration then they can and that is bad.
  // This widget is closed when the debugger finishes i.e. they press "Stop"

  // Do we need NoFocus???
  KWin::setState( winId(), NET::StaysOnTop | NET::Modal);
  KWin::setType( winId(), NET::Override );    // So it has no decoration

  QBoxLayout* topLayout     = new QVBoxLayout( this );

  QBoxLayout* nextLayout    = new QHBoxLayout();
  QBoxLayout* stepLayout    = new QHBoxLayout();
  QBoxLayout* focusLayout   = new QHBoxLayout();

  DbgMoveHandle*  moveHandle= new DbgMoveHandle(this);

  QPushButton*  bRun        = new DbgButton(BarIcon("dbgrun"),        i18n("Run"),        this);
  QPushButton*  bInterrupt  = new DbgButton(BarIcon("player_pause"),  i18n("Interrupt"),  this);
  QPushButton*  bNext       = new DbgButton(BarIcon("dbgnext"),       QString::null,      this);
  QPushButton*  bNexti      = new DbgButton(BarIcon("dbgnextinst"),   QString::null,      this);
  QPushButton*  bStep       = new DbgButton(BarIcon("dbgstep"),       QString::null,      this);
  QPushButton*  bStepi      = new DbgButton(BarIcon("dbgstepinst"),   QString::null,      this);
  QPushButton*  bFinish     = new DbgButton(BarIcon("dbgstepout"),    i18n("Step Out"),   this);
  QPushButton*  bView       = new DbgButton(BarIcon("dbgmemview"),    i18n("Viewers"),    this);
                bKDevFocus_ = new DbgButton(BarIcon("kdevelop"),      QString::null,      this);
                bPrevFocus_ = new DbgButton(BarIcon("dbgmemview"),    QString::null,      this);

  connect(bRun,         SIGNAL(clicked()),  ckDevelop_,     SLOT(slotDebugRun()));
  connect(bInterrupt,   SIGNAL(clicked()),  dbgController,  SLOT(slotBreakInto()));
  connect(bNext,        SIGNAL(clicked()),  dbgController,  SLOT(slotStepOver()));
  connect(bNexti,       SIGNAL(clicked()),  dbgController,  SLOT(slotStepOverIns()));
  connect(bStep,        SIGNAL(clicked()),  dbgController,  SLOT(slotStepInto()));
  connect(bStepi,       SIGNAL(clicked()),  dbgController,  SLOT(slotStepIntoIns()));
  connect(bFinish,      SIGNAL(clicked()),  dbgController,  SLOT(slotStepOutOff()));
  connect(bView,        SIGNAL(clicked()),  ckDevelop_,     SLOT(slotDebugMemoryView()));
  connect(bKDevFocus_,  SIGNAL(clicked()),                  SLOT(slotDbgKdevFocus()));
  connect(bPrevFocus_,  SIGNAL(clicked()),                  SLOT(slotDbgPrevFocus()));

  QToolTip::add( bRun,        i18n("Continue with app execution. May start the app") );
  QToolTip::add( bInterrupt,  i18n("Interrupt the app execution") );
  QToolTip::add( bNext,       i18n("Execute one line of code, but run through functions") );
  QToolTip::add( bNexti,      i18n("Execute one assembler instruction, but run through functions") );
  QToolTip::add( bStep,       i18n("Execute one line of code, stepping into fn if appropriate") );
  QToolTip::add( bStepi,      i18n("Execute one assembler instruction, stepping into fn if appropriate") );
  QToolTip::add( bFinish,     i18n("Execute to end of current stack frame") );
  QToolTip::add( bView,       i18n("Memory, dissemble, registers, library viewers") );
  QToolTip::add( bKDevFocus_, i18n("Set focus on KDevelop") );
  QToolTip::add( bPrevFocus_, i18n("Set focus on window that had focus when \"kdev\" was pressed") );

  topLayout->addWidget(moveHandle);
  topLayout->addWidget(bRun);
  topLayout->addLayout(nextLayout);
  topLayout->addLayout(stepLayout);
  topLayout->addWidget(bFinish);
  topLayout->addWidget(bView);
  topLayout->addWidget(bInterrupt);
  topLayout->addLayout(focusLayout);

  focusLayout->addWidget(bKDevFocus_);
  focusLayout->addWidget(bPrevFocus_);

  stepLayout->addWidget(bStep);
  stepLayout->addWidget(bStepi);

  nextLayout->addWidget(bNext);
  nextLayout->addWidget(bNexti);

  int w = QMAX(bRun->sizeHint().width(), bFinish->sizeHint().width());
      w = QMAX(w, bInterrupt->sizeHint().width());
      w = QMAX(w, bView->sizeHint().width());

  // they should have the same height, so don't be too fussy
  int h = bFinish->sizeHint().height();

  bNext->setMinimumHeight(h);
  bNexti->setMinimumHeight(h);
  bStep->setMinimumHeight(h);
  bStepi->setMinimumHeight(h);
  bKDevFocus_->setMinimumHeight(h);
  bPrevFocus_->setMinimumHeight(h);

  setMinimumSize(w+10, h*7);
  setMaximumSize(w+10, h*7);

  setAppIndicator(appIsActive_);
  topLayout->activate();
}

// **************************************************************************

DbgToolbar::~DbgToolbar()
{
  slotUndock();
}

// **************************************************************************

void DbgToolbar::slotDbgKdevFocus()
{
  // I really want to be able to set the focus on the _application_ being debugged
  // but this is the best compromise I can come up with. All we do is save the
  // window that had focus when they switch to the kdevelop window. To do this
  // the toolbar _cannot_ accept focus.
  // If anyone has a way of determining what window the app is _actually_ running on
  // then please fix and send a patch.

  if (winModule_->activeWindow() != ckDevelop_->winId())
    activeWindow_ = winModule_->activeWindow();

  KWin::setActiveWindow(ckDevelop_->winId());
}

// **************************************************************************

void DbgToolbar::slotDbgPrevFocus()
{
    KWin::setActiveWindow(activeWindow_);
}

// **************************************************************************

// If the app is active then the app button is highlighted, otherwise
// kdev button is highlighted.
void DbgToolbar::slotDbgStatus(const QString&,int state)
{
  bool appIndicator = state & s_appBusy;
  if (appIndicator != appIsActive_)
  {
    setAppIndicator(appIndicator);
    appIsActive_ = appIndicator;
  }
}

// **************************************************************************

void DbgToolbar::setAppIndicator(bool appIndicator)
{
  if (appIndicator)
  {
    bPrevFocus_->setPalette(QPalette(colorGroup().mid()));
    bKDevFocus_->setPalette(QPalette(colorGroup().background()));
  }
  else
  {
    bPrevFocus_->setPalette(QPalette(colorGroup().background()));
    bKDevFocus_->setPalette(QPalette(colorGroup().mid()));
  }
}

// **************************************************************************

void DbgToolbar::slotDock()
{
  if (docked_)
    return;

//  ASSERT(!docker_);
  hide();

  docker_->show();
  docked_ = true;
}

// **************************************************************************

void DbgToolbar::slotIconifyAndDock()
{
  if (docked_)
    return;

//  KWin::iconifyWindow(ckDevelop_->winId(), true);
  slotDock();
}

// **************************************************************************

void DbgToolbar::slotUndock()
{
  if (!docked_)
    return;

  show();
  docker_->hide();
  docked_ = false;
}

// **************************************************************************

void DbgToolbar::slotActivateAndUndock()
{
  if (!docked_)
    return;

  KWin::setActiveWindow(ckDevelop_->winId());
  slotUndock();
}

// **************************************************************************
#include "dbgtoolbar.moc"
