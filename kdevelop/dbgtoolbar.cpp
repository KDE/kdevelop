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

#include "kapp.h"
#include "kwm.h"

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
    DbgMoveHandle(QWidget * parent=0, const char * name=0,
                    WFlags f=0, bool allowLines=TRUE);
    virtual ~DbgMoveHandle();

    virtual void mousePressEvent(QMouseEvent *e);
    virtual void mouseReleaseEvent( QMouseEvent *e);
    virtual void mouseMoveEvent(QMouseEvent *e);

  private:
    QWidget*    parent_;
    QPoint      offset_;
    bool        moving_;
//    QRect       limit_;
};

// **************************************************************************

DbgMoveHandle::DbgMoveHandle(QWidget * parent,
                              const char * name, WFlags f, bool allowLines) :
  QFrame(parent, name, f, allowLines),
  parent_(parent),
  offset_(QPoint(0,0)),
  moving_(false)
//  limit_(KWM::getWindowRegion(KWM::desktop(winId())))
{
  setFrameStyle( QFrame::Panel|QFrame::Raised);
  setFixedHeight( 9 );
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

  moving_ = true;
  offset_ = parent_->pos() - e->globalPos();
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

  parent_->move(e->globalPos() + offset_);

// I've had the window jump off the screen once. which made it impossible
// to control the debugger. I tried the following code, but it wasn't good
// enough. The screen edges didn't work nicely, because the desktop region
// doesn't contain the panel or taskbar.

//  QPoint moveTo = e->globalPos() + offset_;

  // Make sure we never go outside the desktop region
//  if (limit_.contains(moveTo))
//    parent_->move(moveTo);
}

// **************************************************************************
// **************************************************************************
// **************************************************************************

// This class adds text _and_ a pixmap to a button. Why doesn't QPushButton
// support that? It only allowed text _or_ pixmap.
// Hmmm, not sure this stuff looks that nice. Perhaps that's why.
class DbgButton : public QPushButton
{
  public:
    DbgButton(const char* text, const QPixmap& pixmap, QWidget * parent) :
      QPushButton(parent),  text_(text),  pixmap_(pixmap) {};
    virtual ~DbgButton()  {};
    void drawButtonLabel(QPainter *painter);
  private:
    QString text_;
    QPixmap pixmap_;
};

// **************************************************************************

void DbgButton::drawButtonLabel(QPainter *painter)
{
  if (!pixmap_.isNull())
  {
    int dx;
    if (text_.isEmpty())
      dx = (width()-pixmap_.width()) / 2;
    else
      dx = (height() - pixmap_.width()) / 2;

    int dy = (height() - pixmap_.height()) / 2;
    painter->drawPixmap(dx, dy, pixmap_);
  }

  if (!text_.isEmpty())
  {
    painter->setPen(colorGroup().text());
    painter->drawText( height()+2, 0, width()-(height()+2), height(), AlignLeft|AlignVCenter, text_);
  }
}

// **************************************************************************

DbgToolbar::DbgToolbar(DbgController* dbgController, CKDevelop* parent) :
  QFrame(0, "DbgToolbar"),
  ckDevelop_(parent),
  activeWindow_(0),
  bKDevFocus_(0),
  bPrevFocus_(0)
{
  // Must have noFocus set so that we can see what window was active.
  // see slotDbgKdevFocus() for more comments
  // I do not want the user to be able to "close" this widget. If we have any
  // decoration then they can and that is bad.
  // This widget is closed when the debugger finishes i.e. they press "Stop"
  KWM::setDecoration(winId(), KWM::noFocus|KWM::noDecoration|KWM::staysOnTop);

  QBoxLayout * topLayout    = new QVBoxLayout( this );
  QBoxLayout * focusLayout  = new QHBoxLayout();

  QPixmap pm;

  DbgMoveHandle*  moveHandle  = new DbgMoveHandle(this);
  pm.load(KApplication::kde_datadir() + "/kdevelop/toolbar/dbgrun.xpm");
  DbgButton*      bRun        = new DbgButton(i18n("Run"), pm, this);

  pm.load(KApplication::kde_datadir() + "/kdevelop/toolbar/dbgbreak.xpm");
  DbgButton*    bInterrupt  = new DbgButton(i18n("Interrupt"), pm, this);

  pm.load(KApplication::kde_datadir() + "/kdevelop/toolbar/dbgnext.xpm");
  DbgButton*    bNext       = new DbgButton(i18n("Step"), pm, this);

  pm.load(KApplication::kde_datadir() + "/kdevelop/toolbar/dbgstep.xpm");
  DbgButton*    bStep       = new DbgButton(i18n("Step in"), pm, this);

  pm.load(KApplication::kde_datadir() + "/kdevelop/toolbar/dbgstepout.xpm");
  DbgButton*    bFinish     = new DbgButton(i18n("Step out"), pm, this);

  pm.load(KApplication::kde_datadir() + "/kdevelop/toolbar/dbgmemview.xpm");
  DbgButton*    bView       = new DbgButton(i18n("View"), pm, this);

  pm.load(KApplication::kde_datadir() + "/kdevelop/toolbar/dbgstop.xpm");
  DbgButton*    bStop       = new DbgButton(i18n("Stop"), pm, this);

  pm.load(KApplication::kde_icondir() + "/mini/kdevelop.xpm");
  bKDevFocus_ = new DbgButton(0, pm,  this);

  pm.load(KApplication::kde_datadir() + "/kdevelop/toolbar/dbgmemview.xpm");
  bPrevFocus_ = new DbgButton(0, pm,  this);

  connect(bRun,         SIGNAL(clicked()),  ckDevelop_,     SLOT(slotDebugRun()));
  connect(bInterrupt,   SIGNAL(clicked()),  dbgController,  SLOT(slotBreakInto()));
  connect(bNext,        SIGNAL(clicked()),  dbgController,  SLOT(slotStepOver()));
  connect(bStep,        SIGNAL(clicked()),  dbgController,  SLOT(slotStepInto()));
  connect(bFinish,      SIGNAL(clicked()),  dbgController,  SLOT(slotStepOutOff()));
  connect(bView,        SIGNAL(clicked()),  ckDevelop_,     SLOT(slotDebugMemoryView()));
  connect(bStop,        SIGNAL(clicked()),                  SLOT(slotDbgStop()));
  connect(bKDevFocus_,  SIGNAL(clicked()),                  SLOT(slotDbgKdevFocus()));
  connect(bPrevFocus_,  SIGNAL(clicked()),                  SLOT(slotDbgPrevFocus()));

  QToolTip::add( bRun,        i18n("Continue with app execution. May start the app") );
  QToolTip::add( bInterrupt,  i18n("Interrupt the app execution") );
  QToolTip::add( bNext,       i18n("Execute one line of code, but run through functions") );
  QToolTip::add( bStep,       i18n("Execute one line of code, stepping into fn if appropriate") );
  QToolTip::add( bFinish,     i18n("Execute to end of current stack frame") );
  QToolTip::add( bView,       i18n("Memory, dissemble, registers, library viewer") );
  QToolTip::add( bStop,       i18n("Stop the debugger") );
  QToolTip::add( bKDevFocus_, i18n("Set focus on KDevelop") );
  QToolTip::add( bPrevFocus_, i18n("Set focus on window that had focus when \"kdev\" was pressed") );

  int w = QMAX(bNext->sizeHint().width(),  bRun->sizeHint().width());
      w = QMAX(w, bStep->sizeHint().width());
      w = QMAX(w, bFinish->sizeHint().width());
      w = QMAX(w, bInterrupt->sizeHint().width());
      w = QMAX(w, bView->sizeHint().width());
      w = QMAX(w, bStop->sizeHint().width());

  int h = QMAX(bNext->sizeHint().height(), bRun->sizeHint().height());
      h = QMAX(h, bStep->sizeHint().height());
      h = QMAX(h, bFinish->sizeHint().height());
      h = QMAX(h, bInterrupt->sizeHint().height());
      h = QMAX(h, bView->sizeHint().height());
      h = QMAX(h, bStop->sizeHint().height());

  topLayout->addWidget(moveHandle);
  topLayout->addWidget(bRun);
  topLayout->addWidget(bNext);
  topLayout->addWidget(bStep);
  topLayout->addWidget(bFinish);
  topLayout->addWidget(bView);
  topLayout->addWidget(bInterrupt);
  topLayout->addWidget(bStop);
  topLayout->addLayout(focusLayout);

  focusLayout->addWidget(bKDevFocus_);
  focusLayout->addWidget(bPrevFocus_);

  setMinimumSize(w+16, h*8);
  setMaximumSize(w+16, h*8);

  topLayout->activate();
}

// **************************************************************************

DbgToolbar::~DbgToolbar()
{
}

// **************************************************************************

void DbgToolbar::slotDbgStop()
{
  ckDevelop_->slotDebugStop();
  close();
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
  if (state & (s_appBusy|s_waitForWrite))
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
