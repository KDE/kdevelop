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
#include "ckdevelop.h"

#include <kapp.h>
#include <kpopmenu.h>
#include <kwm.h>

#include <qlayout.h>
#include <qpushbutton.h>
#include <qframe.h>

#include "./dbg/dbgcontroller.h"

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
  DbgMoveHandle(DbgToolbar* parent=0, const char * name=0,
                  WFlags f=0, bool allowLines=TRUE);
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
                              const char * name, WFlags f, bool allowLines) :
  QFrame(toolBar, name, f, allowLines),
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
    menu->setTitle("Debug toolbar");
    menu->insertItem(i18n("Dock to panel"),                     toolBar_, SLOT(slotDock()));
    menu->insertItem(i18n("Dock to panel and iconify kDevelop"),toolBar_, SLOT(slotIconifyAndDock()));
    menu->popup(e->globalPos());
    return;
  }

  moving_ = true;
  offset_ = toolBar_->pos() - e->globalPos();
  setFrameStyle( QFrame::Panel|QFrame::Sunken);
  QApplication::setOverrideCursor(QCursor(sizeAllCursor));
  setPalette(QPalette(kapp->selectColor));
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
  setPalette(QPalette(kapp->backgroundColor));
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
  DbgButton(const char* text, const QPixmap& pixmap, DbgToolbar* parent) :
    QPushButton(parent),
    pixmap_(pixmap)
    { setText(text); }
  virtual ~DbgButton()  {};
  void drawButtonLabel(QPainter *painter);
  QSize sizeHint() const;

private:
  QPixmap     pixmap_;
};

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
	
  return QPushButton::sizeHint();	
}

// **************************************************************************
// **************************************************************************
// **************************************************************************

DbgDocker::DbgDocker(DbgToolbar* toolBar, const QPixmap& pixmap) :
  QLabel(0, "DbgDocker"),    // Cannot have a parent !!!
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
      // Not really a click, but it'll hold for the time being !!!
      emit clicked();
      break;

    case RightButton:
      KPopupMenu* menu = new KPopupMenu( this );
      menu->setTitle("Debug toolbar");
      menu->insertItem(i18n("Activate"),                        toolBar_, SLOT(slotUndock()));
      menu->insertItem(i18n("Activate (KDevelop gets focus)"),  toolBar_, SLOT(slotActivateAndUndock()));
      menu->popup(e->globalPos());
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
  bKDevFocus_(0),
  bPrevFocus_(0),
  appIsActive_(false),
  docked_(false),
  docker_(0)
{
  // Must have noFocus set so that we can see what window was active.
  // see slotDbgKdevFocus() for more comments
  // I do not want the user to be able to "close" this widget. If we have any
  // decoration then they can and that is bad.
  // This widget is closed when the debugger finishes i.e. they press "Stop"
  KWM::setDecoration(winId(), KWM::noFocus|KWM::noDecoration|KWM::staysOnTop);

  QBoxLayout* topLayout   = new QVBoxLayout( this );

  QBoxLayout* nextLayout  = new QHBoxLayout();
  QBoxLayout* stepLayout  = new QHBoxLayout();
  QBoxLayout* focusLayout = new QHBoxLayout();

  QPixmap pm;

  DbgMoveHandle*  moveHandle  = new DbgMoveHandle(this);

  pm.load(KApplication::kde_datadir() + "/kdevelop/toolbar/dbgrun.xpm");
  DbgButton*      bRun        = new DbgButton(i18n("Run"), pm, this);

  pm.load(KApplication::kde_datadir() + "/kdevelop/toolbar/dbgbreak.xpm");
  DbgButton*    bInterrupt  = new DbgButton(i18n("Interrupt"), pm, this);

  pm.load(KApplication::kde_datadir() + "/kdevelop/toolbar/dbgnext.xpm");
  DbgButton*    bNext       = new DbgButton(0, pm, this);

  pm.load(KApplication::kde_datadir() + "/kdevelop/toolbar/dbgnextinst.xpm");
  DbgButton*    bNexti      = new DbgButton(0, pm, this);

  pm.load(KApplication::kde_datadir() + "/kdevelop/toolbar/dbgstep.xpm");
  DbgButton*    bStep       = new DbgButton(0, pm, this);

  pm.load(KApplication::kde_datadir() + "/kdevelop/toolbar/dbgstepinst.xpm");
  DbgButton*    bStepi      = new DbgButton(0, pm, this);

  pm.load(KApplication::kde_datadir() + "/kdevelop/toolbar/dbgstepout.xpm");
  DbgButton*    bFinish     = new DbgButton(i18n("Step out"), pm, this);

  pm.load(KApplication::kde_datadir() + "/kdevelop/toolbar/dbgmemview.xpm");
  DbgButton*    bView       = new DbgButton(i18n("Viewers"), pm, this);

  pm.load(KApplication::kde_icondir() + "/mini/kdevelop.xpm");
  bKDevFocus_ = new DbgButton(0, pm,  this);

  pm.load(KApplication::kde_datadir() + "/kdevelop/toolbar/dbgmemview.xpm");
  bPrevFocus_ = new DbgButton(0, pm,  this);

  bStep->setAccel(CTRL+'S');

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

  setMinimumSize(w, h*7);
  setMaximumSize(w, h*7);

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
  if (KWM::activeWindow() != ckDevelop_->winId())
    activeWindow_ = KWM::activeWindow();

  KWM::activate(ckDevelop_->winId());
}

// **************************************************************************

void DbgToolbar::slotDbgPrevFocus()
{
  KWM::activate(activeWindow_);
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
    bKDevFocus_->setPalette(QPalette(kapp->backgroundColor));
  }
  else
  {
    bPrevFocus_->setPalette(QPalette(kapp->backgroundColor));
    bKDevFocus_->setPalette(QPalette(colorGroup().mid()));
  }
}

// **************************************************************************

void DbgToolbar::slotDock()
{
  if (docked_)
    return;

  ASSERT(!docker_);
  hide();
  QPixmap pm;
  pm.load(KApplication::kde_datadir() + "/kdevelop/toolbar/dbgnext.xpm");
  docker_ = new DbgDocker(this, pm);
  connect(docker_, SIGNAL(clicked()),  dbgController_,  SLOT(slotStepOver()));
  KWM::setDockWindow(docker_->winId());
  docker_->show();
  docked_ = true;
}

// **************************************************************************

void DbgToolbar::slotIconifyAndDock()
{
  if (docked_)
    return;

  KWM::setIconify(ckDevelop_->winId(), true);
  slotDock();
}

// **************************************************************************

void DbgToolbar::slotUndock()
{
  if (!docked_)
    return;

  ASSERT(docker_);
  //TODO - this is bad - we want to remove the docker_ _in_ a call
  // from the docker_. Yuk!!!
  // Should create()/destroy() the window?, and always have the docked widget
  // instantiated.?
  docker_->close();
  delete docker_;
  docker_ = 0;
  show();
  docked_ = false;
}

// **************************************************************************

void DbgToolbar::slotActivateAndUndock()
{
  if (!docked_)
    return;

  KWM::activate(ckDevelop_->winId());
  slotUndock();
}

// **************************************************************************
