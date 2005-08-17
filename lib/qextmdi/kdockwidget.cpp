
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
#include "kdockwidget.h"
#include "kdockwidget_private.h"
#include "kdockwidget_p.h"

#include <qapplication.h>
#include <qlayout.h>
#include <qpainter.h>
#include <qobject.h>
#include <q3strlist.h>
#include <qcursor.h>
#include <qwidget.h>
#include <qtabwidget.h>
#include <qtooltip.h>
#include <qstyle.h>
//Added by qt3to4:
#include <QPixmap>
#include <QPaintEvent>
#include <QChildEvent>
#include <Q3PtrList>
#include <QEvent>
#include <Q3Frame>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QResizeEvent>
#include <QMouseEvent>

#ifndef NO_KDE2
#include <kconfig.h>
#include <kglobal.h>
#include <klocale.h>
#include <ktoolbar.h>
#include <kpopupmenu.h>
#include <kwin.h>
#include <kdebug.h>
#include <kglobalsettings.h>

#include "config.h"
#ifdef Q_WS_X11
#include <X11/X.h>
#include <X11/Xlib.h>
#endif

#else
#include <q3toolbar.h>
#include <q3popupmenu.h>
#endif

#include <stdlib.h>

#undef BORDERLESS_WINDOWS

#define DOCK_CONFIG_VERSION "0.0.5"

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
 * A special kind of KMainWindow that is able to have dockwidget child widgets.
 *
 * The main widget should be a dockwidget where other dockwidgets can be docked to
 * the left, right, top, bottom or to the middle.
 * Furthermore, the KDockMainWindow has got the KDocManager and some data about the dock states.
 *
 * @author Max Judin.
*/
KDockMainWindow::KDockMainWindow( QWidget* parent, const char *name, Qt::WFlags f)
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
  if ( view->isA("KDockWidget") ){
    if ( view->parent() != this ) ((KDockWidget*)view)->applyToWidget( this );
  }

#ifndef NO_KDE2
  KMainWindow::setCentralWidget(view);
#else
  Q3MainWindow::setCentralWidget(view);
#endif
}

KDockWidget* KDockMainWindow::createDockWidget( const QString& name, const QPixmap &pixmap, QWidget* parent, const QString& strCaption, const QString& strTabPageLabel)
{
  return new KDockWidget( dockManager, name.latin1(), pixmap, parent, strCaption, strTabPageLabel );
}

void KDockMainWindow::makeDockVisible( KDockWidget* dock )
{
  if ( dock )
    dock->makeDockVisible();
}

void KDockMainWindow::makeDockInvisible( KDockWidget* dock )
{
  if ( dock )
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
  if (!pSender->inherits("KDockWidget")) return;
  KDockWidget* pDW = (KDockWidget*) pSender;
  emit dockWidgetHasUndocked( pDW);
}

/*************************************************************************/
KDockWidgetAbstractHeaderDrag::KDockWidgetAbstractHeaderDrag( KDockWidgetAbstractHeader* parent, KDockWidget* dock, const char* name )
:Q3Frame( parent, name )
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
:Q3Frame( parent, name )
{
}
/*************************************************************************/
KDockWidgetHeader::KDockWidgetHeader( KDockWidget* parent, const char* name )
:KDockWidgetAbstractHeader( parent, name )
{
#ifdef BORDERLESS_WINDOWS
  setCursor(QCursor(Qt::ArrowCursor));
#endif
  d = new KDockWidgetHeaderPrivate( this );

  layout = new QHBoxLayout( this );
  layout->setResizeMode( QLayout::SetMinimumSize );

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
   for (Q3PtrListIterator<KDockButton_Private> it( d->btns );it.current();++it) {
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

KDockWidgetHeaderDrag *KDockWidgetHeader::dragPanel() {
	return drag;
}

void KDockWidgetHeader::setDragPanel( KDockWidgetHeaderDrag* nd )
{
  if ( !nd ) return;

  delete layout;
  layout = new QHBoxLayout( this );
  layout->setResizeMode( QLayout::SetMinimumSize );

  delete drag;
  drag = nd;
  if (drag->parentWidget()!=this) {
	drag->reparent(this,QPoint(0,0));
  }


  layout->addWidget( drag );
  layout->addWidget( dockbackButton );
  layout->addWidget( d->dummy );
  layout->addWidget( d->toDesktopButton );
  layout->addWidget( stayButton );
  bool dontShowDummy=drag->isVisibleTo(this) || dockbackButton->isVisibleTo(this) ||
	d->toDesktopButton->isVisibleTo(this) || stayButton->isVisibleTo(this) ||
	closeButton->isVisibleTo(this);
  for (Q3PtrListIterator<KDockButton_Private> it( d->btns );it.current();++it) {
      layout->addWidget(it.current());
	dontShowDummy=dontShowDummy || (it.current()->isVisibleTo(this));
  }
  if (dontShowDummy) d->dummy->hide(); else d->dummy->show();
  layout->addWidget( closeButton );
  layout->activate();
  kdDebug(282)<<"KdockWidgetHeader::setDragPanel:minimum height="<<layout->minimumSize().height()<<endl;
#ifdef __GNUC__
#warning FIXME
#endif
  drag->setFixedHeight( closeButton->height()); // /*layout->minimumS*/sizeHint().height() );
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
	layout->setResizeMode( QLayout::SetMinimumSize );

	layout->addWidget( drag );
 	layout->addWidget( dockbackButton );
	layout->addWidget( d->toDesktopButton );
	layout->addWidget( d->dummy);
	layout->addWidget( stayButton );
	 bool dontShowDummy=drag->isVisibleTo(this) || dockbackButton->isVisibleTo(this) ||
	        d->toDesktopButton->isVisibleTo(this) || stayButton->isVisibleTo(this) ||
        	closeButton->isVisibleTo(this);
	 for (Q3PtrListIterator<KDockButton_Private> it( d->btns );it.current();++it) {
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
  kdDebug(282)<<"KDockWidgetHeader::showUndockButton("<<show<<")"<<endl;
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
   * These flags store information about the splitter behavior
   */
  bool splitterOpaqueResize;
  bool splitterKeepSize;
  bool splitterHighResolution;

  QPointer<KDockWidget> mainDockWidget;

  QObjectList containerDocks;

  QPointer<KDockWidget> leftContainer;
  QPointer<KDockWidget> topContainer;
  QPointer<KDockWidget> rightContainer;
  QPointer<KDockWidget> bottomContainer;
  int m_readDockConfigMode;
};


/*************************************************************************/
KDockWidget::KDockWidget( KDockManager* dockManager, const char* name, const QPixmap &pixmap, QWidget* parent, const QString& strCaption, const QString& strTabPageLabel, Qt::WFlags f)
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
  ,isGroup(false)
{
  d = new KDockWidgetPrivate();  // create private data

  d->_parent = parent;

  layout = new QVBoxLayout( this );
  layout->setResizeMode( QLayout::SetMinimumSize );

  manager = dockManager;
  manager->childDock->append( this );
  installEventFilter( manager );

  eDocking = DockFullDocking;
  sDocking = DockFullSite;

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
         if (contWid) {
         	KDockContainer *x = dynamic_cast<KDockContainer*>(contWid);
                if (x) {
                        x->setPixmap(this,pixmap);
                }
         }
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

  if (latestKDockContainer()) {
    KDockContainer *x = dynamic_cast<KDockContainer*>(latestKDockContainer());
    if (x) {
      x->removeWidget(this);
    }
  }
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
		kdDebug(282)<<"KDockWidget::mousePressEvent"<<endl;

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
		kdDebug(282)<<"mousemovevent"<<endl;
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
			if (btop) setCursor(QCursor(Qt::SizeFDiagCursor));
			else
			if (bbottom) setCursor(QCursor(Qt::SizeBDiagCursor));
			else setCursor(QCursor(Qt::SizeHorCursor));
		}
		else
		if (bbottom)
		{
			d->resizeMode=KDockWidgetPrivate::ResizeBottom;
			d->resizePos=QPoint(0,height())-mme->pos();
		}
		else
		if  (btop) setCursor(QCursor(Qt::SizeVerCursor));
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
	kdDebug(282)<<"mousemovevent"<<endl;
	if (bright)
	{
		if (btop) setCursor(QCursor(Qt::SizeBDiagCursor));
		else
		if (bbottom) setCursor(QCursor(Qt::SizeFDiagCursor));
		else setCursor(QCursor(Qt::SizeHorCursor));
	}
	else if (bleft)
	{
		if (btop) setCursor(QCursor(Qt::SizeFDiagCursor));
		else
		if (bbottom) setCursor(QCursor(Qt::SizeBDiagCursor));
		else setCursor(QCursor(Qt::SizeHorCursor));
	}
	else
	if (bbottom ||  btop) setCursor(QCursor(Qt::SizeVerCursor));
	else setCursor(QCursor(Qt::ArrowCursor));
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
	if (dynamic_cast<KDockContainer*>(d->container.operator->())) return d->container;
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
    layout->setResizeMode( QLayout::SetMinimumSize );
    layout->addWidget( header );
     setWidget( widget );
  } else {
    header = h;
    layout->addWidget( header );
  }
  kdDebug(282)<<caption()<<": KDockWidget::setHeader"<<endl;
  setEnableDocking(eDocking);
}

void KDockWidget::setEnableDocking( int pos )
{
  eDocking = pos;
  if( header && header->inherits( "KDockWidgetHeader" ) )
     ( ( KDockWidgetHeader* ) header )->showUndockButton( pos & DockDesktop );
  updateHeader();
}

void KDockWidget::updateHeader()
{
  if ( parent() ){
#ifdef BORDERLESS_WINDOWS
      layout->setMargin(0);
      setMouseTracking(false);
      setCursor(QCursor(Qt::ArrowCursor));
#endif

    if ( (parent() == manager->main) || isGroup || (eDocking == KDockWidget::DockNone) ){
      header->hide();
    } else {
      header->setTopLevel( false );
      if (widget && dynamic_cast<KDockContainer*>(widget))
        header->hide();
      else
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

  if ( s && s->inherits("KDockMainWindow") ){
    ((KDockMainWindow*)s)->setView( this );
  }

  if ( manager && s == manager->main ){
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
#endif // BORDERLESS_WINDOW
#endif // Q_WS_X11
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
    case QEvent::WindowTitleChange:
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
	Qt::Orientation orientation=((pos==Qt::DockLeft) || (pos==Qt::DockRight)) ? Qt::Vertical:Qt::Horizontal;
		if (((KDockSplitter*)(parent()))->orientation()==orientation)
		{
			KDockWidget *neighbor=
				((pos==Qt::DockLeft)||(pos==Qt::DockTop))?
				static_cast<KDockWidget*>(((KDockSplitter*)(parent()))->getFirst()):
				static_cast<KDockWidget*>(((KDockSplitter*)(parent()))->getLast());

			if (neighbor==this)
			return (static_cast<KDockWidget*>(parent()->parent())->findNearestDockWidget(pos));
			else
			if (neighbor->getWidget() && (neighbor->getWidget()->qt_cast("KDockTabGroup")))
				return (KDockWidget*)(((KDockTabGroup*)neighbor->getWidget())->page(0));
			else
			return neighbor;
		}
		else
		return (static_cast<KDockWidget*>(parent()->parent())->findNearestDockWidget(pos));

	return 0;
}


KDockWidget* KDockWidget::manualDock( KDockWidget* target, DockPosition dockPos, int spliPos, QPoint pos, bool check, int tabIndex )
{
  if (this == target)
    return 0L;  // docking to itself not possible
//  kdDebug(282)<<"manualDock called "<<endl;
  bool succes = true; // tested flag

  // check allowed this dock submit this operations
  if ( !(eDocking & (int)dockPos) ){
    succes = false;
//  kdDebug(282)<<"KDockWidget::manualDock(): success = false (1)"<<endl;
  }

  KDockWidget *tmpTarget;
  switch (dockPos) {
	case Qt::DockLeft:tmpTarget=dockManager()->d->leftContainer;
		break;
	case Qt::DockRight:tmpTarget=dockManager()->d->rightContainer;
		break;
	case Qt::DockBottom:tmpTarget=dockManager()->d->bottomContainer;
		break;
	case Qt::DockTop:tmpTarget=dockManager()->d->topContainer;
		break;
	default: tmpTarget=0;
  }

  if (this!=tmpTarget) {
    if (target && (target==dockManager()->d->mainDockWidget) && tmpTarget) {
  	return manualDock(tmpTarget,DockCenter,spliPos,pos,check,tabIndex);
    }
  }

  // check allowed target submit this operations
  if ( target && !(target->sDocking & (int)dockPos) ){
    succes = false;
//  kdDebug(282)<<"KDockWidget::manualDock(): success = false (2)"<<endl;
  }

  if ( parent() && !parent()->inherits("KDockSplitter") && !parentDockTabGroup() &&
  	!(dynamic_cast<KDockContainer*>(parent())) && !parentDockContainer()){
//  kdDebug(282)<<"KDockWidget::manualDock(): success = false (3)"<<endl;
//  kdDebug(282)<<parent()->name()<<endl;
    succes = false;
  }

//  kdDebug(282)<<"KDockWidget::manualDock(): success == false "<<endl;
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

//  kdDebug(282)<<"Looking for  KDockTabGroup"<<endl;
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
//  	kdDebug(282)<<"Looking for  KDockContainer"<<endl;
  	QWidget *contWid=target->parentDockContainer();
	  if (!contWid) contWid=target->widget;
	  if (contWid)
	  {
	  	KDockContainer *cont=dynamic_cast<KDockContainer*>(contWid);
		  if (cont)
		  {
			if (latestKDockContainer() && (latestKDockContainer()!=contWid)) {
				KDockContainer* dc = dynamic_cast<KDockContainer*>(latestKDockContainer());
				if (dc) {
					dc->removeWidget(this);
				}
			}
//			kdDebug(282)<<"KDockContainerFound"<<endl;
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
  if( target->formerBrotherDockWidget ) {
    newDock->setFormerBrotherDockWidget(target->formerBrotherDockWidget);
    if( formerBrotherDockWidget )
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
    if ( dockPos == KDockWidget::DockTop  || dockPos == KDockWidget::DockBottom ) panner = new KDockSplitter( newDock, "_dock_split_", Qt::Horizontal, spliPos, manager->splitterHighResolution() );
    if ( dockPos == KDockWidget::DockLeft || dockPos == KDockWidget::DockRight  ) panner = new KDockSplitter( newDock, "_dock_split_", Qt::Vertical , spliPos, manager->splitterHighResolution() );
    newDock->setWidget( panner );

    panner->setOpaqueResize(manager->splitterOpaqueResize());
    panner->setKeepSize(manager->splitterKeepSize());
    panner->setFocusPolicy( Qt::NoFocus );
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
  if ( candidate && candidate->inherits("KDockTabGroup") ) return (KDockTabGroup*)candidate;
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

KDockWidget::DockPosition KDockWidget::currentDockPosition() const
{
	return currentDockPos;
}

void KDockWidget::undock()
{
//  kdDebug(282)<<"KDockWidget::undock()"<<endl;

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
    setFormerBrotherDockWidget((KDockWidget*)parentTab->page(0));
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
      if ( !parentOfDockWidget ){
          if ( isV ) lastTab->show();
      } else {
        if ( parentOfDockWidget->inherits("KDockSplitter") ){
          KDockSplitter* split = (KDockSplitter*)parentOfDockWidget;
          lastTab->applyToWidget( split );
          split->deactivate();
          if ( split->getFirst() == parentOfTab ){
            split->activate( lastTab );
            if ( ((KDockWidget*)split->parent())->splitterOrientation == Qt::Vertical )
              emit ((KDockWidget*)split->getAnother(parentOfTab))->docking( parentOfTab, KDockWidget::DockLeft );
            else
              emit ((KDockWidget*)split->getAnother(parentOfTab))->docking( parentOfTab, KDockWidget::DockTop );
          } else {
            split->activate( 0L, lastTab );
            if ( ((KDockWidget*)split->parent())->splitterOrientation == Qt::Vertical )
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
//	  kdDebug(282)<<"undocked from dockcontainer"<<endl;
	  undockedFromContainer=true;
	  KDockContainer* dc = dynamic_cast<KDockContainer*>(d->container.operator->());
	  if (dc) {
		  dc->undockWidget(this);
		  setFormerBrotherDockWidget(dc->parentDockWidget());
	  }
	  applyToWidget( 0L );
  }
   if (!undockedFromContainer) {
/*********************************************************************************************/
    if ( parentW->inherits("KDockSplitter") ){
      KDockSplitter* parentSplitterOfDockWidget = (KDockSplitter*)parentW;
      d->splitPosInPercent = parentSplitterOfDockWidget->separatorPos();

      KDockWidget* secondWidget = (KDockWidget*)parentSplitterOfDockWidget->getAnother( this );
      KDockWidget* group        = (KDockWidget*)parentSplitterOfDockWidget->parentWidget();
      setFormerBrotherDockWidget(secondWidget);
      applyToWidget( 0L );
      group->hide();

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
  if (!mw->ownCursor()) mw->setCursor(QCursor(Qt::ArrowCursor));
#endif
  widget = mw;
  delete layout;

  layout = new QVBoxLayout( this );
  layout->setResizeMode( QLayout::SetMinimumSize );

  KDockContainer* dc = dynamic_cast<KDockContainer*>(widget);
  if (dc)
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
  emit widgetSet(mw);
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
    if ( manager->main->inherits("KDockMainWindow") ){
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
  if (parentDockContainer()) {
    QWidget *contWid=parentDockContainer();
    KDockContainer *x = dynamic_cast<KDockContainer*>(contWid);
    if (x) {
      x->showWidget(this);
    }
  }
  if ( isVisible() ) return;

  QWidget* p = parentWidget();
  while ( p ){
    if ( !p->isVisible() )
      p->show();
    p = p->parentWidget();
  }
  if( !parent() ) // is undocked
    dockBack();
  show();
}

void KDockWidget::setFormerBrotherDockWidget(KDockWidget *dockWidget)
{
  formerBrotherDockWidget = dockWidget;
  if( formerBrotherDockWidget )
    QObject::connect( formerBrotherDockWidget, SIGNAL(iMBeingClosed()),
                      this, SLOT(loseFormerBrotherDockWidget()) );
}

void KDockWidget::loseFormerBrotherDockWidget()
{
  if( formerBrotherDockWidget )
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
    QObjectList* cl = queryList("KDockWidget");
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

  // else dockback to the dockmainwindow (default behavior)
  manualDock( ((KDockMainWindow*)manager->main)->getMainDockWidget(), formerDockPos, d->splitPosInPercent, QPoint(0,0), false, d->index);
  formerBrotherDockWidget = 0L;
  if (parent())
    makeDockVisible();
}

bool KDockWidget::isDockBackPossible() const
{
  if( !(formerBrotherDockWidget) || !(formerBrotherDockWidget->dockSite() & formerDockPos))
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
  ,dragging(false)
  ,undockProcess(false)
  ,dropCancel(true)
{
  d = new KDockManagerPrivate;

  d->readyToDrag = false;
  d->mainDockWidget=0;

#ifndef NO_KDE2
  d->splitterOpaqueResize = KGlobalSettings::opaqueResize();
#else
  d->splitterOpaqueResize = false;
#endif

  d->splitterKeepSize = false;
  d->splitterHighResolution = false;
  d->m_readDockConfigMode = WrapExistingWidgetsOnly; // default as before

  main->installEventFilter( this );

  undockProcess = false;

  menuData = new Q3PtrList<MenuDockData>;
  menuData->setAutoDelete( true );
  menuData->setAutoDelete( true );

#ifndef NO_KDE2
  menu = new KPopupMenu();
#else
  menu = new Q3PopupMenu();
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

  if ( obj->inherits("KDockWidgetAbstractHeaderDrag") ){
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
        if ( ((QMouseEvent*)event)->button() == Qt::LeftButton ){
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
        if ( ((QMouseEvent*)event)->button() == Qt::LeftButton ){
          if ( dragging ){
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
          dragging = false;
          dropCancel = true;
        }
        break;
      case QEvent::MouseMove:
        if ( dragging ) {

#ifdef BORDERLESS_WINDOWS
//BEGIN TEST
	  KDockWidget *oldMoveWidget;
	  if (!curdw->parent())
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

          if ( !pDockWdgAtCursor && !(curdw->eDocking & (int)KDockWidget::DockDesktop) ){
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
          if ( (((QMouseEvent*)event)->state() == Qt::LeftButton) &&
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
    if ( !p->inherits("KDockWidget") ) {
      return 0L;
    }
    w = p;
  }
  if ( qt_find_obj_child( w, "KDockSplitter", "_dock_split_" ) ) return 0L;
  if ( qt_find_obj_child( w, "KDockTabGroup", "_dock_tab" ) ) return 0L;
  if (dynamic_cast<KDockContainer*>(w)) return 0L;

  if (!childDockWidgetList) return 0L;
  if ( childDockWidgetList->find(w) != -1 ) return 0L;
  if ( currentDragWidget->isGroup && ((KDockWidget*)w)->parentDockTabGroup() ) return 0L;

  KDockWidget* www = (KDockWidget*)w;
  if ( www->sDocking == (int)KDockWidget::DockNone ) return 0L;
  if( !www->widget )
    return 0L;

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
          if ( w->inherits("KDockWidget") ) ww = w;
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
          if ( w->inherits("KDockWidget") ) list->append( w );
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
  dragging = true;

  QApplication::setOverrideCursor(QCursor(Qt::SizeAllCursor));
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
  if ( !currentMoveWidget && (!(currentDragWidget->eDocking & (int)KDockWidget::DockDesktop)) ) {
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
                                   const QString &subTagName, const Q3StrList &list)
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


static Q3StrList listEntry(QDomElement &base, const QString &tagName, const QString &subTagName)
{
    Q3StrList list;

    for( QDomNode n = base.namedItem(tagName).firstChild(); !n.isNull(); n = n.nextSibling() )
    {
        QDomElement subel = n.toElement();
        if (subel.tagName() == subTagName)
            list.append(subel.firstChild().toText().data().latin1());
    }

    return list;
}


void KDockManager::writeConfig(QDomElement &base)
{
    // First of all, clear the tree under base
    while (!base.firstChild().isNull())
        base.removeChild(base.firstChild());
    QDomDocument doc = base.ownerDocument();

    Q3StrList nameList;
    QString mainWidgetStr;

    // collect widget names
    QStringList nList;
    QObjectListIt it(*childDock);
    KDockWidget *obj1;
    while ( (obj1=(KDockWidget*)it.current()) ) {
        if ( obj1->parent() == main )
            mainWidgetStr = QString::fromLatin1(obj1->name());
        nList.append(obj1->name());
        ++it;
    }

    for (QObjectListIt it(d->containerDocks);it.current();++it)
    {
        KDockContainer* dc = dynamic_cast<KDockContainer*>(((KDockWidget*)it.current())->widget);
        if (dc) {
                dc->prepareSave(nList);
        }
    }

    QStringList::Iterator nListIt=nList.begin();
    while ( nListIt!=nList.end() ) {
        KDockWidget *obj = getDockWidgetFromName( *nListIt);
        if ((obj->isGroup && (!obj->d->isContainer)) && (nameList.find( obj->firstName.latin1() ) == -1
                             || nameList.find(obj->lastName.latin1()) == -1)) {
            // Skip until children are saved (why?)
            ++nListIt;
//            nList.next();
//falk?            if ( !nList.current() ) nList.first();
            continue;
        }

        QDomElement groupEl;
	if (obj->d->isContainer) {
      		KDockContainer* x = dynamic_cast<KDockContainer*>(obj->widget);
       		if (x) {
			groupEl=doc.createElement("dockContainer");
          		x->save(groupEl);
       		}
    	} else
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

            Q3StrList list;
            for ( int i = 0; i < ((KDockTabGroup*)obj->widget)->count(); ++i )
                list.append( ((KDockTabGroup*)obj->widget)->page( i )->name() );
            groupEl.appendChild(createListEntry(doc, "tabs", "tab", list));
            groupEl.appendChild(createNumberEntry(doc, "currentTab", ((KDockTabGroup*)obj->widget)->currentPageIndex()));
            if (!obj->parent()) {
                groupEl.appendChild(createStringEntry(doc, "dockBackTo", obj->formerBrotherDockWidget ? obj->formerBrotherDockWidget->name() : ""));
                groupEl.appendChild(createNumberEntry(doc, "dockBackToPos", obj->formerDockPos));
            }
        } else {
            //// Save an ordinary dock widget
            groupEl = doc.createElement("dock");
            groupEl.appendChild(createStringEntry(doc, "tabCaption", obj->tabPageLabel()));
            groupEl.appendChild(createStringEntry(doc, "tabToolTip", obj->toolTipString()));
            if (!obj->parent()) {
                groupEl.appendChild(createStringEntry(doc, "dockBackTo", obj->formerBrotherDockWidget ? obj->formerBrotherDockWidget->name() : ""));
                groupEl.appendChild(createNumberEntry(doc, "dockBackToPos", obj->formerDockPos));
            }
        }

        groupEl.appendChild(createStringEntry(doc, "name", QString::fromLatin1(obj->name())));
        groupEl.appendChild(createBoolEntry(doc, "hasParent", obj->parent()));
        if ( !obj->parent() ) {
            groupEl.appendChild(createRectEntry(doc, "geometry", QRect(main->frameGeometry().topLeft(), main->size())));
            groupEl.appendChild(createBoolEntry(doc, "visible", obj->isVisible()));
        }
        if (obj->header && obj->header->inherits("KDockWidgetHeader")) {
            KDockWidgetHeader *h = static_cast<KDockWidgetHeader*>(obj->header);
            groupEl.appendChild(createBoolEntry(doc, "dragEnabled", h->dragEnabled()));
        }

        base.appendChild(groupEl);
        nameList.append(obj->name());
        nList.remove(nListIt);
        nListIt=nList.begin();
    }

    if (main->inherits("KDockMainWindow")) {
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
        && base.namedItem("dock").isNull()
	&& base.namedItem("dockContainer").isNull()) {
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

    // firstly, recreate all common dockwidgets
    for( QDomNode n = base.firstChild(); !n.isNull(); n = n.nextSibling() )
    {
        QDomElement childEl = n.toElement();
        if (childEl.tagName() != "dock") continue;

        // Read an ordinary dock widget
        KDockWidget *obj = getDockWidgetFromName(stringEntry(childEl, "name"));
        obj->setTabPageLabel(stringEntry(childEl, "tabCaption"));
        obj->setToolTipString(stringEntry(childEl, "tabToolTip"));

        if (!boolEntry(childEl, "hasParent")) {
            QRect r = rectEntry(childEl, "geometry");
            obj = getDockWidgetFromName(stringEntry(childEl, "name"));
            obj->applyToWidget(0);
            obj->setGeometry(r);
            if (boolEntry(childEl, "visible"))
                obj->QWidget::show();
        }

        if (obj && obj->header && obj->header->inherits("KDockWidgetHeader")) {
            KDockWidgetHeader *h = static_cast<KDockWidgetHeader*>(obj->header);
            h->setDragEnabled(boolEntry(childEl, "dragEnabled"));
        }
    }

    // secondly, now iterate again and create the groups and tabwidgets, apply the dockwidgets to them
    for( QDomNode n = base.firstChild(); !n.isNull(); n = n.nextSibling() )
    {
        QDomElement childEl = n.toElement();
        if (childEl.isNull()) continue;

        KDockWidget *obj = 0;

	if (childEl.tagName() == "dockContainer") {

		KDockWidget *cont=getDockWidgetFromName(stringEntry(childEl, "name"));
		kdDebug(282)<<"dockContainer: "<<stringEntry(childEl,"name")<<endl;
		if (!(cont->d->isContainer)) {
			kdDebug(282)<<"restoration of dockContainer is only supported for already existing dock containers"<<endl;
		} else {
			KDockContainer *dc=dynamic_cast<KDockContainer*>(cont->getWidget());
			if (!dc) kdDebug(282)<<"Error while trying to handle dockcontainer configuration restoration"<<endl;
				else {
					dc->load(childEl);
					removeFromAutoCreateList(cont);
				}

		}
	}
	else
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
                                        (orientation == (int)Qt::Vertical)? KDockWidget::DockLeft : KDockWidget::DockTop,
                                        separatorPos);
                if (obj)
                    obj->setName(name.latin1());
            }
        } else if (childEl.tagName() == "tabGroup") {
            // Read a tab group
            QString name = stringEntry(childEl, "name");
            Q3StrList list = listEntry(childEl, "tabs", "tab");

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
        } else {
            continue;
        }

        if (!boolEntry(childEl, "hasParent")) {
            QRect r = rectEntry(childEl, "geometry");
            obj = getDockWidgetFromName(stringEntry(childEl, "name"));
            obj->applyToWidget(0);
            obj->setGeometry(r);
            if (boolEntry(childEl, "visible"))
                obj->QWidget::show();
        }

        if (obj && obj->header && obj->header->inherits("KDockWidgetHeader")) {
            KDockWidgetHeader *h = static_cast<KDockWidgetHeader*>(obj->header);
            h->setDragEnabled(boolEntry(childEl, "dragEnabled"));
        }
    }

    // thirdly, now that all ordinary dockwidgets are created,
    // iterate them again and link them with their corresponding dockwidget for the dockback action
    for( QDomNode n = base.firstChild(); !n.isNull(); n = n.nextSibling() )
    {
        QDomElement childEl = n.toElement();

        if (childEl.tagName() != "dock" && childEl.tagName() != "tabGroup")
            continue;

        KDockWidget *obj = 0;

        if (!boolEntry(childEl, "hasParent")) {
            // Read a common toplevel dock widget
            obj = getDockWidgetFromName(stringEntry(childEl, "name"));
            QString name = stringEntry(childEl, "dockBackTo");
            if (!name.isEmpty()) {
                obj->setFormerBrotherDockWidget(getDockWidgetFromName(name));
            }
            obj->formerDockPos = KDockWidget::DockPosition(numberEntry(childEl, "dockBackToPos"));
            obj->updateHeader();
        }
    }

    if (main->inherits("KDockMainWindow")) {
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

        // only resize + move non-mainwindows
        QRect mr = rectEntry(base, "geometry");
        main->move(mr.topLeft());
        main->resize(mr.size());
    }

    if (isMainVisible)
        main->show();

    if (d->m_readDockConfigMode == WrapExistingWidgetsOnly) {
        finishReadDockConfig(); // remove empty dockwidgets
    }
}

void KDockManager::removeFromAutoCreateList(KDockWidget* pDockWidget)
{
    if (!autoCreateDock) return;
    autoCreateDock->setAutoDelete(false);
    autoCreateDock->removeRef(pDockWidget);
    autoCreateDock->setAutoDelete(true);
}

void KDockManager::finishReadDockConfig()
{
    delete autoCreateDock;
    autoCreateDock = 0;
}

void KDockManager::setReadDockConfigMode(int mode)
{
    d->m_readDockConfigMode = mode;
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

//  kdDebug(282)<<QString("list size: %1").arg(nList.count())<<endl;
  for (QObjectListIt it(d->containerDocks);it.current();++it)
  {
  	KDockContainer* dc = dynamic_cast<KDockContainer*>(((KDockWidget*)it.current())->widget);
	if (dc) {
		dc->prepareSave(nList);
	}
  }
//  kdDebug(282)<<QString("new list size: %1").arg(nList.count())<<endl;

  QStringList::Iterator nListIt=nList.begin();
  while ( nListIt!=nList.end() ){
    //debug("  -Try to save %s", nList.current());
    obj = getDockWidgetFromName( *nListIt );
    QString cname = obj->name();
    if ( obj->header ){
      obj->header->saveConfig( c );
    }
    if (obj->d->isContainer) {
       KDockContainer* x = dynamic_cast<KDockContainer*>(obj->widget);
       if (x) {
          x->save(c,group);
       }
    }
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
          c->writeEntry( cname+":dockBackTo", obj->formerBrotherDockWidget ? obj->formerBrotherDockWidget->name() : "");
          c->writeEntry( cname+":dockBackToPos", obj->formerDockPos);
        } else {
          c->writeEntry( cname+":parent", "yes");
        }
        Q3StrList list;
        for ( int i = 0; i < ((KDockTabGroup*)obj->widget)->count(); ++i )
          list.append( ((KDockTabGroup*)obj->widget)->page( i )->name() );
        c->writeEntry( cname+":tabNames", list );
        c->writeEntry( cname+":curTab", ((KDockTabGroup*)obj->widget)->currentPageIndex() );

        nameList.append( obj->name() );
        findList.append( obj->name() ); // not really need !!!
        //debug("  Save %s", nList.current());
        nList.remove(nListIt);
        nListIt=nList.begin();
      } else {
/*************************************************************************************************/
        c->writeEntry( cname+":tabCaption", obj->tabPageLabel());
        c->writeEntry( cname+":tabToolTip", obj->toolTipString());
        if ( !obj->parent() ){
          c->writeEntry( cname+":type", "NULL_DOCK");
          c->writeEntry( cname+":geometry", QRect(obj->frameGeometry().topLeft(), obj->size()) );
          c->writeEntry( cname+":visible", obj->isVisible());
          c->writeEntry( cname+":dockBackTo", obj->formerBrotherDockWidget ? obj->formerBrotherDockWidget->name() : "");
          c->writeEntry( cname+":dockBackToPos", obj->formerDockPos);
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

  if ( main->inherits("KDockMainWindow") ){
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
  Q3StrList nameList;
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

  // firstly, only the common dockwidgets,
  // they must be restored before e.g. tabgroups are restored
  nameList.first();
  while ( nameList.current() ){
    QString oname = nameList.current();
    c->setGroup( group );
    QString type = c->readEntry( oname + ":type" );
    obj = 0L;

    if ( type == "NULL_DOCK" || c->readEntry( oname + ":parent") == "___null___" ){
      QRect r = c->readRectEntry( oname + ":geometry" );
      obj = getDockWidgetFromName( oname );
      obj->applyToWidget( 0L );
      obj->setGeometry(r);

      c->setGroup( group );
      obj->setTabPageLabel(c->readEntry( oname + ":tabCaption" ));
      obj->setToolTipString(c->readEntry( oname + ":tabToolTip" ));
      if ( c->readBoolEntry( oname + ":visible" ) ){
        obj->QWidget::show();
      }
    }

    if ( type == "DOCK"  ){
      obj = getDockWidgetFromName( oname );
      obj->setTabPageLabel(c->readEntry( oname + ":tabCaption" ));
      obj->setToolTipString(c->readEntry( oname + ":tabToolTip" ));
    }

    if (obj && obj->d->isContainer) {
		dynamic_cast<KDockContainer*>(obj->widget)->load(c,group);
		removeFromAutoCreateList(obj);
    }
    if ( obj && obj->header){
      obj->header->loadConfig( c );
    }
    nameList.next();
  }

  // secondly, after the common dockwidgets, restore the groups and tabgroups
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

      Qt::Orientation p = (Qt::Orientation)c->readNumEntry( oname + ":orientation" );
      if ( first  && last ){
        obj = first->manualDock( last, ( p == Qt::Vertical ) ? KDockWidget::DockLeft : KDockWidget::DockTop, sepPos );
        if (obj){
          obj->setName( oname.latin1() );
        }
      }
    }

    if ( type == "TAB_GROUP" ){
      Q3StrList list;
      KDockWidget* tabDockGroup = 0L;
      c->readListEntry( oname+":tabNames", list );
      KDockWidget* d1 = getDockWidgetFromName( list.first() );
      list.next();
      KDockWidget* d2 = getDockWidgetFromName( list.current() );
      tabDockGroup = d2->manualDock( d1, KDockWidget::DockCenter );
      if ( tabDockGroup ){
        KDockTabGroup* tab = dynamic_cast<KDockTabGroup*>(tabDockGroup->widget);
        list.next();
        while ( list.current() && tabDockGroup ){
          KDockWidget* tabDock = getDockWidgetFromName( list.current() );
          tabDockGroup = tabDock->manualDock( d1, KDockWidget::DockCenter );
          list.next();
        }
        if ( tabDockGroup ){
          tabDockGroup->setName( oname.latin1() );
          c->setGroup( group );
          if (tab)
            tab->showPage( tab->page( c->readNumEntry( oname+":curTab" ) ) );
        }
      }
      obj = tabDockGroup;
    }

    if (obj && obj->d->isContainer)  dynamic_cast<KDockContainer*>(obj->widget)->load(c,group);
    if ( obj && obj->header){
      obj->header->loadConfig( c );
    }
    nameList.next();
  }

  // thirdly, now that all ordinary dockwidgets are created,
  // iterate them again and link the toplevel ones of them with their corresponding dockwidget for the dockback action
  nameList.first();
  while ( nameList.current() ){
    QString oname = nameList.current();
    c->setGroup( group );
    QString type = c->readEntry( oname + ":type" );
    obj = 0L;

    if ( type == "NULL_DOCK" || c->readEntry( oname + ":parent") == "___null___" ){
      obj = getDockWidgetFromName( oname );
      c->setGroup( group );
      QString name = c->readEntry( oname + ":dockBackTo" );
      if (!name.isEmpty()) {
          obj->setFormerBrotherDockWidget(getDockWidgetFromName( name ));
      }
      obj->formerDockPos = KDockWidget::DockPosition(c->readNumEntry( oname + ":dockBackToPos" ));
    }

    nameList.next();
  }

  if ( main->inherits("KDockMainWindow") ){
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
  if (d->m_readDockConfigMode == WrapExistingWidgetsOnly) {
    finishReadDockConfig(); // remove empty dockwidgets
  }

  c->setGroup( group );
  QRect mr = c->readRectEntry("Main:Geometry");
  main->move(mr.topLeft());
  main->resize(mr.size());
  if ( isMainVisible ) main->show();
}
#endif


void KDockManager::dumpDockWidgets() {
  QObjectListIt it( *childDock );
  KDockWidget * obj;
  while ( (obj=(KDockWidget*)it.current()) ) {
    ++it;
    kdDebug(282)<<"KDockManager::dumpDockWidgets:"<<obj->name()<<endl;
  }

}

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
    kdDebug(282)<<"Autocreating dock: "<<dockName<<endl;
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
#ifdef BORDERLESS_WINDOWS
	return
#endif
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

void KDockManager::setSpecialLeftDockContainer(KDockWidget* container) {
	d->leftContainer=container;
}

void KDockManager::setSpecialTopDockContainer(KDockWidget* container) {
	d->topContainer=container;
}

void KDockManager::setSpecialRightDockContainer(KDockWidget* container) {
	d->rightContainer=container;

}

void KDockManager::setSpecialBottomDockContainer(KDockWidget* container) {
	d->bottomContainer=container;
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
  if ( dock )
    dock->makeDockVisible();
}

void KDockArea::makeDockInvisible( KDockWidget* dock )
{
  if ( dock )
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
  if (!pSender->inherits("KDockWidget")) return;
  KDockWidget* pDW = (KDockWidget*) pSender;
  emit dockWidgetHasUndocked( pDW);
}

void KDockArea::resizeEvent(QResizeEvent *rsize)
{
  QWidget::resizeEvent(rsize);
  if (children()){
#ifndef NO_KDE2
//    kdDebug(282)<<"KDockArea::resize"<<endl;
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
       if (split = dynamic_cast<KDockSplitter*>(obj))
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

	if (m_childrenListBegin)
	{
		struct ListItem *tmp=m_childrenListBegin;
		while (tmp)
		{
			struct ListItem *tmp2=tmp->next;
			free(tmp->data);
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
			kdDebug(282)<<"KDockContainer::activateOverlapMode: recalculating sizes"<<endl;
			KDockSplitter *sp= static_cast<KDockSplitter*>(parentDockWidget()->
				parent()->qt_cast("KDockSplitter"));
			if (sp) sp->resizeEvent(0);
		}
	}
}

void KDockContainer::deactivateOverlapMode() {
	if (!m_overlapMode) return;
	m_overlapMode=false;
	if (parentDockWidget()) {
		if (parentDockWidget()->parent()) {
			kdDebug(282)<<"KDockContainer::deactivateOverlapMode: recalculating sizes"<<endl;
			KDockSplitter *sp= static_cast<KDockSplitter*>(parentDockWidget()->
				parent()->qt_cast("KDockSplitter"));
			if (sp) sp->resizeEvent(0);
		}
	}

}

bool KDockContainer::isOverlapMode() {
	return m_overlapMode;
}


bool KDockContainer::dockDragEnter(KDockWidget* dockWidget, QMouseEvent *event) { return false;}
bool KDockContainer::dockDragMove(KDockWidget* dockWidget, QMouseEvent *event) { return false;}
bool KDockContainer::dockDragLeave(KDockWidget* dockWidget, QMouseEvent *event) { return false;}


KDockWidget *KDockContainer::parentDockWidget(){return 0;}

QStringList KDockContainer::containedWidgets() const {
	QStringList tmp;
	for (struct ListItem *it=m_childrenListBegin;it;it=it->next) {
		tmp<<QString(it->data);
	}

	return tmp;
}

void KDockContainer::showWidget(KDockWidget *) {
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
void KDockContainer::load (KConfig*, const QString&){;}
void KDockContainer::save (KConfig*, const QString&){;}
void KDockContainer::load (QDomElement&){;}
void KDockContainer::save (QDomElement&){;}
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
		KDockWidget *dw=static_cast<KDockWidget*>(page(i)->qt_cast("KDockWidget"));
		QWidget *tmp;
		if ((tmp=dw->transientTo())) {
			if (!tT) tT=tmp;
			else {
				if (tT!=tmp) {
					kdDebug(282)<<"KDockTabGroup::transientTo: widget mismatch"<<endl;
					return 0;
				}
			}
		}
	}

	kdDebug(282)<<"KDockTabGroup::transientTo: "<<(tT?"YES":"NO")<<endl;

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


#ifndef NO_INCLUDE_MOCFILES // for Qt-only projects, because tmake doesn't take this name
#include "kdockwidget.moc"
#endif
