
/* This file is part of the KDE libraries
   Copyright (C) 2000 Max Judin <novaprint@mtu-net.ru>
   Copyright (C) 2002,2003 Joseph Wenninger <jowenn@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/
#include "kdockwidget_compat.h"

#include <qapplication.h>
#include <qlayout.h>
#include <qpainter.h>
#include <qobjectlist.h>
#include <qstrlist.h>
#include <qcursor.h>
#include <qwidgetlist.h>
#include <qtabwidget.h>
#include <qtooltip.h>
#include <qstyle.h>
#include <qpushbutton.h>
#include <qguardedptr.h>

#ifndef NO_KDE2
#include <kconfig.h>
#include <kglobal.h>
#include <klocale.h>
#include <ktoolbar.h>
#include <kpopupmenu.h>
#include <kwin.h>
#include <kdebug.h>
#ifdef Q_WS_X11
#include <X11/X.h>
#include <X11/Xlib.h>
#endif
#else
#include <qtoolbar.h>
#include <qpopupmenu.h>
#endif

#include <stdlib.h>

#ifndef NO_KDE2
#include <netwm_def.h>
#endif

#undef BORDERLESS_WINDOWS

#define DOCK_CONFIG_VERSION "0.0.5"

using namespace KDockWidget_Compat;

static const char* const dockback_xpm[]={
"6 6 2 1",
"# c black",
". c None",
"......",
".#....",
"..#..#",
"...#.#",
"....##",
"..####"};

static const char* const todesktop_xpm[]={
"5 5 2 1",
"# c black",
". c None",
"####.",
"##...",
"#.#..",
"#..#.",
"....#"};

static const char* const not_close_xpm[]={
"5 5 2 1",
"# c black",
". c None",
"#####",
"#...#",
"#...#",
"#...#",
"#####"};

/**
 * Like QSplitter but specially designed for dockwidgets stuff.
 * @internal
 *
 * @author Max Judin.
*/
class KDockSplitter : public QWidget
{
  Q_OBJECT
public:
  KDockSplitter(QWidget *parent= 0, const char *name= 0, Orientation orient= Vertical, int pos= 50, bool highResolution=false);  
  virtual ~KDockSplitter(){};

  void activate(QWidget *c0, QWidget *c1 = 0L);
  void deactivate();

  int separatorPos() const;
  void setSeparatorPos(int pos, bool do_resize = true);

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
};

/**
 * A mini-button usually placed in the dockpanel.
 * @internal
 *
 * @author Max Judin.
*/
class KDockButton_Private : public QPushButton
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
class KDockWidgetPrivate : public QObject
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

class KDockWidgetHeaderPrivate
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

/**
 * A special kind of KMainWindow that is able to have dockwidget child widgets.
 *
 * The main widget should be a dockwidget where other dockwidgets can be docked to
 * the left, right, top, bottom or to the middle.
 * Furthermore, the KDockMainWindow has got the KDocManager and some data about the dock states.
 *
 * @author Max Judin.
*/
KDockMainWindow::KDockMainWindow( QWidget* parent, const char *name, WFlags f)
:KMainWindow( parent, name, f )
{
  QString new_name = QString(name) + QString("_DockManager");
  dockManager = new KDockManager( this, new_name.latin1() );
  mainDockWidget = 0L;
}

KDockMainWindow::~KDockMainWindow()
{
	delete dockManager;
}

void KDockMainWindow::setMainDockWidget( KDockWidget* mdw )
{
  if ( mainDockWidget == mdw ) return;
  mainDockWidget = mdw;
  dockManager->setMainDockWidget2(mdw);
}

void KDockMainWindow::setView( QWidget *view )
{
  if ( view->isA("KDockWidget_Compat::KDockWidget") ){
    if ( view->parent() != this ) ((KDockWidget*)view)->applyToWidget( this );
  }

#ifndef NO_KDE2
  KMainWindow::setCentralWidget(view);
#else
  QMainWindow::setCentralWidget(view);
#endif
}

KDockWidget* KDockMainWindow::createDockWidget( const QString& name, const QPixmap &pixmap, QWidget* parent, const QString& strCaption, const QString& strTabPageLabel)
{
  return new KDockWidget( dockManager, name.latin1(), pixmap, parent, strCaption, strTabPageLabel );
}

void KDockMainWindow::makeDockVisible( KDockWidget* dock )
{
  if ( dock != 0L)
    dock->makeDockVisible();
}

void KDockMainWindow::makeDockInvisible( KDockWidget* dock )
{
  if ( dock != 0L)
    dock->undock();
}

void KDockMainWindow::makeWidgetDockVisible( QWidget* widget )
{
  makeDockVisible( dockManager->findWidgetParentDock(widget) );
}

void KDockMainWindow::writeDockConfig(QDomElement &base)
{
  dockManager->writeConfig(base);
}

void KDockMainWindow::readDockConfig(QDomElement &base)
{
  dockManager->readConfig(base);
}

#ifndef NO_KDE2
void KDockMainWindow::writeDockConfig( KConfig* c, QString group )
{
  dockManager->writeConfig( c, group );
}

void KDockMainWindow::readDockConfig( KConfig* c, QString group )
{
  dockManager->readConfig( c, group );
}
#endif

void KDockMainWindow::slotDockWidgetUndocked()
{
  QObject* pSender = (QObject*) sender();
  if (!pSender->inherits("KDockWidget_Compat::KDockWidget")) return;
  KDockWidget* pDW = (KDockWidget*) pSender;
  emit dockWidgetHasUndocked( pDW);
}

/*************************************************************************/
KDockWidgetAbstractHeaderDrag::KDockWidgetAbstractHeaderDrag( KDockWidgetAbstractHeader* parent, KDockWidget* dock, const char* name )
:QFrame( parent, name )
{
  dw = dock;
  installEventFilter( dock->dockManager() );
}
/*************************************************************************/
KDockWidgetHeaderDrag::KDockWidgetHeaderDrag( KDockWidgetAbstractHeader* parent, KDockWidget* dock, const char* name )
:KDockWidgetAbstractHeaderDrag( parent, dock, name )
{
}

void KDockWidgetHeaderDrag::paintEvent( QPaintEvent* )
{
  QPainter paint;

  paint.begin( this );

  style().drawPrimitive (QStyle::PE_DockWindowHandle, &paint, QRect(0,0,width(), height()), colorGroup());

  paint.end();
}
/*************************************************************************/
KDockWidgetAbstractHeader::KDockWidgetAbstractHeader( KDockWidget* parent, const char* name )
:QFrame( parent, name )
{
}
/*************************************************************************/
KDockWidgetHeader::KDockWidgetHeader( KDockWidget* parent, const char* name )
:KDockWidgetAbstractHeader( parent, name )
{
#ifdef BORDERLESS_WINDOWS
  setCursor(QCursor(ArrowCursor));
#endif
  d = new KDockWidgetHeaderPrivate( this );

  layout = new QHBoxLayout( this );
  layout->setResizeMode( QLayout::Minimum );

  drag = new KDockWidgetHeaderDrag( this, parent );

  closeButton = new KDockButton_Private( this, "DockCloseButton" );
  QToolTip::add( closeButton, i18n("Close") );
  closeButton->setPixmap( style().stylePixmap (QStyle::SP_TitleBarCloseButton , this));
  closeButton->setFixedSize(closeButton->pixmap()->width(),closeButton->pixmap()->height());
  connect( closeButton, SIGNAL(clicked()), parent, SIGNAL(headerCloseButtonClicked()));
  connect( closeButton, SIGNAL(clicked()), parent, SLOT(undock()));

  stayButton = new KDockButton_Private( this, "DockStayButton" );
  QToolTip::add( stayButton, i18n("Freeze the window geometry", "Freeze") );
  stayButton->setToggleButton( true );
  stayButton->setPixmap( const_cast< const char** >(not_close_xpm) );
  stayButton->setFixedSize(closeButton->pixmap()->width(),closeButton->pixmap()->height());
  connect( stayButton, SIGNAL(clicked()), this, SLOT(slotStayClicked()));

  dockbackButton = new KDockButton_Private( this, "DockbackButton" );
  QToolTip::add( dockbackButton, i18n("Dock this window", "Dock") );
  dockbackButton->setPixmap( const_cast< const char** >(dockback_xpm));
  dockbackButton->setFixedSize(closeButton->pixmap()->width(),closeButton->pixmap()->height());
  connect( dockbackButton, SIGNAL(clicked()), parent, SIGNAL(headerDockbackButtonClicked()));
  connect( dockbackButton, SIGNAL(clicked()), parent, SLOT(dockBack()));

  d->toDesktopButton = new KDockButton_Private( this, "ToDesktopButton" );
  QToolTip::add( d->toDesktopButton, i18n("Detach") );
  d->toDesktopButton->setPixmap( const_cast< const char** >(todesktop_xpm));
  d->toDesktopButton->setFixedSize(closeButton->pixmap()->width(),closeButton->pixmap()->height());
  connect( d->toDesktopButton, SIGNAL(clicked()), parent, SLOT(toDesktop()));
  stayButton->hide();

  d->dummy = new QWidget( this );
  d->dummy->setFixedSize( 1,closeButton->pixmap()->height() );

  
  layout->addWidget( drag );
  layout->addWidget( dockbackButton );
  layout->addWidget( d->toDesktopButton );
  layout->addWidget( d->dummy);
  layout->addWidget( stayButton );
  layout->addWidget( closeButton );
  layout->activate();
  d->dummy->hide();
  drag->setFixedHeight( layout->minimumSize().height() );
}

void KDockWidgetHeader::setTopLevel( bool isTopLevel )
{
  d->topLevel = isTopLevel;
  if ( isTopLevel ){
    KDockWidget* par = (KDockWidget*)parent();
    if( par) {
      if( par->isDockBackPossible())
        dockbackButton->show();
      else
        dockbackButton->hide();
    }
    stayButton->hide();
    closeButton->hide();
    d->toDesktopButton->hide();
    drag->setEnabled( true );
  } else {
    dockbackButton->hide();
    stayButton->hide();
    if (!d->forceCloseButtonHidden) closeButton->show();
    if( d->showToDesktopButton )
      d->toDesktopButton->show();
  }
  layout->activate();
  
   bool dontShowDummy=drag->isVisibleTo(this) || dockbackButton->isVisibleTo(this) ||
        d->toDesktopButton->isVisibleTo(this) || stayButton->isVisibleTo(this) ||
        closeButton->isVisibleTo(this);
   for (QPtrListIterator<KDockButton_Private> it( d->btns );it.current()!=0;++it) {
        dontShowDummy=dontShowDummy || (it.current()->isVisibleTo(this));
   }
   if (dontShowDummy) d->dummy->hide(); else d->dummy->show();

  updateGeometry();
}

void KDockWidgetHeader::forceCloseButtonHidden(bool hidden) {
  d->forceCloseButtonHidden=hidden;
  if (hidden) closeButton->hide();
  else closeButton->show();
}

void KDockWidgetHeader::setDragPanel( KDockWidgetHeaderDrag* nd )
{
  if ( !nd ) return;

  delete layout;
  layout = new QHBoxLayout( this );
  layout->setResizeMode( QLayout::Minimum );

  delete drag;
  drag = nd;

  layout->addWidget( drag );
  layout->addWidget( dockbackButton );
  layout->addWidget( d->dummy );
  layout->addWidget( d->toDesktopButton );
  layout->addWidget( stayButton );
  bool dontShowDummy=drag->isVisibleTo(this) || dockbackButton->isVisibleTo(this) ||
	d->toDesktopButton->isVisibleTo(this) || stayButton->isVisibleTo(this) ||
	closeButton->isVisibleTo(this);
  for (QPtrListIterator<KDockButton_Private> it( d->btns );it.current()!=0;++it) {
      layout->addWidget(it.current());
	dontShowDummy=dontShowDummy || (it.current()->isVisibleTo(this));
  }
  if (dontShowDummy) d->dummy->hide(); else d->dummy->show();
  layout->addWidget( closeButton );
  layout->activate();
  drag->setFixedHeight( layout->minimumSize().height() );
}

void KDockWidgetHeader::addButton(KDockButton_Private* btn) {
	if (!btn) return;

	if (btn->parentWidget()!=this) {
		btn->reparent(this,QPoint(0,0));
	}
	btn->setFixedSize(closeButton->pixmap()->width(),closeButton->pixmap()->height());
	if (!d->btns.containsRef(btn)) d->btns.append(btn);

	btn->show();

	delete layout;
	layout = new QHBoxLayout( this );
	layout->setResizeMode( QLayout::Minimum );

	layout->addWidget( drag );
 	layout->addWidget( dockbackButton );
	layout->addWidget( d->toDesktopButton );
	layout->addWidget( d->dummy);
	layout->addWidget( stayButton );
	 bool dontShowDummy=drag->isVisibleTo(this) || dockbackButton->isVisibleTo(this) ||
	        d->toDesktopButton->isVisibleTo(this) || stayButton->isVisibleTo(this) ||
        	closeButton->isVisibleTo(this);
	 for (QPtrListIterator<KDockButton_Private> it( d->btns );it.current()!=0;++it) {
	        layout->addWidget(it.current());
		dontShowDummy=dontShowDummy || (it.current()->isVisibleTo(this));
   	}
  	if (dontShowDummy) d->dummy->hide(); else d->dummy->show();
	layout->addWidget( closeButton );
	layout->activate();
	drag->setFixedHeight( layout->minimumSize().height() );
}

void KDockWidgetHeader::removeButton(KDockButton_Private* btn) {
	if (btn->parentWidget()==this) {
		if (d->btns.containsRef(btn)) d->btns.removeRef(btn);
		delete btn;
	}
}


void KDockWidgetHeader::slotStayClicked()
{
  setDragEnabled(!stayButton->isOn());
}

bool KDockWidgetHeader::dragEnabled() const
{
  return drag->isEnabled();
}

void KDockWidgetHeader::showUndockButton(bool show)
{
//  kdDebug()<<"KDockWidgetHeader::showUndockButton("<<show<<")"<<endl;
  if( d->showToDesktopButton == show )
    return;

  d->showToDesktopButton = show;
  if( !show || d->topLevel )
    d->toDesktopButton->hide( );
  else
    d->toDesktopButton->show( );
}

void KDockWidgetHeader::setDragEnabled(bool b)
{
  stayButton->setOn(!b);
  closeButton->setEnabled(b);
  drag->setEnabled(b);
}

#ifndef NO_KDE2
void KDockWidgetHeader::saveConfig( KConfig* c )
{
  c->writeEntry( QString("%1%2").arg(parent()->name()).arg(":stayButton"), stayButton->isOn() );
}

void KDockWidgetHeader::loadConfig( KConfig* c )
{
  setDragEnabled( !c->readBoolEntry( QString("%1%2").arg(parent()->name()).arg(":stayButton"), false ) );
}
#endif

/*************************************************************************/

class KDockManager::KDockManagerPrivate
{
public:
  /**
   * This rectangle is used to highlight the current dockposition. It stores global screen coordinates.
   */
  QRect dragRect;

  /**
   * This rectangle is used to erase the previously highlighted dockposition. It stores global screen coordinates.
   */
  QRect oldDragRect;

  /**
   * This flag stores the information if dragging is ready to start. Used between mousePress and mouseMove event.
   */
  bool readyToDrag;

  /**
   * This variable stores the offset of the mouse cursor to the upper left edge of the current drag widget.
   */
  QPoint dragOffset;

  /**
   * These flags store information about the splitter behaviour
   */
  bool splitterOpaqueResize;
  bool splitterKeepSize;
  bool splitterHighResolution;

  QGuardedPtr<KDockWidget> mainDockWidget;

  QObjectList containerDocks;
};


/*************************************************************************/
KDockWidget::KDockWidget( KDockManager* dockManager, const char* name, const QPixmap &pixmap, QWidget* parent, const QString& strCaption, const QString& strTabPageLabel, WFlags f)
#ifdef BORDERLESS_WINDOWS
: QWidget( parent, name, f )//| WType_Dialog | WStyle_Customize | WStyle_NoBorder )
#else
: QWidget( parent, name, f )
#endif
  ,formerBrotherDockWidget(0L)
  ,currentDockPos(DockNone)
  ,formerDockPos(DockNone)
  ,widget(0L)
  ,pix(new QPixmap(pixmap))
  ,prevSideDockPosBeforeDrag(DockNone)
{
  d = new KDockWidgetPrivate();  // create private data

  d->_parent = parent;

  layout = new QVBoxLayout( this );
  layout->setResizeMode( QLayout::Minimum );

  manager = dockManager;
  manager->childDock->append( this );
  installEventFilter( manager );

  header = 0L;
  setHeader( new KDockWidgetHeader( this, "AutoCreatedDockHeader" ) );

  if( strCaption.isNull() )
    setCaption( name );
  else
    setCaption( strCaption);

  if( strTabPageLabel == " ")
    setTabPageLabel( caption());
  else
    setTabPageLabel( strTabPageLabel);

  eDocking = DockFullDocking;
  sDocking = DockFullSite;

  isGroup = false;
  isTabGroup = false;
  d->isContainer =false;
  setIcon( pixmap);
  widget = 0L;

  QObject::connect(this, SIGNAL(hasUndocked()), manager->main, SLOT(slotDockWidgetUndocked()) );
  applyToWidget( parent, QPoint(0,0) );
}

void KDockWidget::setPixmap(const QPixmap& pixmap) {
	delete pix;
	pix=new QPixmap(pixmap);
	setIcon(*pix);
	KDockTabGroup *dtg=parentDockTabGroup();
	if (dtg)
		dtg->changeTab(this,pixmap,dtg->tabLabel(this));
	 QWidget *contWid=parentDockContainer();
         if (contWid)
         	dynamic_cast<KDockContainer*>(contWid)->setPixmap(this,pixmap);


}

const QPixmap& KDockWidget::pixmap() const {
	return *pix;
}

KDockWidget::~KDockWidget()
{
  d->pendingDtor = true;
  if ( !manager->undockProcess ){
    d->blockHasUndockedSignal = true;
    undock();
    d->blockHasUndockedSignal = false;
  }

  if (latestKDockContainer()) dynamic_cast<KDockContainer*>(latestKDockContainer())->removeWidget(this);
  emit iMBeingClosed();
  if (manager->d) manager->d->containerDocks.remove(this);
  manager->childDock->remove( this );
  delete pix;
  delete d; // destroy private data
  d=0;
}

void KDockWidget::paintEvent(QPaintEvent* pe)
{
	QWidget::paintEvent(pe);
        QPainter paint;
        paint.begin( this );
        style().drawPrimitive (QStyle::PE_Panel, &paint, QRect(0,0,width(), height()), colorGroup());
        paint.end();
}

void KDockWidget::leaveEvent(QEvent *e)
{
	QWidget::leaveEvent(e);
#ifdef BORDERLESS_WINDOWS
	if (parent()) return;
//	setCursor(QCursor(ArrowCursor));
#endif
}

void KDockWidget::mousePressEvent(QMouseEvent* mme)
{
#ifdef BORDERLESS_WINDOWS
	if (!parent())
	{
		kdDebug()<<"KDockWidget::mousePressEvent"<<endl;

		bool bbottom;
		bool bleft;
		bool bright;
		bool btop;
		int styleheight;
		QPoint mp;
		mp=mme->pos();
      		styleheight=2*style().pixelMetric(QStyle::PM_DefaultFrameWidth,this);
		bbottom=mp.y()>=height()-styleheight;
		btop=mp.y()<=styleheight;
		bleft=mp.x()<=styleheight;
		bright=mp.x()>=width()-styleheight;
		kdDebug()<<"mousemovevent"<<endl;
 		d->resizing=true;
		if (bright)
		{
			if (btop)
			{
				d->resizeMode=KDockWidgetPrivate::ResizeTopRight;
				d->resizePos=QPoint(width(),0)-mme->pos();

			}
			else
			{
				d->resizePos=QPoint(width(),height())-mme->pos();
				if (bbottom) d->resizeMode=KDockWidgetPrivate::ResizeBottomRight;
				else d->resizeMode=KDockWidgetPrivate::ResizeRight;
			}
		}
		else if (bleft)
		{
			if (btop) setCursor(QCursor(SizeFDiagCursor));
			else
			if (bbottom) setCursor(QCursor(SizeBDiagCursor));
			else setCursor(QCursor(SizeHorCursor));
		}
		else
		if (bbottom)
		{
			d->resizeMode=KDockWidgetPrivate::ResizeBottom;
			d->resizePos=QPoint(0,height())-mme->pos();
		}
		else
		if  (btop) setCursor(QCursor(SizeVerCursor));
		else d->resizing=false;

		if (d->resizing) grabMouse(cursor());

	}
#endif
	QWidget::mousePressEvent(mme);
}

void KDockWidget::mouseReleaseEvent(QMouseEvent* ev)
{
#ifdef BORDERLESS_WINDOWS
	d->resizing=false;
	releaseMouse();
#endif
	QWidget::mouseReleaseEvent(ev);
}

void  KDockWidget::mouseMoveEvent(QMouseEvent* mme)
{
	QWidget::mouseMoveEvent(mme);
#ifdef BORDERLESS_WINDOWS
	if (parent()) return;

	if (d->resizing)
	{
		switch (d->resizeMode)
		{
			case KDockWidgetPrivate::ResizeRight:
				resize(mme->pos().x()+d->resizePos.x(),height());
				break;
			case KDockWidgetPrivate::ResizeBottomRight:
				resize(mme->pos().x()+d->resizePos.x(),mme->pos().y()+d->resizePos.y());
				break;
			case KDockWidgetPrivate::ResizeBottom:
				resize(width(),mme->pos().y()+d->resizePos.y());
				break;
			default:
				break;
		}
		return;
	}


	bool bbottom;
	bool bleft;
	bool bright;
	bool btop;
	int styleheight;
	QPoint mp;
	mp=mme->pos();
      	styleheight=2*style().pixelMetric(QStyle::PM_DefaultFrameWidth,this);
	bbottom=mp.y()>=height()-styleheight;
	btop=mp.y()<=styleheight;
	bleft=mp.x()<=styleheight;
	bright=mp.x()>=width()-styleheight;
	kdDebug()<<"mousemovevent"<<endl;
	if (bright)
	{
		if (btop) setCursor(QCursor(SizeBDiagCursor));
		else
		if (bbottom) setCursor(QCursor(SizeFDiagCursor));
		else setCursor(QCursor(SizeHorCursor));
	}
	else if (bleft)
	{
		if (btop) setCursor(QCursor(SizeFDiagCursor));
		else
		if (bbottom) setCursor(QCursor(SizeBDiagCursor));
		else setCursor(QCursor(SizeHorCursor));
	}
	else
	if (bbottom ||  btop) setCursor(QCursor(SizeVerCursor));
	else setCursor(QCursor(ArrowCursor));
#endif
}

void KDockWidget::setLatestKDockContainer(QWidget* container)
{
	if (container)
	{
		if (dynamic_cast<KDockContainer*>(container))
		d->container=container;
		else
		d->container=0;
	}
}

QWidget* KDockWidget::latestKDockContainer()
{
	if (!(d->container)) return 0;
    QWidget* w = d->container.operator->();
	if (dynamic_cast<KDockContainer*>(w)) return d->container;
	return 0;
}



KDockWidgetAbstractHeader *KDockWidget::getHeader() {
	return header;
}

void KDockWidget::setHeader( KDockWidgetAbstractHeader* h )
{
  if ( !h ) return;

  if ( header ){
    delete header;
    delete layout;
    header = h;
    layout = new QVBoxLayout( this );
    layout->setResizeMode( QLayout::Minimum );
    layout->addWidget( header );
     setWidget( widget );
  } else {
    header = h;
    layout->addWidget( header );
  }
}

void KDockWidget::setEnableDocking( int pos )
{
  eDocking = pos;
  if( header && header->inherits( "KDockWidget_Compat::KDockWidgetHeader" ) )
     ( ( KDockWidgetHeader* ) header )->showUndockButton( pos & DockDesktop );
  updateHeader();
}

void KDockWidget::updateHeader()
{
  if ( parent() ){
#ifdef BORDERLESS_WINDOWS
      layout->setMargin(0);
      setMouseTracking(false);
      setCursor(QCursor(ArrowCursor));
#endif

    if ( (parent() == manager->main) || isGroup || (eDocking == KDockWidget::DockNone) ){
      header->hide();
    } else {
      header->setTopLevel( false );
      if (widget && dynamic_cast<KDockContainer*>(widget)) header->hide(); else
      header->show();
    }
  } else {
    header->setTopLevel( true );
    header->show();
#ifdef BORDERLESS_WINDOWS
      layout->setMargin(2*style().pixelMetric(QStyle::PM_DefaultFrameWidth,this));
      setMouseTracking(true);
#endif
  }
}

void KDockWidget::applyToWidget( QWidget* s, const QPoint& p )
{
  if ( parent() != s )
  {
    hide();
    reparent(s, 0, QPoint(0,0), false);
  }

  if ( s && (s->inherits("KDockWidget_Compat::KDockMainWindow") || s->inherits("KDockMainWindow")) ){
    ((KDockMainWindow*)s)->setView( this );
  }

  if ( s == manager->main ){
      setGeometry( QRect(QPoint(0,0), manager->main->geometry().size()) );
  }

  if ( !s )
  {
    move(p);

#ifndef NO_KDE2
#ifdef Q_WS_X11
    if (d->transient && d->_parent)
      XSetTransientForHint( qt_xdisplay(), winId(), d->_parent->winId() );

#ifdef BORDERLESS_WINDOWS
    KWin::setType( winId(), NET::Override); //d->windowType );
//      setWFlags(WStyle_Customize | WStyle_NoBorder | WStyle_Tool);
#else
    KWin::setType( winId(), d->windowType );
#endif
#endif
#endif

  }
  updateHeader();

  setIcon(*pix);
}

void KDockWidget::show()
{
  if ( parent() || manager->main->isVisible() )
    if ( !parent() ){
     emit manager->setDockDefaultPos( this );
     emit setDockDefaultPos();
     if ( parent() ){
        makeDockVisible();
      } else {
        QWidget::show();
      }
    } else {
     QWidget::show();
    }
}

#ifndef NO_KDE2

void KDockWidget::setDockWindowType (NET::WindowType windowType)
{
  d->windowType = windowType;
  applyToWidget( parentWidget(), QPoint(0,0) );
}

#endif

void KDockWidget::setDockWindowTransient (QWidget *parent, bool transientEnabled)
{
  d->_parent = parent;
  d->transient = transientEnabled;
  applyToWidget( parentWidget(), QPoint(0,0) );
}

QWidget *KDockWidget::transientTo() {
	if (d->transient && d->_parent) return d->_parent; else return 0;
}

bool KDockWidget::event( QEvent *event )
{
  switch ( event->type() )
  {
    #undef FocusIn
    case QEvent::FocusIn:
      if (widget && !d->pendingFocusInEvent) {
         d->pendingFocusInEvent = true;
         widget->setFocus();
      }
      d->pendingFocusInEvent = false;
      break;
    case QEvent::ChildRemoved:
      if ( widget == ((QChildEvent*)event)->child() ) widget = 0L;
      break;
    case QEvent::Show:
      if ( widget ) widget->show();
      emit manager->change();
      break;
    case QEvent::Hide:
      if ( widget ) widget->hide();
      emit manager->change();
      break;
    case QEvent::CaptionChange:
      if ( parentWidget() ){
        if ( parent()->inherits("KDockSplitter") ){
          ((KDockSplitter*)(parent()))->updateName();
        }
        if ( parentDockTabGroup() ){
          setDockTabName( parentDockTabGroup() );
          parentDockTabGroup()->setTabLabel( this, tabPageLabel() );
        }
      }
      break;
    case QEvent::Close:
      emit iMBeingClosed();
      break;
    default:
      break;
  }
  return QWidget::event( event );
}

KDockWidget *KDockWidget::findNearestDockWidget(DockPosition pos)
{
	if (!parent()) return 0;
	if (!parent()->inherits("KDockSplitter")) return 0;
	Orientation orientation=((pos==DockLeft) || (pos==DockRight)) ? Vertical:Horizontal;
		if (((KDockSplitter*)(parent()))->orientation()==orientation)
		{
			KDockWidget *neighbour=
				((pos==DockLeft)||(pos==DockTop))?
				static_cast<KDockWidget*>(((KDockSplitter*)(parent()))->getFirst()):
				static_cast<KDockWidget*>(((KDockSplitter*)(parent()))->getLast());

			if (neighbour==this)
			return (static_cast<KDockWidget*>(parent()->parent())->findNearestDockWidget(pos));
			else
			if (neighbour->getWidget() && (neighbour->getWidget()->qt_cast("KDockWidget_Compat::KDockTabGroup")))
				return (KDockWidget*)(((KDockTabGroup*)neighbour->getWidget())->page(0));
			else
			return neighbour;
		}
		else
		return (static_cast<KDockWidget*>(parent()->parent())->findNearestDockWidget(pos));

	return 0;
}


KDockWidget* KDockWidget::manualDock( KDockWidget* target, DockPosition dockPos, int spliPos, QPoint pos, bool check, int tabIndex )
{
  if (this == target)
    return 0L;  // docking to itself not possible
//  kdDebug()<<"manualDock called "<<endl;
  bool succes = true; // tested flag

  // check allowed this dock submit this operations
  if ( !(eDocking & (int)dockPos) ){
    succes = false;
//  kdDebug()<<"KDockWidget::manualDock(): success = false (1)"<<endl;
  }

  // check allowed target submit this operations
  if ( target && !(target->sDocking & (int)dockPos) ){
    succes = false;
//  kdDebug()<<"KDockWidget::manualDock(): success = false (2)"<<endl;
  }

  if ( parent() && !parent()->inherits("KDockSplitter") && !parentDockTabGroup() &&
  	!(dynamic_cast<KDockContainer*>(parent())) && !parentDockContainer()){
//  kdDebug()<<"KDockWidget::manualDock(): success = false (3)"<<endl;
//  kdDebug()<<parent()->name()<<endl;
    succes = false;
  }

//  kdDebug()<<"KDockWidget::manualDock(): success == false "<<endl;
  if ( !succes ){
    // try to make another manualDock
    KDockWidget* dock_result = 0L;
    if ( target && !check ){
      KDockWidget::DockPosition another__dockPos = KDockWidget::DockNone;
      switch ( dockPos ){
        case KDockWidget::DockLeft  : another__dockPos = KDockWidget::DockRight ; break;
        case KDockWidget::DockRight : another__dockPos = KDockWidget::DockLeft  ; break;
        case KDockWidget::DockTop   : another__dockPos = KDockWidget::DockBottom; break;
        case KDockWidget::DockBottom: another__dockPos = KDockWidget::DockTop   ; break;
        default: break;
      }
      dock_result = target->manualDock( this, another__dockPos, spliPos, pos, true, tabIndex );
    }
    return dock_result;
  }
  // end check block

  d->blockHasUndockedSignal = true;
  undock();
  d->blockHasUndockedSignal = false;

  if ( !target ){
    move( pos );
    show();
    emit manager->change();
    return this;
  }

//  kdDebug()<<"Looking for  KDockTabGroup"<<endl;
  KDockTabGroup* parentTab = target->parentDockTabGroup();
  if ( parentTab ){
    // add to existing TabGroup
    applyToWidget( parentTab );
    parentTab->insertTab( this, icon() ? *icon() : QPixmap(),
                          tabPageLabel(), tabIndex );

    QWidget *wantTransient=parentTab->transientTo();
    target->setDockWindowTransient(wantTransient,wantTransient);

    setDockTabName( parentTab );
    if( !toolTipStr.isEmpty())
      parentTab->setTabToolTip( this, toolTipStr);

    currentDockPos = KDockWidget::DockCenter;
    emit manager->change();
    return (KDockWidget*)parentTab->parent();
  }
  else
  {
//  	kdDebug()<<"Looking for  KDockContainer"<<endl;
  	QWidget *contWid=target->parentDockContainer();
	  if (!contWid) contWid=target->widget;
	  if (contWid)
	  {
	  	KDockContainer *cont=dynamic_cast<KDockContainer*>(contWid);
		  if (cont)
		  {
			if (latestKDockContainer() && (latestKDockContainer()!=contWid))
				dynamic_cast<KDockContainer*>(latestKDockContainer())->removeWidget(this);
//			kdDebug()<<"KDockContainerFound"<<endl;
			applyToWidget( contWid );
			cont->insertWidget( this, icon() ? *icon() : QPixmap(),
						tabPageLabel(), tabIndex );
			setLatestKDockContainer(contWid);
//			setDockTabName( parentTab );
			if( !toolTipStr.isEmpty())
			cont->setToolTip( this, toolTipStr);

			currentDockPos = KDockWidget::DockCenter;
			emit manager->change();
			return (KDockWidget*)(cont->parentDockWidget());

		  }
	  }
  }

  // create a new dockwidget that will contain the target and this
  QWidget* parentDock = target->parentWidget();
  KDockWidget* newDock = new KDockWidget( manager, "tempName", QPixmap(""), parentDock );
  newDock->currentDockPos = target->currentDockPos;

  if ( dockPos == KDockWidget::DockCenter ){
    newDock->isTabGroup = true;
  } else {
    newDock->isGroup = true;
  }
  newDock->eDocking = (target->eDocking & eDocking) & (~(int)KDockWidget::DockCenter);

  newDock->applyToWidget( parentDock );

  if ( !parentDock ){
    // dock to a toplevel dockwidget means newDock is toplevel now
    newDock->move( target->frameGeometry().topLeft() );
    newDock->resize( target->geometry().size() );
    if ( target->isVisibleToTLW() ) newDock->show();
  }

  // redirect the dockback button to the new dockwidget
  if( target->formerBrotherDockWidget != 0L) {
    newDock->formerBrotherDockWidget = target->formerBrotherDockWidget;
    if( formerBrotherDockWidget != 0L)
      QObject::connect( newDock->formerBrotherDockWidget, SIGNAL(iMBeingClosed()),
                        newDock, SLOT(loseFormerBrotherDockWidget()) );
      target->loseFormerBrotherDockWidget();
    }
  newDock->formerDockPos = target->formerDockPos;


 // HERE SOMETING CREATING CONTAINERS SHOULD BE ADDED !!!!!
  if ( dockPos == KDockWidget::DockCenter )
  {
    KDockTabGroup* tab = new KDockTabGroup( newDock, "_dock_tab");
    QObject::connect(tab, SIGNAL(currentChanged(QWidget*)), d, SLOT(slotFocusEmbeddedWidget(QWidget*)));
    newDock->setWidget( tab );

    target->applyToWidget( tab );
    applyToWidget( tab );


    tab->insertTab( target, target->icon() ? *(target->icon()) : QPixmap(),
                    target->tabPageLabel() );



    if( !target->toolTipString().isEmpty())
      tab->setTabToolTip( target, target->toolTipString());

    tab->insertTab( this, icon() ? *icon() : QPixmap(),
                    tabPageLabel(), tabIndex );

    QRect geom=newDock->geometry();
    QWidget *wantTransient=tab->transientTo();
    newDock->setDockWindowTransient(wantTransient,wantTransient);
    newDock->setGeometry(geom);

    if( !toolTipString().isEmpty())
      tab->setTabToolTip( this, toolTipString());

    setDockTabName( tab );
    tab->show();

    currentDockPos = DockCenter;
    target->formerDockPos = target->currentDockPos;
    target->currentDockPos = DockCenter;
  }
  else {
    // if to dock not to the center of the target dockwidget,
    // dock to newDock
    KDockSplitter* panner = 0L;
    if ( dockPos == KDockWidget::DockTop  || dockPos == KDockWidget::DockBottom ) panner = new KDockSplitter( newDock, "_dock_split_", Horizontal, spliPos, manager->splitterHighResolution() );
    if ( dockPos == KDockWidget::DockLeft || dockPos == KDockWidget::DockRight  ) panner = new KDockSplitter( newDock, "_dock_split_", Vertical , spliPos, manager->splitterHighResolution() );
    newDock->setWidget( panner );

    panner->setOpaqueResize(manager->splitterOpaqueResize());
    panner->setKeepSize(manager->splitterKeepSize());
    panner->setFocusPolicy( NoFocus );
    target->applyToWidget( panner );
    applyToWidget( panner );
    target->formerDockPos = target->currentDockPos;
    if ( dockPos == KDockWidget::DockRight) {
      panner->activate( target, this );
      currentDockPos = KDockWidget::DockRight;
      target->currentDockPos = KDockWidget::DockLeft;
    }
    else if( dockPos == KDockWidget::DockBottom) {
      panner->activate( target, this );
      currentDockPos = KDockWidget::DockBottom;
      target->currentDockPos = KDockWidget::DockTop;
    }
    else if( dockPos == KDockWidget::DockTop) {
      panner->activate( this, target );
      currentDockPos = KDockWidget::DockTop;
      target->currentDockPos = KDockWidget::DockBottom;
    }
    else if( dockPos == KDockWidget::DockLeft) {
      panner->activate( this, target );
      currentDockPos = KDockWidget::DockLeft;
      target->currentDockPos = KDockWidget::DockRight;
    }
    target->show();
    show();
    panner->show();
  }

  if ( parentDock ){
    if ( parentDock->inherits("KDockSplitter") ){
      KDockSplitter* sp = (KDockSplitter*)parentDock;
      sp->deactivate();
      if ( sp->getFirst() == target )
        sp->activate( newDock, 0L );
      else
        sp->activate( 0L, newDock );
    }
  }

  newDock->show();
  emit target->docking( this, dockPos );
  emit manager->replaceDock( target, newDock );
  emit manager->change();

  return newDock;
}

KDockTabGroup* KDockWidget::parentDockTabGroup() const
{
  if ( !parent() ) return 0L;
  QWidget* candidate = parentWidget()->parentWidget();
  if ( candidate && candidate->inherits("KDockWidget_Compat::KDockTabGroup") ) return (KDockTabGroup*)candidate;
  return 0L;
}

QWidget *KDockWidget::parentDockContainer() const
{
  if (!parent()) return 0L;
  QWidget* candidate = parentWidget()->parentWidget();
  if (candidate && dynamic_cast<KDockContainer*>(candidate)) return candidate;
  return 0L;
}


void KDockWidget::setForcedFixedWidth(int w)
{
	d->forcedWidth=w;
	setFixedWidth(w);
	if (!parent()) return;
	if (parent()->inherits("KDockSplitter"))
		static_cast<KDockSplitter*>(parent()->qt_cast("KDockSplitter"))->setForcedFixedWidth(this,w);
}

void KDockWidget::setForcedFixedHeight(int h)
{
	d->forcedHeight=h;
	setFixedHeight(h);
	if (!parent()) return;
	if (parent()->inherits("KDockSplitter"))
		static_cast<KDockSplitter*>(parent()->qt_cast("KDockSplitter"))->setForcedFixedHeight(this,h);
}

int KDockWidget::forcedFixedWidth()
{
	return d->forcedWidth;
}

int KDockWidget::forcedFixedHeight()
{
	return d->forcedHeight;
}

void KDockWidget::restoreFromForcedFixedSize()
{
	d->forcedWidth=-1;
	setMinimumWidth(0);
	setMaximumWidth(32000);
	setMinimumHeight(0);
	setMaximumHeight(32000);
	if (!parent()) return;
	if (parent()->inherits("KDockSplitter"))
		static_cast<KDockSplitter*>(parent()->qt_cast("KDockSplitter"))->restoreFromForcedFixedSize(this);
}

void KDockWidget::toDesktop()
{
   QPoint p = mapToGlobal( QPoint( -30, -30 ) );
   if( p.x( ) < 0 )
      p.setX( 0 );
   if( p.y( ) < 0 )
      p.setY( 0 );
   manualDock( 0, DockDesktop, 50, p );
}

void KDockWidget::undock()
{
//  kdDebug()<<"KDockWidget::undock()"<<endl;

  manager->d->dragRect = QRect ();
  manager->drawDragRectangle ();

  QWidget* parentW = parentWidget();
  if ( !parentW ){
    hide();
    if (!d->blockHasUndockedSignal)
      emit hasUndocked();
    return;
  }

  formerDockPos = currentDockPos;
  currentDockPos = KDockWidget::DockDesktop;

  manager->blockSignals(true);
  manager->undockProcess = true;

  bool isV = parentW->isVisibleToTLW();

  //UNDOCK HAS TO BE IMPLEMENTED CORRECTLY :)
  KDockTabGroup* parentTab = parentDockTabGroup();
  if ( parentTab ){
    d->index = parentTab->indexOf( this); // memorize the page position in the tab widget
    parentTab->removePage( this );
/*
    QWidget *wantTransient=parentTab->transientTo();
    target->setDockWindowTransient(wantTransient,wantTransient);
 */
    formerBrotherDockWidget = (KDockWidget*)parentTab->page(0);
    QObject::connect( formerBrotherDockWidget, SIGNAL(iMBeingClosed()),
                      this, SLOT(loseFormerBrotherDockWidget()) );
    applyToWidget( 0L );
    if ( parentTab->count() == 1 ){

      // last subdock widget in the tab control
      KDockWidget* lastTab = (KDockWidget*)parentTab->page(0);
      parentTab->removePage( lastTab );
/*      QWidget *wantTransient=parentTab->transientTo();
      target->setDockWindowTransient(wantTransient,wantTransient);*/

      lastTab->applyToWidget( 0L );
      lastTab->move( parentTab->mapToGlobal(parentTab->frameGeometry().topLeft()) );

      // KDockTabGroup always have a parent that is a KDockWidget
      KDockWidget* parentOfTab = (KDockWidget*)parentTab->parent();
      delete parentTab; // KDockTabGroup

      QWidget* parentOfDockWidget = parentOfTab->parentWidget();
      if ( parentOfDockWidget == 0L ){
          if ( isV ) lastTab->show();
      } else {
        if ( parentOfDockWidget->inherits("KDockSplitter") ){
          KDockSplitter* split = (KDockSplitter*)parentOfDockWidget;
          lastTab->applyToWidget( split );
          split->deactivate();
          if ( split->getFirst() == parentOfTab ){
            split->activate( lastTab );
            if ( ((KDockWidget*)split->parent())->splitterOrientation == Vertical )
              emit ((KDockWidget*)split->getAnother(parentOfTab))->docking( parentOfTab, KDockWidget::DockLeft );
            else
              emit ((KDockWidget*)split->getAnother(parentOfTab))->docking( parentOfTab, KDockWidget::DockTop );
          } else {
            split->activate( 0L, lastTab );
            if ( ((KDockWidget*)split->parent())->splitterOrientation == Vertical )
              emit ((KDockWidget*)split->getAnother(parentOfTab))->docking( parentOfTab, KDockWidget::DockRight );
            else
              emit ((KDockWidget*)split->getAnother(parentOfTab))->docking( parentOfTab, KDockWidget::DockBottom );
          }
          split->show();
        } else {
          lastTab->applyToWidget( parentOfDockWidget );
        }
        lastTab->show();
      }
      manager->blockSignals(false);
      emit manager->replaceDock( parentOfTab, lastTab );
      lastTab->currentDockPos = parentOfTab->currentDockPos;
      emit parentOfTab->iMBeingClosed();
      manager->blockSignals(true);
      delete parentOfTab;

    } else {
      setDockTabName( parentTab );
    }
  } else {
 /*********************************************************************************************/
  //QWidget* containerWidget = (QWidget*)parent();
  bool undockedFromContainer=false;
  if (d->container)
  {
//  		  kdDebug()<<"undocked from dockcontainer"<<endl;
  		  undockedFromContainer=true;
          KDockContainer* dc = dynamic_cast<KDockContainer*>(d->container.operator->());
		  dc->undockWidget(this);
		  formerBrotherDockWidget=dc->parentDockWidget();
		  applyToWidget( 0L );
  }
   if (!undockedFromContainer) {
/*********************************************************************************************/
    if ( parentW->inherits("KDockSplitter") ){
      KDockSplitter* parentSplitterOfDockWidget = (KDockSplitter*)parentW;
      d->splitPosInPercent = parentSplitterOfDockWidget->separatorPos();

      KDockWidget* secondWidget = (KDockWidget*)parentSplitterOfDockWidget->getAnother( this );
      KDockWidget* group        = (KDockWidget*)parentSplitterOfDockWidget->parentWidget();
      formerBrotherDockWidget = secondWidget;
      applyToWidget( 0L );
      group->hide();

      if( formerBrotherDockWidget != 0L)
        QObject::connect( formerBrotherDockWidget, SIGNAL(iMBeingClosed()),
                          this, SLOT(loseFormerBrotherDockWidget()) );

      if ( !group->parentWidget() ){
        secondWidget->applyToWidget( 0L, group->frameGeometry().topLeft() );
        secondWidget->resize( group->width(), group->height() );
      } else {
        QWidget* obj = group->parentWidget();
        secondWidget->applyToWidget( obj );
        if ( obj->inherits("KDockSplitter") ){
          KDockSplitter* parentOfGroup = (KDockSplitter*)obj;
          parentOfGroup->deactivate();

          if ( parentOfGroup->getFirst() == group )
            parentOfGroup->activate( secondWidget );
          else
            parentOfGroup->activate( 0L, secondWidget );
        }
      }
      secondWidget->currentDockPos = group->currentDockPos;
      secondWidget->formerDockPos  = group->formerDockPos;
      delete parentSplitterOfDockWidget;
      manager->blockSignals(false);
      emit manager->replaceDock( group, secondWidget );
      emit group->iMBeingClosed();
      manager->blockSignals(true);
      delete group;

      if ( isV ) secondWidget->show();
    } else {
      if (!d->pendingDtor) {
        // don't reparent in the dtor of this
        applyToWidget( 0L );
      }
    }
/*********************************************************************************************/
  }
  }
  manager->blockSignals(false);
  if (!d->blockHasUndockedSignal)
    emit manager->change();
  manager->undockProcess = false;

  if (!d->blockHasUndockedSignal)
    emit hasUndocked();
}

void KDockWidget::setWidget( QWidget* mw )
{
  if ( !mw ) return;

  if ( mw->parent() != this ){
    mw->reparent(this, 0, QPoint(0,0), false);
  }

#ifdef BORDERLESS_WINDOWS
  if (!mw->ownCursor()) mw->setCursor(QCursor(ArrowCursor));
#endif
  widget = mw;
  delete layout;

  layout = new QVBoxLayout( this );
  layout->setResizeMode( QLayout::Minimum );

  if (dynamic_cast<KDockContainer*>(widget))
  {
    d->isContainer=true;
    manager->d->containerDocks.append(this);
  }
  else
  {
  	d->isContainer=false;
  }

  {
     header->show();
     layout->addWidget( header );
     layout->addWidget( widget,1 );
  }
  updateHeader();
}

void KDockWidget::setDockTabName( KDockTabGroup* tab )
{
  QString listOfName;
  QString listOfCaption;
  for ( int i = 0; i < tab->count(); ++i ) {
    QWidget *w = tab->page( i );
    listOfCaption.append( w->caption() ).append(",");
    listOfName.append( w->name() ).append(",");
  }
  listOfCaption.remove( listOfCaption.length()-1, 1 );
  listOfName.remove( listOfName.length()-1, 1 );

  tab->parentWidget()->setName( listOfName.utf8() );
  tab->parentWidget()->setCaption( listOfCaption );

  tab->parentWidget()->repaint( false ); // KDockWidget->repaint
  if ( tab->parentWidget()->parent() )
    if ( tab->parentWidget()->parent()->inherits("KDockSplitter") )
      ((KDockSplitter*)(tab->parentWidget()->parent()))->updateName();
}

bool KDockWidget::mayBeHide() const
{
  bool f = (parent() != manager->main);
  return ( !isGroup && !isTabGroup && f && isVisible() && ( eDocking != (int)KDockWidget::DockNone ) );
}

bool KDockWidget::mayBeShow() const
{
  bool f = (parent() != manager->main);
  return ( !isGroup && !isTabGroup && f && !isVisible() );
}

void KDockWidget::changeHideShowState()
{
  if ( mayBeHide() ){
    undock();
    return;
  }

  if ( mayBeShow() ){
    if ( manager->main->inherits("KDockWidget_Compat::KDockMainWindow") || manager->main->inherits("KDockMainWindow")){
      ((KDockMainWindow*)manager->main)->makeDockVisible(this);
    } else {
      makeDockVisible();
    }
  }
}

void KDockWidget::makeDockVisible()
{
  if ( parentDockTabGroup() ){
    parentDockTabGroup()->showPage( this );
  }
  if ( isVisible() ) return;

  QWidget* p = parentWidget();
  while ( p ){
    if ( !p->isVisible() )
      p->show();
    p = p->parentWidget();
  }
  if( parent() == 0L) // is undocked
    dockBack();
  show();
}

void KDockWidget::loseFormerBrotherDockWidget()
{
  if( formerBrotherDockWidget != 0L)
    QObject::disconnect( formerBrotherDockWidget, SIGNAL(iMBeingClosed()),
                         this, SLOT(loseFormerBrotherDockWidget()) );
  formerBrotherDockWidget = 0L;
  repaint();
}

void KDockWidget::dockBack()
{
  if( formerBrotherDockWidget) {
    // search all children if it tries to dock back to a child
    bool found = false;
    QObjectList* cl = queryList("KDockWidget_Compat::KDockWidget");
    QObjectListIt it( *cl );
    QObject * obj;
    while ( !found && (obj=it.current()) != 0 ) {
      ++it;
      QWidget* widg = (QWidget*)obj;
      if( widg == formerBrotherDockWidget)
        found = true;
    }
    delete cl;

    if( !found) {
      // can dock back to the former brother dockwidget
      manualDock( formerBrotherDockWidget, formerDockPos, d->splitPosInPercent, QPoint(0,0), false, d->index);
      formerBrotherDockWidget = 0L;
      makeDockVisible();
      return;
    }
  }

  // else dockback to the dockmainwindow (default behaviour)
  manualDock( ((KDockMainWindow*)manager->main)->getMainDockWidget(), formerDockPos, d->splitPosInPercent, QPoint(0,0), false, d->index);
  formerBrotherDockWidget = 0L;
  if (parent())
    makeDockVisible();
}

bool KDockWidget::isDockBackPossible() const
{
  if( (formerBrotherDockWidget == 0L) || !(formerBrotherDockWidget->dockSite() & formerDockPos))
    return false;
  else
    return true;
}

/**************************************************************************************/


KDockManager::KDockManager( QWidget* mainWindow , const char* name )
:QObject( mainWindow, name )
  ,main(mainWindow)
  ,currentDragWidget(0L)
  ,currentMoveWidget(0L)
  ,childDockWidgetList(0L)
  ,autoCreateDock(0L)
  ,storeW(0)
  ,storeH(0)
  ,draging(false)
  ,undockProcess(false)
  ,dropCancel(true)
{
  d = new KDockManagerPrivate;
  d->mainDockWidget=0;
  d->splitterOpaqueResize = false;
  d->splitterKeepSize = false;
  d->splitterHighResolution = false;

  main->installEventFilter( this );

  undockProcess = false;

  menuData = new QPtrList<MenuDockData>;
  menuData->setAutoDelete( true );
  menuData->setAutoDelete( true );

#ifndef NO_KDE2
  menu = new KPopupMenu();
#else
  menu = new QPopupMenu();
#endif

  connect( menu, SIGNAL(aboutToShow()), SLOT(slotMenuPopup()) );
  connect( menu, SIGNAL(activated(int)), SLOT(slotMenuActivated(int)) );

  childDock = new QObjectList();
  childDock->setAutoDelete( false );
}


void KDockManager::setMainDockWidget2(KDockWidget *w)
{
  d->mainDockWidget=w;
}

KDockManager::~KDockManager()
{
  delete menuData;
  delete menu;

  QObjectListIt it( *childDock );
  KDockWidget * obj;

  while ( (obj=(KDockWidget*)it.current()) ) {
    delete obj;
  }
  delete childDock;
  delete d;
  d=0;
}

void KDockManager::activate()
{
  QObjectListIt it( *childDock );
  KDockWidget * obj;

  while ( (obj=(KDockWidget*)it.current()) ) {
    ++it;
    if ( obj->widget ) obj->widget->show();
    if ( !obj->parentDockTabGroup() ){
        obj->show();
    }
  }
  if ( !main->inherits("QDialog") ) main->show();
}

bool KDockManager::eventFilter( QObject *obj, QEvent *event )
{

  if ( obj->inherits("KDockWidget_Compat::KDockWidgetAbstractHeaderDrag") ){
    KDockWidget* pDockWdgAtCursor = 0L;
    KDockWidget* curdw = ((KDockWidgetAbstractHeaderDrag*)obj)->dockWidget();
    switch ( event->type() ){
      case QEvent::MouseButtonDblClick:
        if (curdw->currentDockPos == KDockWidget::DockDesktop)  curdw->dockBack();
        else
	{
		curdw->toDesktop();
		// curdw->manualDock (0, KDockWidget::DockDesktop);
	}
        break;

      case QEvent::MouseButtonPress:
        if ( ((QMouseEvent*)event)->button() == LeftButton ){
          if ( curdw->eDocking != (int)KDockWidget::DockNone ){
            dropCancel = true;
            curdw->setFocus();
            qApp->processOneEvent();

            currentDragWidget = curdw;
            currentMoveWidget = 0L;
            childDockWidgetList = new QWidgetList();
            childDockWidgetList->append( curdw );
            findChildDockWidget( curdw, childDockWidgetList );

            //d->oldDragRect = QRect(); should fix rectangle not erased problem
            d->dragRect = QRect(curdw->geometry());
            QPoint p = curdw->mapToGlobal(QPoint(0,0));
            d->dragRect.moveTopLeft(p);
            drawDragRectangle();
            d->readyToDrag = true;

            d->dragOffset = QCursor::pos()-currentDragWidget->mapToGlobal(QPoint(0,0));
          }

        }
        break;
      case QEvent::MouseButtonRelease:
        if ( ((QMouseEvent*)event)->button() == LeftButton ){
          if ( draging ){
            if ( !dropCancel )
              drop();
            else
              cancelDrop();
          }
          if (d->readyToDrag) {
              d->readyToDrag = false;
              //d->oldDragRect = QRect(); should fix rectangle not erased problem
              d->dragRect = QRect(curdw->geometry());
              QPoint p = curdw->mapToGlobal(QPoint(0,0));
              d->dragRect.moveTopLeft(p);
              drawDragRectangle();
              currentDragWidget = 0L;
              delete childDockWidgetList;
              childDockWidgetList = 0L;
          }
          draging = false;
          dropCancel = true;
        }
        break;
      case QEvent::MouseMove:
        if ( draging ) {

#ifdef BORDERLESS_WINDOWS
//BEGIN TEST
	  KDockWidget *oldMoveWidget;
	  if (curdw->parent()==0)
	  {
	  	curdw->move(QCursor::pos()-d->dragOffset);
   	        pDockWdgAtCursor = findDockWidgetAt( QCursor::pos()-QPoint(0,d->dragOffset.y()+3) );
                oldMoveWidget = currentMoveWidget;
	  }
	  else
	  {
	        pDockWdgAtCursor = findDockWidgetAt( QCursor::pos() );
                oldMoveWidget = currentMoveWidget;
	  }
//END TEST
#else
	  pDockWdgAtCursor = findDockWidgetAt( QCursor::pos() );
          KDockWidget* oldMoveWidget = currentMoveWidget;
#endif

	  if ( currentMoveWidget  && pDockWdgAtCursor == currentMoveWidget ) { //move
            dragMove( currentMoveWidget, currentMoveWidget->mapFromGlobal( QCursor::pos() ) );
            break;
          } else {
            if (dropCancel && curdw) {
              d->dragRect = QRect(curdw->geometry());
              QPoint p = curdw->mapToGlobal(QPoint(0,0));
              d->dragRect.moveTopLeft(p);
            }else
              d->dragRect = QRect();

            drawDragRectangle();
          }

          if ( !pDockWdgAtCursor && (curdw->eDocking & (int)KDockWidget::DockDesktop) == 0 ){
              // just moving at the desktop
              currentMoveWidget = pDockWdgAtCursor;
              curPos = KDockWidget::DockDesktop;
          } else {
            if ( oldMoveWidget && pDockWdgAtCursor != currentMoveWidget ) { //leave
              currentMoveWidget = pDockWdgAtCursor;
              curPos = KDockWidget::DockDesktop;
            }
          }

          if ( oldMoveWidget != pDockWdgAtCursor && pDockWdgAtCursor ) { //enter pDockWdgAtCursor
            currentMoveWidget = pDockWdgAtCursor;
            curPos = KDockWidget::DockDesktop;
          }
        } else {
          if (d->readyToDrag) {
            d->readyToDrag = false;
          }
          if ( (((QMouseEvent*)event)->state() == LeftButton) &&
               (curdw->eDocking != (int)KDockWidget::DockNone) ) {
            startDrag( curdw);
          }
        }
	break;
      default:
        break;
    }
  }
  return QObject::eventFilter( obj, event );
}

KDockWidget* KDockManager::findDockWidgetAt( const QPoint& pos )
{
  dropCancel = true;

  if (!currentDragWidget)
    return 0L; // pointer access safety

  if (currentDragWidget->eDocking == (int)KDockWidget::DockNone ) return 0L;

  QWidget* p = QApplication::widgetAt( pos );
  if ( !p ) {
    dropCancel = false;
    return 0L;
  }
#if defined(_OS_WIN32_) || defined(Q_OS_WIN32)
  p = p->topLevelWidget();
#endif
  QWidget* w = 0L;
  findChildDockWidget( w, p, p->mapFromGlobal(pos) );
  if ( !w ){
    if ( !p->inherits("KDockWidget_Compat::KDockWidget") ) {
      return 0L;
    }
    w = p;
  }
  if ( qt_find_obj_child( w, "KDockSplitter", "_dock_split_" ) ) return 0L;
  if ( qt_find_obj_child( w, "KDockWidget_Compat::KDockTabGroup", "_dock_tab" ) ) return 0L;
  if (dynamic_cast<KDockContainer*>(w)) return 0L;

  if (!childDockWidgetList) return 0L;
  if ( childDockWidgetList->find(w) != -1 ) return 0L;
  if ( currentDragWidget->isGroup && ((KDockWidget*)w)->parentDockTabGroup() ) return 0L;

  KDockWidget* www = (KDockWidget*)w;
  if ( www->sDocking == (int)KDockWidget::DockNone ) return 0L;

  KDockWidget::DockPosition curPos = KDockWidget::DockDesktop;
  QPoint cpos  = www->mapFromGlobal( pos );

  int ww = www->widget->width() / 3;
  int hh = www->widget->height() / 3;

  if ( cpos.y() <= hh ){
    curPos = KDockWidget::DockTop;
  } else
    if ( cpos.y() >= 2*hh ){
      curPos = KDockWidget::DockBottom;
    } else
      if ( cpos.x() <= ww ){
        curPos = KDockWidget::DockLeft;
      } else
        if ( cpos.x() >= 2*ww ){
          curPos = KDockWidget::DockRight;
        } else
            curPos = KDockWidget::DockCenter;

  if ( !(www->sDocking & (int)curPos) ) return 0L;
  if ( !(currentDragWidget->eDocking & (int)curPos) ) return 0L;
  if ( www->manager != this ) return 0L;

  dropCancel = false;
  return www;
}

void KDockManager::findChildDockWidget( QWidget*& ww, const QWidget* p, const QPoint& pos )
{
  if ( p->children() ) {
    QWidget *w;
    QObjectListIt it( *p->children() );
    it.toLast();
    while ( it.current() ) {
      if ( it.current()->isWidgetType() ) {
        w = (QWidget*)it.current();
        if ( w->isVisible() && w->geometry().contains(pos) ) {
          if ( w->inherits("KDockWidget_Compat::KDockWidget") ) ww = w;
          findChildDockWidget( ww, w, w->mapFromParent(pos) );
          return;
        }
      }
      --it;
    }
  }
  return;
}

void KDockManager::findChildDockWidget( const QWidget* p, QWidgetList*& list )
{
  if ( p->children() ) {
    QWidget *w;
    QObjectListIt it( *p->children() );
    it.toLast();
    while ( it.current() ) {
      if ( it.current()->isWidgetType() ) {
        w = (QWidget*)it.current();
        if ( w->isVisible() ) {
          if ( w->inherits("KDockWidget_Compat::KDockWidget") ) list->append( w );
          findChildDockWidget( w, list );
        }
      }
      --it;
    }
  }
  return;
}

void KDockManager::startDrag( KDockWidget* w )
{
  if(( w->currentDockPos == KDockWidget::DockLeft) || ( w->currentDockPos == KDockWidget::DockRight)
   || ( w->currentDockPos == KDockWidget::DockTop) || ( w->currentDockPos == KDockWidget::DockBottom)) {
    w->prevSideDockPosBeforeDrag = w->currentDockPos;

    if ( w->parentWidget()->inherits("KDockSplitter") ){
      KDockSplitter* parentSplitterOfDockWidget = (KDockSplitter*)(w->parentWidget());
      w->d->splitPosInPercent = parentSplitterOfDockWidget->separatorPos();
    }
  }

  curPos = KDockWidget::DockDesktop;
  draging = true;

  QApplication::setOverrideCursor(QCursor(sizeAllCursor));
}

void KDockManager::dragMove( KDockWidget* dw, QPoint pos )
{
  QPoint p = dw->mapToGlobal( dw->widget->pos() );
  KDockWidget::DockPosition oldPos = curPos;

  QSize r = dw->widget->size();
  if ( dw->parentDockTabGroup() ){
    curPos = KDockWidget::DockCenter;
    if ( oldPos != curPos ) {
      d->dragRect.setRect( p.x()+2, p.y()+2, r.width()-4, r.height()-4 );
    }
    return;
  }

  int w = r.width() / 3;
  int h = r.height() / 3;

  if ( pos.y() <= h ){
    curPos = KDockWidget::DockTop;
    w = r.width();
  } else
    if ( pos.y() >= 2*h ){
      curPos = KDockWidget::DockBottom;
      p.setY( p.y() + 2*h );
      w = r.width();
    } else
      if ( pos.x() <= w ){
        curPos = KDockWidget::DockLeft;
        h = r.height();
      } else
        if ( pos.x() >= 2*w ){
          curPos = KDockWidget::DockRight;
          p.setX( p.x() + 2*w );
          h = r.height();
        } else
          {
            curPos = KDockWidget::DockCenter;
            p.setX( p.x() + w );
            p.setY( p.y() + h );
          }

  if ( oldPos != curPos ) {
    d->dragRect.setRect( p.x(), p.y(), w, h );
    drawDragRectangle();
  }
}


void KDockManager::cancelDrop()
{
  QApplication::restoreOverrideCursor();

  delete childDockWidgetList;
  childDockWidgetList = 0L;

  d->dragRect = QRect();  // cancel drawing
  drawDragRectangle();    // only the old rect will be deleted
}


void KDockManager::drop()
{
  d->dragRect = QRect();  // cancel drawing
  drawDragRectangle();    // only the old rect will be deleted

  QApplication::restoreOverrideCursor();

  delete childDockWidgetList;
  childDockWidgetList = 0L;

  if ( dropCancel ) return;
  if ( !currentMoveWidget && ((currentDragWidget->eDocking & (int)KDockWidget::DockDesktop) == 0) ) {
    d->dragRect = QRect();  // cancel drawing
    drawDragRectangle();    // only the old rect will be deleted
    return;
  }
  if ( !currentMoveWidget && !currentDragWidget->parent() ) {
    currentDragWidget->move( QCursor::pos() - d->dragOffset );
  }
  else {
    int splitPos = currentDragWidget->d->splitPosInPercent;
    // do we have to calculate 100%-splitPosInPercent?
    if( (curPos != currentDragWidget->prevSideDockPosBeforeDrag) && (curPos != KDockWidget::DockCenter) && (curPos != KDockWidget::DockDesktop)) {
      switch( currentDragWidget->prevSideDockPosBeforeDrag) {
      case KDockWidget::DockLeft:   if(curPos != KDockWidget::DockTop)    splitPos = 100-splitPos; break;
      case KDockWidget::DockRight:  if(curPos != KDockWidget::DockBottom) splitPos = 100-splitPos; break;
      case KDockWidget::DockTop:    if(curPos != KDockWidget::DockLeft)   splitPos = 100-splitPos; break;
      case KDockWidget::DockBottom: if(curPos != KDockWidget::DockRight)  splitPos = 100-splitPos; break;
      default: break;
      }
    }
    currentDragWidget->manualDock( currentMoveWidget, curPos , splitPos, QCursor::pos() - d->dragOffset );
    currentDragWidget->makeDockVisible();
  }
}


static QDomElement createStringEntry(QDomDocument &doc, const QString &tagName, const QString &str)
{
    QDomElement el = doc.createElement(tagName);

    el.appendChild(doc.createTextNode(str));
    return el;
}


static QDomElement createBoolEntry(QDomDocument &doc, const QString &tagName, bool b)
{
    return createStringEntry(doc, tagName, QString::fromLatin1(b? "true" : "false"));
}


static QDomElement createNumberEntry(QDomDocument &doc, const QString &tagName, int n)
{
    return createStringEntry(doc, tagName, QString::number(n));
}


static QDomElement createRectEntry(QDomDocument &doc, const QString &tagName, const QRect &rect)
{
    QDomElement el = doc.createElement(tagName);

    QDomElement xel = doc.createElement("x");
    xel.appendChild(doc.createTextNode(QString::number(rect.x())));
    el.appendChild(xel);
    QDomElement yel = doc.createElement("y");
    yel.appendChild(doc.createTextNode(QString::number(rect.y())));
    el.appendChild(yel);
    QDomElement wel = doc.createElement("width");
    wel.appendChild(doc.createTextNode(QString::number(rect.width())));
    el.appendChild(wel);
    QDomElement hel = doc.createElement("height");
    hel.appendChild(doc.createTextNode(QString::number(rect.height())));
    el.appendChild(hel);

    return el;
}


static QDomElement createListEntry(QDomDocument &doc, const QString &tagName,
                                   const QString &subTagName, const QStrList &list)
{
    QDomElement el = doc.createElement(tagName);

    QStrListIterator it(list);
    for (; it.current(); ++it) {
        QDomElement subel = doc.createElement(subTagName);
        subel.appendChild(doc.createTextNode(QString::fromLatin1(it.current())));
        el.appendChild(subel);
    }

    return el;
}


static QString stringEntry(QDomElement &base, const QString &tagName)
{
    return base.namedItem(tagName).firstChild().toText().data();
}


static bool boolEntry(QDomElement &base, const QString &tagName)
{
    return base.namedItem(tagName).firstChild().toText().data() == "true";
}


static int numberEntry(QDomElement &base, const QString &tagName)
{
    return stringEntry(base, tagName).toInt();
}


static QRect rectEntry(QDomElement &base, const QString &tagName)
{
    QDomElement el = base.namedItem(tagName).toElement();

    int x = numberEntry(el, "x");
    int y = numberEntry(el, "y");
    int width = numberEntry(el, "width");
    int height = numberEntry(el,  "height");

    return QRect(x, y, width, height);
}


static QStrList listEntry(QDomElement &base, const QString &tagName, const QString &subTagName)
{
    QStrList list;

    QDomElement subel = base.namedItem(tagName).firstChild().toElement();
    while (!subel.isNull()) {
        if (subel.tagName() == subTagName)
            list.append(subel.firstChild().toText().data().latin1());
        subel = subel.nextSibling().toElement();
    }

    return list;
}


void KDockManager::writeConfig(QDomElement &base)
{
    // First of all, clear the tree under base
    while (!base.firstChild().isNull())
        base.removeChild(base.firstChild());
    QDomDocument doc = base.ownerDocument();

    QStrList nameList;
    QString mainWidgetStr;

    // collect widget names
    QStrList nList;
    QObjectListIt it(*childDock);
    KDockWidget *obj1;
    while ( (obj1=(KDockWidget*)it.current()) ) {
        if ( obj1->parent() == main )
            mainWidgetStr = QString::fromLatin1(obj1->name());
        nList.append(obj1->name());
        ++it;
    }

    nList.first();
    while ( nList.current() ) {
        KDockWidget *obj = getDockWidgetFromName( nList.current() );
        if (obj->isGroup && (nameList.find( obj->firstName.latin1() ) == -1
                             || nameList.find(obj->lastName.latin1()) == -1)) {
            // Skip until children are saved (why?)
            nList.next();
            if ( !nList.current() ) nList.first();
            continue;
        }

        QDomElement groupEl;

        if (obj->isGroup) {
            //// Save a group
            groupEl = doc.createElement("splitGroup");

            groupEl.appendChild(createStringEntry(doc, "firstName", obj->firstName));
            groupEl.appendChild(createStringEntry(doc, "secondName", obj->lastName));
            groupEl.appendChild(createNumberEntry(doc, "orientation", (int)obj->splitterOrientation));
            groupEl.appendChild(createNumberEntry(doc, "separatorPos", ((KDockSplitter*)obj->widget)->separatorPos()));
        } else if (obj->isTabGroup) {
            //// Save a tab group
            groupEl = doc.createElement("tabGroup");

            QStrList list;
            for ( int i = 0; i < ((KDockTabGroup*)obj->widget)->count(); ++i )
                list.append( ((KDockTabGroup*)obj->widget)->page( i )->name() );
            groupEl.appendChild(createListEntry(doc, "tabs", "tab", list));
            groupEl.appendChild(createNumberEntry(doc, "currentTab", ((KDockTabGroup*)obj->widget)->currentPageIndex()));
        } else {
            //// Save an ordinary dock widget
            groupEl = doc.createElement("dock");
        }

        groupEl.appendChild(createStringEntry(doc, "name", QString::fromLatin1(obj->name())));
        groupEl.appendChild(createBoolEntry(doc, "hasParent", obj->parent()));
        if ( !obj->parent() ) {
            groupEl.appendChild(createRectEntry(doc, "geometry", QRect(main->frameGeometry().topLeft(), main->size())));
            groupEl.appendChild(createBoolEntry(doc, "visible", obj->isVisible()));
        }
        if (obj->header && obj->header->inherits("KDockWidget_Compat::KDockWidgetHeader")) {
            KDockWidgetHeader *h = static_cast<KDockWidgetHeader*>(obj->header);
            groupEl.appendChild(createBoolEntry(doc, "dragEnabled", h->dragEnabled()));
        }

        base.appendChild(groupEl);
        nameList.append(obj->name());
        nList.remove();
        nList.first();
    }

    if (main->inherits("KDockWidget_Compat::KDockMainWindow") || main->inherits("KDockMainWindow")) {
        KDockMainWindow *dmain = (KDockMainWindow*)main;
        QString centralWidgetStr = QString(dmain->centralWidget()? dmain->centralWidget()->name() : "");
        base.appendChild(createStringEntry(doc, "centralWidget", centralWidgetStr));
        QString mainDockWidgetStr = QString(dmain->getMainDockWidget()? dmain->getMainDockWidget()->name() : "");
        base.appendChild(createStringEntry(doc, "mainDockWidget", mainDockWidgetStr));
    } else {
        base.appendChild(createStringEntry(doc, "mainWidget", mainWidgetStr));
    }

    base.appendChild(createRectEntry(doc, "geometry", QRect(main->frameGeometry().topLeft(), main->size())));
}


void KDockManager::readConfig(QDomElement &base)
{
    if (base.namedItem("group").isNull()
        && base.namedItem("tabgroup").isNull()
        && base.namedItem("dock").isNull()) {
        activate();
        return;
    }

    autoCreateDock = new QObjectList();
    autoCreateDock->setAutoDelete( true );

    bool isMainVisible = main->isVisible();
    main->hide();

    QObjectListIt it(*childDock);
    KDockWidget *obj1;
    while ( (obj1=(KDockWidget*)it.current()) ) {
        if ( !obj1->isGroup && !obj1->isTabGroup ) {
            if ( obj1->parent() )
                obj1->undock();
            else
                obj1->hide();
        }
        ++it;
    }

    QDomElement childEl = base.firstChild().toElement();
    while (!childEl.isNull() ) {
        KDockWidget *obj = 0;

        if (childEl.tagName() == "splitGroup") {
            // Read a group
            QString name = stringEntry(childEl, "name");
            QString firstName = stringEntry(childEl, "firstName");
            QString secondName = stringEntry(childEl, "secondName");
            int orientation = numberEntry(childEl, "orientation");
            int separatorPos = numberEntry(childEl, "separatorPos");

            KDockWidget *first = getDockWidgetFromName(firstName);
            KDockWidget *second = getDockWidgetFromName(secondName);
            if (first && second) {
                obj = first->manualDock(second,
                                        (orientation == (int)Vertical)? KDockWidget::DockLeft : KDockWidget::DockTop,
                                        separatorPos);
                if (obj)
                    obj->setName(name.latin1());
            }
        } else if (childEl.tagName() == "tabGroup") {
            // Read a tab group
            QString name = stringEntry(childEl, "name");
            QStrList list = listEntry(childEl, "tabs", "tab");

            KDockWidget *d1 = getDockWidgetFromName( list.first() );
            list.next();
            KDockWidget *d2 = getDockWidgetFromName( list.current() );

            KDockWidget *obj = d2->manualDock( d1, KDockWidget::DockCenter );
            if (obj) {
                KDockTabGroup *tab = (KDockTabGroup*)obj->widget;
                list.next();
                while (list.current() && obj) {
                    KDockWidget *tabDock = getDockWidgetFromName(list.current());
                    obj = tabDock->manualDock(d1, KDockWidget::DockCenter);
                    list.next();
                }
                if (obj) {
                    obj->setName(name.latin1());
                    tab->showPage(tab->page(numberEntry(childEl, "currentTab")));
                }
            }
        } else if (childEl.tagName() == "dock") {
            // Read an ordinary dock widget
            obj = getDockWidgetFromName(stringEntry(childEl, "name"));
        }

        if (!boolEntry(childEl, "hasParent")) {
            QRect r = rectEntry(childEl, "geometry");
            obj = getDockWidgetFromName(stringEntry(childEl, "name"));
            obj->applyToWidget(0);
            obj->setGeometry(r);
            if (boolEntry(childEl, "visible"))
                obj->QWidget::show();
        }

        if (obj && obj->header && obj->header->inherits("KDockWidget_Compat::KDockWidgetHeader")) {
            KDockWidgetHeader *h = static_cast<KDockWidgetHeader*>(obj->header);
            h->setDragEnabled(boolEntry(childEl, "dragEnabled"));
        }

        childEl = childEl.nextSibling().toElement();
    }

    if (main->inherits("KDockWidget_Compat::KDockMainWindow") || main->inherits("KDockMainWindow")) {
        KDockMainWindow *dmain = (KDockMainWindow*)main;

        QString mv = stringEntry(base, "centralWidget");
        if (!mv.isEmpty() && getDockWidgetFromName(mv) ) {
            KDockWidget *mvd  = getDockWidgetFromName(mv);
            mvd->applyToWidget(dmain);
            mvd->show();
            dmain->setCentralWidget(mvd);
        }
        QString md = stringEntry(base, "mainDockWidget");
        if (!md.isEmpty() && getDockWidgetFromName(md)) {
            KDockWidget *mvd  = getDockWidgetFromName(md);
            dmain->setMainDockWidget(mvd);
        }
    } else {
        QString mv = stringEntry(base, "mainWidget");
        if (!mv.isEmpty() && getDockWidgetFromName(mv)) {
            KDockWidget *mvd  = getDockWidgetFromName(mv);
            mvd->applyToWidget(main);
            mvd->show();
        }
    }

    QRect mr = rectEntry(base, "geometry");
    main->move(mr.topLeft());
    main->resize(mr.size());
    if (isMainVisible)
        main->show();

    delete autoCreateDock;
    autoCreateDock = 0;
}


#ifndef NO_KDE2
void KDockManager::writeConfig( KConfig* c, QString group )
{
  //debug("BEGIN Write Config");
  if ( !c ) c = KGlobal::config();
  if ( group.isEmpty() ) group = "dock_setting_default";

  c->setGroup( group );
  c->writeEntry( "Version", DOCK_CONFIG_VERSION );

  QStringList nameList;
  QStringList findList;
  QObjectListIt it( *childDock );
  KDockWidget * obj;

  // collect KDockWidget's name
  QStringList nList;
  while ( (obj=(KDockWidget*)it.current()) ) {
    ++it;
    //debug("  +Add subdock %s", obj->name());
    nList.append( obj->name() );
    if ( obj->parent() == main )
      c->writeEntry( "Main:view", obj->name() );
  }

//  kdDebug()<<QString("list size: %1").arg(nList.count())<<endl;
  for (QObjectListIt it(d->containerDocks);it.current();++it)
  {
  	dynamic_cast<KDockContainer*>(((KDockWidget*)it.current())->widget)->prepareSave(nList);
  }
//  kdDebug()<<QString("new list size: %1").arg(nList.count())<<endl;

  QStringList::Iterator nListIt=nList.begin();
  while ( nListIt!=nList.end() ){
    //debug("  -Try to save %s", nList.current());
    obj = getDockWidgetFromName( *nListIt );
    QString cname = obj->name();
    if ( obj->header ){
      obj->header->saveConfig( c );
    }
    if (obj->d->isContainer) dynamic_cast<KDockContainer*>(obj->widget)->save(c);
/*************************************************************************************************/
    if ( obj->isGroup ){
      if ( (findList.find( obj->firstName ) != findList.end()) && (findList.find( obj->lastName ) != findList.end() )){

        c->writeEntry( cname+":type", "GROUP");
        if ( !obj->parent() ){
          c->writeEntry( cname+":parent", "___null___");
          c->writeEntry( cname+":geometry", QRect(obj->frameGeometry().topLeft(), obj->size()) );
          c->writeEntry( cname+":visible", obj->isVisible());
        } else {
          c->writeEntry( cname+":parent", "yes");
        }
        c->writeEntry( cname+":first_name", obj->firstName );
        c->writeEntry( cname+":last_name", obj->lastName );
        c->writeEntry( cname+":orientation", (int)obj->splitterOrientation );
        c->writeEntry( cname+":sepPos", ((KDockSplitter*)obj->widget)->separatorPos() );

        nameList.append( obj->name() );
        findList.append( obj->name() );
        //debug("  Save %s", nList.current());
        nList.remove(nListIt);
        nListIt=nList.begin(); //nList.first();
      } else {
/*************************************************************************************************/
        //debug("  Skip %s", nList.current());
        //if ( findList.find( obj->firstName ) == -1 )
        //  debug("  ? Not found %s", obj->firstName);
        //if ( findList.find( obj->lastName ) == -1 )
        //  debug("  ? Not found %s", obj->lastName);
        ++nListIt;
        // if ( !nList.current() ) nList.first();
	if (nListIt==nList.end()) nListIt=nList.begin();
      }
    } else {
/*************************************************************************************************/
      if ( obj->isTabGroup){
        c->writeEntry( cname+":type", "TAB_GROUP");
        if ( !obj->parent() ){
          c->writeEntry( cname+":parent", "___null___");
          c->writeEntry( cname+":geometry", QRect(obj->frameGeometry().topLeft(), obj->size()) );
          c->writeEntry( cname+":visible", obj->isVisible());
        } else {
          c->writeEntry( cname+":parent", "yes");
        }
        QStrList list;
        for ( int i = 0; i < ((KDockTabGroup*)obj->widget)->count(); ++i )
          list.append( ((KDockTabGroup*)obj->widget)->page( i )->name() );
        c->writeEntry( cname+":tabNames", list );
        c->writeEntry( cname+":curTab", ((KDockTabGroup*)obj->widget)->currentPageIndex() );

        nameList.append( obj->name() );
        findList.append( obj->name() ); // not realy need !!!
        //debug("  Save %s", nList.current());
        nList.remove(nListIt);
        nListIt=nList.begin();
      } else {
/*************************************************************************************************/
        if ( !obj->parent() ){
          c->writeEntry( cname+":type", "NULL_DOCK");
          c->writeEntry( cname+":geometry", QRect(obj->frameGeometry().topLeft(), obj->size()) );
          c->writeEntry( cname+":visible", obj->isVisible());
        } else {
          c->writeEntry( cname+":type", "DOCK");
        }
        nameList.append( cname.latin1() );
        //debug("  Save %s", nList.current());
        findList.append( obj->name() );
        nList.remove(nListIt);
        nListIt=nList.begin();
      }
    }
  }
  c->writeEntry( "NameList", nameList );

  c->writeEntry( "Main:Geometry", QRect(main->frameGeometry().topLeft(), main->size()) );
  c->writeEntry( "Main:visible", main->isVisible()); // curently nou use

  if ( main->inherits("KDockWidget_Compat::KDockMainWindow") || main->inherits("KDockMainWindow") ){
    KDockMainWindow* dmain = (KDockMainWindow*)main;
    // for KDockMainWindow->setView() in readConfig()
    c->writeEntry( "Main:view", dmain->centralWidget() ? dmain->centralWidget()->name():"" );
    c->writeEntry( "Main:dock", dmain->getMainDockWidget()     ? dmain->getMainDockWidget()->name()    :"" );
  }

  c->sync();
  //debug("END Write Config");
}
#include <qmessagebox.h>
void KDockManager::readConfig( KConfig* c, QString group )
{
  if ( !c ) c = KGlobal::config();
  if ( group.isEmpty() ) group = "dock_setting_default";

  c->setGroup( group );
  QStrList nameList;
  c->readListEntry( "NameList", nameList );
  QString ver = c->readEntry( "Version", "0.0.1" );
  nameList.first();
  if ( !nameList.current() || ver != DOCK_CONFIG_VERSION ){
    activate();
    return;
  }

  autoCreateDock = new QObjectList();
  autoCreateDock->setAutoDelete( true );

  bool isMainVisible = main->isVisible();
 // if (isMainVisible)  // CCC
  //QMessageBox::information(0,"","hallo");
//COMMENTED4TESTING  main->hide();

  QObjectListIt it( *childDock );
  KDockWidget * obj;

  while ( (obj=(KDockWidget*)it.current()) ){
    ++it;
    if ( !obj->isGroup && !obj->isTabGroup )
    {
      if ( obj->parent() ) obj->undock(); else obj->hide();
    }
  }

  nameList.first();
  while ( nameList.current() ){
    QString oname = nameList.current();
    c->setGroup( group );
    QString type = c->readEntry( oname + ":type" );
    obj = 0L;

    if ( type == "GROUP" ){
      KDockWidget* first = getDockWidgetFromName( c->readEntry( oname + ":first_name" ) );
      KDockWidget* last  = getDockWidgetFromName( c->readEntry( oname + ":last_name"  ) );
      int sepPos = c->readNumEntry( oname + ":sepPos" );

      Orientation p = (Orientation)c->readNumEntry( oname + ":orientation" );
      if ( first  && last ){
        obj = first->manualDock( last, ( p == Vertical ) ? KDockWidget::DockLeft : KDockWidget::DockTop, sepPos );
        if (obj){
          obj->setName( oname.latin1() );
        }
      }
    }

    if ( type == "TAB_GROUP" ){
      QStrList list;
      KDockWidget* tabDockGroup = 0L;
      c->readListEntry( oname+":tabNames", list );
      KDockWidget* d1 = getDockWidgetFromName( list.first() );
      list.next();
      KDockWidget* d2 = getDockWidgetFromName( list.current() );
      tabDockGroup = d2->manualDock( d1, KDockWidget::DockCenter );
      if ( tabDockGroup ){
        KDockTabGroup* tab = (KDockTabGroup*)tabDockGroup->widget;
        list.next();
        while ( list.current() && tabDockGroup ){
          KDockWidget* tabDock = getDockWidgetFromName( list.current() );
          tabDockGroup = tabDock->manualDock( d1, KDockWidget::DockCenter );
          list.next();
        }
        if ( tabDockGroup ){
          tabDockGroup->setName( oname.latin1() );
          c->setGroup( group );
          tab->showPage( tab->page( c->readNumEntry( oname+":curTab" ) ) );
        }
      }
      obj = tabDockGroup;
    }

    if ( type == "NULL_DOCK" || c->readEntry( oname + ":parent") == "___null___" ){
      QRect r = c->readRectEntry( oname + ":geometry" );
      obj = getDockWidgetFromName( oname );
      obj->applyToWidget( 0L );
      obj->setGeometry(r);

      c->setGroup( group );
      if ( c->readBoolEntry( oname + ":visible" ) ){
        obj->QWidget::show();
      }
    }

    if ( type == "DOCK"  ){
      obj = getDockWidgetFromName( oname );
    }

    if (obj && obj->d->isContainer)  dynamic_cast<KDockContainer*>(obj->widget)->load(c);
    if ( obj && obj->header){
      obj->header->loadConfig( c );
    }
    nameList.next();
  }

  if ( main->inherits("KDockWidget_Compat::KDockMainWindow") || main->inherits("KDockMainWindow") ){
    KDockMainWindow* dmain = (KDockMainWindow*)main;

    c->setGroup( group );
    QString mv = c->readEntry( "Main:view" );
    if ( !mv.isEmpty() && getDockWidgetFromName( mv ) ){
      KDockWidget* mvd  = getDockWidgetFromName( mv );
      mvd->applyToWidget( dmain );
      mvd->show();
      dmain->setView( mvd );
    }
    c->setGroup( group );
    QString md = c->readEntry( "Main:dock" );
    if ( !md.isEmpty() && getDockWidgetFromName( md ) ){
      KDockWidget* mvd  = getDockWidgetFromName( md );
      dmain->setMainDockWidget( mvd );
    }
  } else {
    c->setGroup( group );
    QString mv = c->readEntry( "Main:view" );
    if ( !mv.isEmpty() && getDockWidgetFromName( mv ) ){
      KDockWidget* mvd  = getDockWidgetFromName( mv );
      mvd->applyToWidget( main );
      mvd->show();
    }

  }
  // delete all autocreate dock
  delete autoCreateDock;
  autoCreateDock = 0L;

  c->setGroup( group );
  QRect mr = c->readRectEntry("Main:Geometry");
  main->move(mr.topLeft());
  main->resize(mr.size());
  if ( isMainVisible ) main->show();
}
#endif

KDockWidget* KDockManager::getDockWidgetFromName( const QString& dockName )
{
  QObjectListIt it( *childDock );
  KDockWidget * obj;
  while ( (obj=(KDockWidget*)it.current()) ) {
    ++it;
    if ( QString(obj->name()) == dockName ) return obj;
  }

  KDockWidget* autoCreate = 0L;
  if ( autoCreateDock ){
    autoCreate = new KDockWidget( this, dockName.latin1(), QPixmap("") );
    autoCreateDock->append( autoCreate );
  }
  return autoCreate;
}
void KDockManager::setSplitterOpaqueResize(bool b)
{
  d->splitterOpaqueResize = b;
}

bool KDockManager::splitterOpaqueResize() const
{
  return d->splitterOpaqueResize;
}

void KDockManager::setSplitterKeepSize(bool b)
{
  d->splitterKeepSize = b;
}

bool KDockManager::splitterKeepSize() const
{
  return d->splitterKeepSize;
}

void KDockManager::setSplitterHighResolution(bool b)
{
  d->splitterHighResolution = b;
}

bool KDockManager::splitterHighResolution() const
{
  return d->splitterHighResolution;
}

void KDockManager::slotMenuPopup()
{
  menu->clear();
  menuData->clear();

  QObjectListIt it( *childDock );
  KDockWidget * obj;
  int numerator = 0;
  while ( (obj=(KDockWidget*)it.current()) ) {
    ++it;
    if ( obj->mayBeHide() )
    {
      menu->insertItem( obj->icon() ? *(obj->icon()) : QPixmap(), i18n("Hide %1").arg(obj->caption()), numerator++ );
      menuData->append( new MenuDockData( obj, true ) );
    }

    if ( obj->mayBeShow() )
    {
      menu->insertItem( obj->icon() ? *(obj->icon()) : QPixmap(), i18n("Show %1").arg(obj->caption()), numerator++ );
      menuData->append( new MenuDockData( obj, false ) );
    }
  }
}

void KDockManager::slotMenuActivated( int id )
{
  MenuDockData* data = menuData->at( id );
  data->dock->changeHideShowState();
}

KDockWidget* KDockManager::findWidgetParentDock( QWidget* w ) const
{
  QObjectListIt it( *childDock );
  KDockWidget * dock;
  KDockWidget * found = 0L;

  while ( (dock=(KDockWidget*)it.current()) ) {
    ++it;
    if ( dock->widget == w ){ found  = dock; break; }
  }
  return found;
}

void KDockManager::drawDragRectangle()
{
  if (d->oldDragRect == d->dragRect)
    return;

  int i;
  QRect oldAndNewDragRect[2];
  oldAndNewDragRect[0] = d->oldDragRect;
  oldAndNewDragRect[1] = d->dragRect;

  // 2 calls, one for the old and one for the new drag rectangle
  for (i = 0; i <= 1; i++) {
    if (oldAndNewDragRect[i].isEmpty())
      continue;

    KDockWidget* pDockWdgAtRect = (KDockWidget*) QApplication::widgetAt( oldAndNewDragRect[i].topLeft(), true );
    if (!pDockWdgAtRect)
      continue;

    bool isOverMainWdg = false;
    bool unclipped;
    KDockMainWindow* pMain = 0L;
    KDockWidget* pTLDockWdg = 0L;
    QWidget* topWdg;
    if (pDockWdgAtRect->topLevelWidget() == main) {
      isOverMainWdg = true;
      topWdg = pMain = (KDockMainWindow*) main;
      unclipped = pMain->testWFlags( WPaintUnclipped );
      pMain->setWFlags( WPaintUnclipped );
    }
    else {
      topWdg = pTLDockWdg = (KDockWidget*) pDockWdgAtRect->topLevelWidget();
      unclipped = pTLDockWdg->testWFlags( WPaintUnclipped );
      pTLDockWdg->setWFlags( WPaintUnclipped );
    }

    // draw the rectangle unclipped over the main dock window
    QPainter p;
    p.begin( topWdg );
      if ( !unclipped ) {
        if (isOverMainWdg)
          pMain->clearWFlags(WPaintUnclipped);
        else
          pTLDockWdg->clearWFlags(WPaintUnclipped);
      }
      // draw the rectangle
      p.setRasterOp(Qt::NotXorROP);
      QRect r = oldAndNewDragRect[i];
      r.moveTopLeft( r.topLeft() - topWdg->mapToGlobal(QPoint(0,0)) );
      p.drawRect(r.x(), r.y(), r.width(), r.height());
    p.end();
  }

  // memorize the current rectangle for later removing
  d->oldDragRect = d->dragRect;
}



KDockArea::KDockArea( QWidget* parent, const char *name)
:QWidget( parent, name)
{
  QString new_name = QString(name) + QString("_DockManager");
  dockManager = new KDockManager( this, new_name.latin1() );
  mainDockWidget = 0L;
}

KDockArea::~KDockArea()
{
	delete dockManager;
}

KDockWidget* KDockArea::createDockWidget( const QString& name, const QPixmap &pixmap, QWidget* parent, const QString& strCaption, const QString& strTabPageLabel)
{
  return new KDockWidget( dockManager, name.latin1(), pixmap, parent, strCaption, strTabPageLabel );
}

void KDockArea::makeDockVisible( KDockWidget* dock )
{
  if ( dock != 0L)
    dock->makeDockVisible();
}

void KDockArea::makeDockInvisible( KDockWidget* dock )
{
  if ( dock != 0L)
    dock->undock();
}

void KDockArea::makeWidgetDockVisible( QWidget* widget )
{
  makeDockVisible( dockManager->findWidgetParentDock(widget) );
}

void KDockArea::writeDockConfig(QDomElement &base)
{
  dockManager->writeConfig(base);
}

void KDockArea::readDockConfig(QDomElement &base)
{
  dockManager->readConfig(base);
}

void KDockArea::slotDockWidgetUndocked()
{
  QObject* pSender = (QObject*) sender();
  if (!pSender->inherits("KDockWidget_Compat::KDockWidget")) return;
  KDockWidget* pDW = (KDockWidget*) pSender;
  emit dockWidgetHasUndocked( pDW);
}

void KDockArea::resizeEvent(QResizeEvent *rsize)
{
  QWidget::resizeEvent(rsize);
  if (children()){
#ifndef NO_KDE2
//    kdDebug()<<"KDockArea::resize"<<endl;
#endif
    QObjectList *list=queryList("QWidget",0,false);

    QObjectListIt it( *list ); // iterate over the buttons
    QObject *obj;

    while ( (obj = it.current()) != 0 ) {
        // for each found object...
        ((QWidget*)obj)->setGeometry(QRect(QPoint(0,0),size()));
	break;
    }
    delete list;
#if 0
    KDockSplitter *split;
//    for (unsigned int i=0;i<children()->count();i++)
    {
//    	QPtrList<QObject> list(children());
//       QObject *obj=((QPtrList<QObject*>)children())->at(i);
	QObject *obj=children()->getFirst();
       if (split=dynamic_cast<KDockSplitter*>(obj))
       {
          split->setGeometry( QRect(QPoint(0,0), size() ));
//	  break;
       }
    }
#endif
   }
}

#ifndef NO_KDE2
void KDockArea::writeDockConfig( KConfig* c, QString group )
{
  dockManager->writeConfig( c, group );
}

void KDockArea::readDockConfig( KConfig* c, QString group )
{
  dockManager->readConfig( c, group );
}

void KDockArea::setMainDockWidget( KDockWidget* mdw )
{
  if ( mainDockWidget == mdw ) return;
  mainDockWidget = mdw;
  mdw->applyToWidget(this);
}
#endif



// KDOCKCONTAINER - AN ABSTRACTION OF THE KDOCKTABWIDGET
KDockContainer::KDockContainer(){m_overlapMode=false; m_childrenListBegin=0; m_childrenListEnd=0;}
KDockContainer::~KDockContainer(){

	if (m_childrenListBegin!=0)
	{
		struct ListItem *tmp=m_childrenListBegin;
		while (tmp)
		{
			struct ListItem *tmp2=tmp->next;
			delete tmp->data;
			delete tmp;
			tmp=tmp2;
		}
		m_childrenListBegin=0;
		m_childrenListEnd=0;
	}

}

void KDockContainer::activateOverlapMode(int nonOverlapSize) {
	m_nonOverlapSize=nonOverlapSize;
	m_overlapMode=true;
	if (parentDockWidget()) {
		if (parentDockWidget()->parent()) {
			KDockSplitter *sp= static_cast<KDockSplitter*>(parentDockWidget()->
				parent()->qt_cast("KDockSplitter"));
			if (sp) sp->resizeEvent(0);
		}
	}
}

void KDockContainer::deactivateOverlapMode() {
	m_overlapMode=false;
	if (parentDockWidget()) {
		if (parentDockWidget()->parent()) {
			KDockSplitter *sp= static_cast<KDockSplitter*>(parentDockWidget()->
				parent()->qt_cast("KDockSplitter"));
			if (sp) sp->resizeEvent(0);
		}
	}

}

bool KDockContainer::isOverlapMode() {
	return m_overlapMode;
}

KDockWidget *KDockContainer::parentDockWidget(){return 0;}

QStringList KDockContainer::containedWidgets() const {
	QStringList tmp;
	for (struct ListItem *it=m_childrenListBegin;it;it=it->next) {
		tmp<<QString(it->data);
	}

	return tmp;
}

void KDockContainer::insertWidget (KDockWidget *dw, QPixmap, const QString &, int &)
	{
		struct ListItem *it=new struct ListItem;
		it->data=strdup(dw->name());
		it->next=0;

		if (m_childrenListEnd)
		{
			m_childrenListEnd->next=it;
			it->prev=m_childrenListEnd;
			m_childrenListEnd=it;
		}
		else
		{
			it->prev=0;
			m_childrenListEnd=it;
			m_childrenListBegin=it;
		}
	}
void KDockContainer::removeWidget (KDockWidget *dw){
	for (struct ListItem *tmp=m_childrenListBegin;tmp;tmp=tmp->next)
	{
		if (!strcmp(tmp->data,dw->name()))
		{
			free(tmp->data);
			if (tmp->next) tmp->next->prev=tmp->prev;
			if (tmp->prev) tmp->prev->next=tmp->next;
			if (tmp==m_childrenListBegin) m_childrenListBegin=tmp->next;
			if (tmp==m_childrenListEnd) m_childrenListEnd=tmp->prev;
			delete tmp;
			break;
		}
	}
}

//m_children.remove(dw->name());}
void KDockContainer::undockWidget (KDockWidget *){;}
void KDockContainer::setToolTip(KDockWidget *, QString &){;}
void KDockContainer::setPixmap(KDockWidget*,const QPixmap&){;}
#ifndef NO_KDE2
void KDockContainer::load (KConfig*){;}
void KDockContainer::save (KConfig*){;}
#endif
void KDockContainer::prepareSave(QStringList &names)
{

	for (struct ListItem *tmp=m_childrenListBegin;tmp; tmp=tmp->next)
		names.remove(tmp->data);
//	for (uint i=0;i<m_children.count();i++)
//	{
//		names.remove(m_children.at(i));
//	}
}


QWidget *KDockTabGroup::transientTo() {
	QWidget *tT=0;
	for (int i=0;i<count();i++) {
		KDockWidget *dw=static_cast<KDockWidget*>(page(i)->qt_cast("KDockWidget_Compat::KDockWidget"));
		QWidget *tmp;
		if ((tmp=dw->transientTo())) {
			if (!tT) tT=tmp;
			else {
				if (tT!=tmp) {
//					kdDebug()<<"KDockTabGroup::transientTo: widget mismatch"<<endl;
					return 0;
				}
			}
		}
	}
  
//	kdDebug()<<"KDockTabGroup::transientTo: "<<((tT!=0)?"YES":"NO")<<endl;

	return tT;
}

void KDockWidgetAbstractHeader::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

void KDockWidgetAbstractHeaderDrag::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

void KDockWidgetHeaderDrag::virtual_hook( int id, void* data )
{ KDockWidgetAbstractHeaderDrag::virtual_hook( id, data ); }

void KDockWidgetHeader::virtual_hook( int id, void* data )
{ KDockWidgetAbstractHeader::virtual_hook( id, data ); }

void KDockTabGroup::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

void KDockWidget::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

void KDockManager::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

void KDockMainWindow::virtual_hook( int id, void* data )
{ KMainWindow::virtual_hook( id, data ); }

void KDockArea::virtual_hook( int, void* )
{ /*KMainWindow::virtual_hook( id, data );*/ }

KDockSplitter::KDockSplitter(QWidget *parent, const char *name, Orientation orient, int pos, bool highResolution)
: QWidget(parent, name)
{
  divider = 0L;
  child0 = 0L;
  child1 = 0L;
  fixedWidth0=-1;
  fixedWidth1=-1;
  fixedHeight0=-1;
  fixedHeight1=-1;
   
  m_orientation = orient;
  mOpaqueResize = false;
  mKeepSize = false;
  mHighResolution = highResolution;
  setSeparatorPos( pos, false );
  initialised = false;
}

void KDockSplitter::activate(QWidget *c0, QWidget *c1)
{
  if ( c0 ) child0 = c0;
  if ( c1 ) child1 = c1;

  setupMinMaxSize();

  if (divider) delete divider;
  divider = new QFrame(this, "pannerdivider");
  divider->setFrameStyle(QFrame::Panel | QFrame::Raised);
  divider->setLineWidth(1);
  divider->raise();

  if (m_orientation == Horizontal)
    divider->setCursor(QCursor(sizeVerCursor));
  else
    divider->setCursor(QCursor(sizeHorCursor));

  divider->installEventFilter(this);

  initialised= true;

  updateName();

  divider->show();
  resizeEvent(0);
  if (fixedWidth0!=-1) restoreFromForcedFixedSize((KDockWidget*)child0);
  if (fixedWidth1!=-1) restoreFromForcedFixedSize((KDockWidget*)child1);
  if (((KDockWidget*)child0)->forcedFixedWidth()!=-1)
  {
  	setForcedFixedWidth(((KDockWidget*)child0),((KDockWidget*)child0)->forcedFixedWidth());
	//QTimer::singleShot(100,this,SLOT(delayedResize()));
  }
  else
  if (((KDockWidget*)child1)->forcedFixedWidth()!=-1)
  {
  	setForcedFixedWidth(((KDockWidget*)child1),((KDockWidget*)child1)->forcedFixedWidth());
	//QTimer::singleShot(100,this,SLOT(delayedResize()));
  }
}

/*
void KDockSplitter::delayedResize()
{
	kdDebug()<<"*********************** DELAYED RESIZE !!!!!!!!!!!!!!!"<<endl;
	resizeEvent(0);	
}*/

void KDockSplitter::setForcedFixedWidth(KDockWidget *dw,int w)
{
	int factor = (mHighResolution)? 10000:100;
	if (dw==child0)
	{
                fixedWidth0=w;
//		setupMinMaxSize();
		savedXPos=xpos;
		setSeparatorPos(w*factor/width(),true);
//		kdDebug()<<"Set forced fixed width for widget 0 :"<<w<<endl;
	}
        else
	{
                fixedWidth1=w;
		savedXPos=xpos;
		setSeparatorPos((width()-w)*factor/width(),true);
//		kdDebug()<<"Set forced fixed width for widget 1 :"<<w<<endl;
//		kdDebug()<<"Width() :"<<width()<<endl;
	}
}

void KDockSplitter::setForcedFixedHeight(KDockWidget *dw,int h)
{
	int factor = (mHighResolution)? 10000:100;
	if (dw==child0)
	{
                fixedHeight0=h;
//		setupMinMaxSize();
		savedXPos=xpos;
		setSeparatorPos(h*factor/height(),true);
//		kdDebug()<<"Set forced fixed width for widget 0 :"<<h<<endl;
	}
        else
	{
                fixedHeight1=h;
		savedXPos=xpos;
		setSeparatorPos((height()-h)*factor/height(),true);
//		kdDebug()<<"Set forced fixed height for widget 1 :"<<h<<endl;
	}
}

void KDockSplitter::restoreFromForcedFixedSize(KDockWidget *dw)
{
	if (dw==child0)
	{
		fixedWidth0=-1;
		fixedHeight0=-1;
		setSeparatorPos(savedXPos,true);
	}
	else
	{
		fixedWidth1=-1;
		fixedHeight1=-1;
		setSeparatorPos(savedXPos,true);
	}
}


void KDockSplitter::setupMinMaxSize()
{
  // Set the minimum and maximum sizes
  int minx, maxx, miny, maxy;
  if (m_orientation == Horizontal) {
    miny = child0->minimumSize().height() + child1->minimumSize().height()+4;
    maxy = child0->maximumSize().height() + child1->maximumSize().height()+4;
    minx = (child0->minimumSize().width() > child1->minimumSize().width()) ? child0->minimumSize().width() : child1->minimumSize().width();
    maxx = (child0->maximumSize().width() > child1->maximumSize().width()) ? child0->maximumSize().width() : child1->maximumSize().width();

    miny = (miny > 4) ? miny : 4;
    maxy = (maxy < 32000) ? maxy : 32000;
    minx = (minx > 2) ? minx : 2;
    maxx = (maxx < 32000) ? maxx : 32000;
  } else {
    minx = child0->minimumSize().width() + child1->minimumSize().width()+4;
    maxx = child0->maximumSize().width() + child1->maximumSize().width()+4;
    miny = (child0->minimumSize().height() > child1->minimumSize().height()) ? child0->minimumSize().height() : child1->minimumSize().height();
    maxy = (child0->maximumSize().height() > child1->maximumSize().height()) ? child0->maximumSize().height() : child1->maximumSize().height();

    minx = (minx > 4) ? minx : 4;
    maxx = (maxx < 32000) ? maxx : 32000;
    miny = (miny > 2) ? miny : 2;
    maxy = (maxy < 32000) ? maxy : 32000;
  }
  setMinimumSize(minx, miny);
  setMaximumSize(maxx, maxy);
}

void KDockSplitter::deactivate()
{
  delete divider;
  divider = 0L;
  initialised= false;
}

void KDockSplitter::setSeparatorPos(int pos, bool do_resize)
{
  xpos = pos;
  if (do_resize)
    resizeEvent(0);
}

int KDockSplitter::separatorPos() const
{
  return xpos;
}

void KDockSplitter::resizeEvent(QResizeEvent *ev)
{
//  kdDebug()<<"ResizeEvent :"<< ((initialised) ? "initialised":"not initialised")<<", "<< ((ev) ? "real event":"")<<
//	", "<<(isVisible() ?"visible":"")<<endl;
  if (initialised){
    int factor = (mHighResolution)? 10000:100;
    // real resize event, recalculate xpos
    if (ev && mKeepSize && isVisible()) {
//	kdDebug()<<"mKeepSize : "<< ((m_orientation == Horizontal) ? "Horizontal":"Vertical") <<endl;

      if (ev->oldSize().width() != ev->size().width())
      {
          if (m_orientation == Horizontal) {
          xpos = factor * checkValue( child0->height()+1 ) / height();
          } else {
          xpos = factor * checkValue( child0->width()+1 ) / width();
	  }
      
          }
      }
          else
          {
//	kdDebug()<<"!mKeepSize : "<< ((m_orientation == Horizontal) ? "Horizontal":"Vertical") <<endl;
	if (ev && isVisible()) {
		if (m_orientation == Horizontal) {
			if (ev->oldSize().height() != ev->size().height())
			{
			  if (fixedHeight0!=-1)
				xpos=fixedHeight0*factor/height();
			  else
			  if (fixedHeight1!=-1)
				xpos=(height()-fixedHeight1)*factor/height();
			}
		}
		else
		{
	        	if (ev->oldSize().width() != ev->size().width())
			{
			  if (fixedWidth0!=-1)
				xpos=fixedWidth0*factor/width();
			  else
			  if (fixedWidth1!=-1)
				xpos=(width()-fixedWidth1)*factor/width();
			}
		}
	}
//	else kdDebug()<<"Something else happened"<<endl;
   }

    KDockContainer *dc;
    KDockWidget *c0=(KDockWidget*)child0;
    KDockWidget *c1=(KDockWidget*)child1;
    bool stdHandling=false;
    if ((fixedWidth0==-1) && (fixedWidth1==-1)) {
	    if ((c0->getWidget()) && (dc=dynamic_cast<KDockContainer*>(c0->getWidget()))
		 && (dc->m_overlapMode)) {
			int position= (m_orientation == Vertical ? width() : height()) * xpos/factor;
			position=checkValueOverlapped(position,child0);
			child0->raise();
			divider->raise();
	        	      if (m_orientation == Horizontal){
        	        	child0->setGeometry(0, 0, width(), position);
	                	child1->setGeometry(0, dc->m_nonOverlapSize+4, width(), 
						height()-dc->m_nonOverlapSize-4);
	        	        divider->setGeometry(0, position, width(), 4);
	        	      } else {
        	        	child0->setGeometry(0, 0, position, height());
		                child1->setGeometry(dc->m_nonOverlapSize+4, 0, 
						width()-dc->m_nonOverlapSize-4, height());
        		        divider->setGeometry(position, 0, 4, height());
		              }
	    } else {
		 if ((c1->getWidget()) && (dc=dynamic_cast<KDockContainer*>(c1->getWidget()))
        	 && (dc->m_overlapMode)) {
                	int position= (m_orientation == Vertical ? width() : height()) * xpos/factor;
			position=checkValueOverlapped(position,child1);
	                child1->raise();
        	        divider->raise();
	                      if (m_orientation == Horizontal){
        	                child0->setGeometry(0, 0, width(), height()-dc->m_nonOverlapSize-4);
                	        child1->setGeometry(0, position+4, width(),
	                                        height()-position-4);
        	                divider->setGeometry(0, position, width(), 4);
                	      } else {
                        	child0->setGeometry(0, 0, width()-dc->m_nonOverlapSize-4, height());
	                        child1->setGeometry(position+4, 0,
        	                                width()-position-4, height());
                	        divider->setGeometry(position, 0, 4, height());
	                      }
		}
		else stdHandling=true;
	      }
            }
	 else stdHandling=true;

	if (stdHandling) {
		      int position = checkValue( (m_orientation == Vertical ? width() : height()) * xpos/factor );
		      if (m_orientation == Horizontal){
        		child0->setGeometry(0, 0, width(), position);
		        child1->setGeometry(0, position+4, width(), height()-position-4);
        		divider->setGeometry(0, position, width(), 4);
		      } else {
        		child0->setGeometry(0, 0, position, height());
	        	child1->setGeometry(position+4, 0, width()-position-4, height());
	        	divider->setGeometry(position, 0, 4, height());
	}

	}
	
  }
}

int KDockSplitter::checkValueOverlapped(int position, QWidget *overlappingWidget) const {
	if (initialised) {
		if (m_orientation == Vertical) {
			if (child0==overlappingWidget) {
				if (position<(child0->minimumSize().width()))
					position=child0->minimumSize().width();
				if (position>width()) position=width()-4;
			} else if (position>(width()-(child1->minimumSize().width())-4)){
				position=width()-(child1->minimumSize().width())-4;
				if (position<0) position=0;
			}
		} else {// orientation  == Horizontal
			if (child0==overlappingWidget) {
				if (position<(child0->minimumSize().height()))
					position=child0->minimumSize().height();
				if (position>height()) position=height()-4;
			} else if (position>(height()-(child1->minimumSize().height())-4)){
				position=height()-(child1->minimumSize().height())-4;
				if (position<0) position=0;

			}
		}

	}
	return position;
}

int KDockSplitter::checkValue( int position ) const
{
  if (initialised){
    if (m_orientation == Vertical){
      if (position < (child0->minimumSize().width()))
        position = child0->minimumSize().width();
      if ((width()-4-position) < (child1->minimumSize().width()))
        position = width() - (child1->minimumSize().width()) -4;
    } else {
      if (position < (child0->minimumSize().height()))
        position = (child0->minimumSize().height());
      if ((height()-4-position) < (child1->minimumSize().height()))
        position = height() - (child1->minimumSize().height()) -4;
    }
  }

  if (position < 0) position = 0;

  if ((m_orientation == Vertical) && (position > width()))
    position = width();
  if ((m_orientation == Horizontal) && (position > height()))
    position = height();

  return position;
}

bool KDockSplitter::eventFilter(QObject *o, QEvent *e)
{
  QMouseEvent *mev;
  bool handled = false;
  int factor = (mHighResolution)? 10000:100;

  switch (e->type()) {
    case QEvent::MouseMove:
      mev= (QMouseEvent*)e;
      child0->setUpdatesEnabled(mOpaqueResize);
      child1->setUpdatesEnabled(mOpaqueResize);
      if (m_orientation == Horizontal) {
        if ((fixedHeight0!=-1) || (fixedHeight1!=-1))
        {
                handled=true; break;
        }

	if (!mOpaqueResize) {
          int position = checkValue( mapFromGlobal(mev->globalPos()).y() );
          divider->move( 0, position );
        } else {
          xpos = factor * checkValue( mapFromGlobal(mev->globalPos()).y() ) / height();
          resizeEvent(0);
          divider->repaint(true);
        }
      } else {
        if ((fixedWidth0!=-1) || (fixedWidth1!=-1))
        {
                handled=true; break;
        }
        if (!mOpaqueResize) {
          int position = checkValue( mapFromGlobal(QCursor::pos()).x() );
          divider->move( position, 0 );
        } else {
          xpos = factor * checkValue( mapFromGlobal( mev->globalPos()).x() ) / width();
          resizeEvent(0);
          divider->repaint(true);
        }
      }
      handled= true;
      break;
    case QEvent::MouseButtonRelease:
      child0->setUpdatesEnabled(true);
      child1->setUpdatesEnabled(true);
      mev= (QMouseEvent*)e;
      if (m_orientation == Horizontal){
        if ((fixedHeight0!=-1) || (fixedHeight1!=-1))
        {
                handled=true; break;
        }
        xpos = factor* checkValue( mapFromGlobal(mev->globalPos()).y() ) / height();
        resizeEvent(0);
        divider->repaint(true);
      } else {
        if ((fixedWidth0!=-1) || (fixedWidth1!=-1))
        {
                handled=true; break;
        }
        xpos = factor* checkValue( mapFromGlobal(mev->globalPos()).x() ) / width();
        resizeEvent(0);
        divider->repaint(true);
      }
      handled= true;
      break;
    default:
      break;
  }
  return (handled) ? true : QWidget::eventFilter( o, e );
}

bool KDockSplitter::event( QEvent* e )
{
  if ( e->type() == QEvent::LayoutHint ){
    // change children min/max size
    setupMinMaxSize();
    setSeparatorPos(xpos);
  }
  return QWidget::event(e);
}

QWidget* KDockSplitter::getAnother( QWidget* w ) const
{
  return ( w == child0 ) ? child1 : child0;
}

void KDockSplitter::updateName()
{
  if ( !initialised ) return;

  QString new_name = QString( child0->name() ) + "," + child1->name();
  parentWidget()->setName( new_name.latin1() );
  parentWidget()->setCaption( child0->caption() + "," + child1->caption() );
  parentWidget()->repaint( false );

  ((KDockWidget*)parentWidget())->firstName = child0->name();
  ((KDockWidget*)parentWidget())->lastName = child1->name();
  ((KDockWidget*)parentWidget())->splitterOrientation = m_orientation;

  QWidget* p = parentWidget()->parentWidget();
  if ( p != 0L && p->inherits("KDockSplitter" ) )
    ((KDockSplitter*)p)->updateName();
}

void KDockSplitter::setOpaqueResize(bool b)
{
  mOpaqueResize = b;
}

bool KDockSplitter::opaqueResize() const
{
  return mOpaqueResize;
}

void KDockSplitter::setKeepSize(bool b)
{
  mKeepSize = b;
}

bool KDockSplitter::keepSize() const
{
  return mKeepSize;
}

void KDockSplitter::setHighResolution(bool b)
{
  if (mHighResolution) {
    if (!b) xpos = xpos/100;
  } else {
    if (b) xpos = xpos*100;
  }
  mHighResolution = b;
}

bool KDockSplitter::highResolution() const
{
  return mHighResolution;
}


/*************************************************************************/
KDockButton_Private::KDockButton_Private( QWidget *parent, const char * name )
:QPushButton( parent, name )
{
  moveMouse = false;
  setFocusPolicy( NoFocus );
}

KDockButton_Private::~KDockButton_Private()
{
}

void KDockButton_Private::drawButton( QPainter* p )
{
  p->fillRect( 0,0, width(), height(), QBrush(colorGroup().brush(QColorGroup::Background)) );
  p->drawPixmap( (width() - pixmap()->width()) / 2, (height() - pixmap()->height()) / 2, *pixmap() );
  if ( moveMouse && !isDown() ){
    p->setPen( white );
    p->moveTo( 0, height() - 1 );
    p->lineTo( 0, 0 );
    p->lineTo( width() - 1, 0 );

    p->setPen( colorGroup().dark() );
    p->lineTo( width() - 1, height() - 1 );
    p->lineTo( 0, height() - 1 );
  }
  if ( isOn() || isDown() ){
    p->setPen( colorGroup().dark() );
    p->moveTo( 0, height() - 1 );
    p->lineTo( 0, 0 );
    p->lineTo( width() - 1, 0 );

    p->setPen( white );
    p->lineTo( width() - 1, height() - 1 );
    p->lineTo( 0, height() - 1 );
  }
}

void KDockButton_Private::enterEvent( QEvent * )
{
  moveMouse = true;
  repaint();
}

void KDockButton_Private::leaveEvent( QEvent * )
{
  moveMouse = false;
  repaint();
}

/*************************************************************************/
KDockWidgetPrivate::KDockWidgetPrivate()
  : QObject()
  ,index(-1)
  ,splitPosInPercent(50)
  ,pendingFocusInEvent(false)
  ,blockHasUndockedSignal(false)
  ,pendingDtor(false)
  ,forcedWidth(-1)
  ,forcedHeight(-1)
  ,isContainer(false)
  ,container(0)
  ,resizePos(0,0)
  ,resizing(false)
{
#ifndef NO_KDE2
  windowType = NET::Normal;
#endif

  _parent = 0L;
  transient = false;
}

KDockWidgetPrivate::~KDockWidgetPrivate()
{
}

void KDockWidgetPrivate::slotFocusEmbeddedWidget(QWidget* w)
{
   if (w) {
      QWidget* embeddedWdg = ((KDockWidget*)w)->getWidget();
      if (embeddedWdg && ((embeddedWdg->focusPolicy() == QWidget::ClickFocus) || (embeddedWdg->focusPolicy() == QWidget::StrongFocus))) {
         embeddedWdg->setFocus();
      }
   }
}

#ifndef NO_INCLUDE_MOCFILES // for Qt-only projects, because tmake doesn't take this name
#include "kdockwidget_compat.moc"
#endif
