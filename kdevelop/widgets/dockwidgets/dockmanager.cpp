/***************************************************************************
                         DockWidget part of KDEStudio
                             -------------------
    copyright            : (C) 1999 by Judin Max
    email                : novaprint@mtu-net.ru
 ***************************************************************************/

#define DOCK_CONFIG_VERSION "0.0.4"

#include <kconfig.h>
#include <qpopupmenu.h>
#include <qapplication.h>
#include <kapp.h>

#include <qobjcoll.h>
#include <qframe.h>
#include <qpainter.h>
#include <ktmainwindow.h>

#include "dockmainwindow.h"
#include "dockmanager.h"
#include "docksplitter.h"
#include "dockmovemanager.h"

#include "close.xpm"
#include "notclose.xpm"

SDockButton::SDockButton( QWidget *parent, const char * name )
:QPushButton( parent, name )
{
  moveMouse = false;
}

SDockButton::~SDockButton()
{
}

void SDockButton::drawButton( QPainter* p )
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

void SDockButton::enterEvent( QEvent * )
{
  moveMouse = true;
  repaint();
}

void SDockButton::leaveEvent( QEvent * )
{
  moveMouse = false;
  repaint();
}

/*************************************************************************/

DockWidget::DockWidget( DockManager* dockManager, const char* name, const QPixmap &pixmap, QWidget* parent )
: QWidget( parent, name )
{
  setCaption( name );

  eDocking = DockTop|DockLeft|DockRight|DockBottom|DockCenter|DockDesktop;
  sDocking = DockTop|DockLeft|DockRight|DockBottom|DockCenter;

  isGroup = false;
  isTabGroup = false;

  manager = dockManager;
  installEventFilter( manager );
  manager->childDock->append( this );

  closeButton = new SDockButton( this, "DockWidgetCloseButton");
  closeButton->setFocusPolicy( NoFocus );
  closeButton->setPixmap(close_xpm);
  connect( closeButton, SIGNAL(clicked()), SLOT(slotCloseButtonClick()));

  stayButton = new SDockButton( this, "DockWidgetStayButton");
  stayButton->setFocusPolicy( NoFocus );
  stayButton->setToggleButton( true );
  stayButton->setPixmap(not_close_xpm);

  closeButton->resize( 9, 9 );
  closeButton->hide();

  stayButton->resize( 9, 9 );
  stayButton->hide();

  QPoint p = QPoint(0,0);
  widget = 0L;
  dockCaptionHeight = 10;

  drawBuffer = 0L;
  pix = new QPixmap(pixmap);

  applyToWidget( parent, p );
}

DockWidget::~DockWidget()
{
  if ( !manager->undockProcess ){
    unDock();
  }
  delete pix;
  delete drawBuffer;
  manager->childDock->remove( this );
}

void DockWidget::setEnableDocking( int pos )
{
  eDocking = pos;
  updateCaptionButton();
}

void DockWidget::updateCaptionButton()
{
  if ( parent() ){
    if ( (parent() == manager->main) || isGroup || (eDocking == DockNone) ){
      dockCaptionHeight = 0;
      closeButton->hide();
      stayButton->hide();
    } else {
      dockCaptionHeight = 10;
      closeButton->show();
      stayButton->show();
    }
  } else {
    dockCaptionHeight = 10;
    closeButton->hide();
    stayButton->hide();
  }
  repaint(false);
}

void DockWidget::applyToWidget( QWidget* s, const QPoint& p )
{
  if ( parent() != s ){
    hide();
    reparent(s, 0, QPoint(0,0), false);
    QApplication::syncX();
  }

  setMouseTracking( true );

  if ( s && s->inherits("DockMainWindow") ){
    ((DockMainWindow*)s)->setDockView( this );
  }

  if ( s == manager->main )
      setGeometry( QRect(QPoint(0,0), manager->main->geometry().size()) );

  if ( !s ){
    //XSetTransientForHint( qt_xdisplay(), winId(), manager->main->topLevelWidget()->winId());
    move(p);
  } else {
    if ( !s->inherits("DockSplitter") && !s->inherits("DockTabCtl") )
      eDocking = DockNone;
  }

  updateCaptionButton();
}

void DockWidget::show()
{
  if ( parent() || manager->main->isVisible() )
    if ( !parent() ){
     emit manager->setDockDefaultPos( this );
     emit setDockDefaultPos();
     if ( parent() ){
        makeDockVisible();
      } else {
        stayButton->setOn( false );
        QWidget::show();
      }
    } else {
     QWidget::show();
    }
}

bool DockWidget::event( QEvent *event )
{
  switch ( event->type() )
  {
    case QEvent::ChildInserted:
      if ( ((QChildEvent*)event)->child()->inherits("SDockButton") ) break;
        widget = (QWidget*)((QChildEvent*)event)->child();
        widget->setGeometry( crect() );
      break;
    case QEvent::ChildRemoved:
      if ( widget == ((QChildEvent*)event)->child() ) widget = 0L;
      break;
    case QEvent::Resize:
      closeButton->move( width() - closeButton->width() - 1, 0 );
      stayButton->move( closeButton->x() - stayButton->width() - 1, 0 );
      if ( widget ){
        widget->setGeometry( crect() );
      }
      if ( drawBuffer ) delete drawBuffer;
      drawBuffer = new QPixmap( width(), height() );
      break;
    case QEvent::Paint:
      paintCaption();
      break;
    case QEvent::Show:
      if ( widget ) widget->show();
      break;
    case QEvent::Hide:
      if ( widget ) widget->hide();
      break;
    default:
      break;
  }
  return QWidget::event( event );
}

QRect DockWidget::crect()
{
  return QRect(0, dockCaptionHeight, width(), height()-dockCaptionHeight);
}

void DockWidget::paintCaption()
{
  if ( dockCaptionHeight == 0 ) return;

  QPainter paint;
  int delta = ( parent() ) ? 24:0;

  paint.begin( drawBuffer );
  paint.fillRect( drawBuffer->rect(), QBrush(colorGroup().brush(QColorGroup::Background)) );

  paint.setPen( white );
  paint.drawLine( 1, 3, 1, 2 );
  paint.drawLine( 1, 2, width()-delta, 2 );

  paint.setPen( colorGroup().mid() );
  paint.drawLine( 1, 4, width()-delta, 4 );
  paint.drawLine( width()-delta, 4, width()-delta, 3 );

  paint.setPen( white );
  paint.drawLine( 1, 6, 1, 5 );
  paint.drawLine( 1, 5, width()-delta, 5 );

  paint.setPen( colorGroup().mid() );
  paint.drawLine( 1, 7, width()-delta, 7 );
  paint.drawLine( width()-delta, 7, width()-delta, 6 );

  bitBlt( this,0,0,drawBuffer,0,0,width(),dockCaptionHeight );
  paint.end();
}

DockWidget* DockWidget::manualDock( DockWidget* target, DockPosition dockPos, int spliPos, QPoint pos, bool check )
{
  bool succes = true; // tested flag

  // check allowed this dock submit this operations
  if ( !(eDocking & (int)dockPos) ){
      succes = false;
  }

  // check allowed target submit this operations
  if ( target && !(target->sDocking & (int)dockPos) ){
      succes = false;
  }

  if ( !succes ){
    // try to make another manualDock
    DockWidget* dock_result = 0L;
    if ( target && !check ){
      int another_spliPos = 100 - spliPos;
      DockPosition another__dockPos = DockNone;
      switch ( dockPos ){
        case DockLeft  : another__dockPos = DockRight ; break;
        case DockRight : another__dockPos = DockLeft  ; break;
        case DockTop   : another__dockPos = DockBottom; break;
        case DockBottom: another__dockPos = DockTop   ; break;
        default: break;
      }
      dock_result = target->manualDock( this, another__dockPos, another_spliPos, pos, true );
    }
    return dock_result;
  }
  // end check block

  unDock();

  if ( !target ){
    move( pos );
    show();
    emit manager->change();
    return this;
  }

  QWidget* parentDock = target->parentWidget();

  if ( parentDock && parentDock->inherits("DockTabCtl") ){
    // add to existing TabGroup
    applyToWidget( parentDock );
    ((DockTabCtl*)parentDock)->insertPage( this, caption() );
    ((DockTabCtl*)parentDock)->setPixmap( this, *pix );
    setDockTabName( (DockTabCtl*)parentDock );

    emit manager->change();
    return (DockWidget*)parentDock->parent();
  }

  DockWidget* newDock = new DockWidget( manager, "tempName", QPixmap(""), parentDock );

  if ( dockPos == DockCenter ){
    newDock->isTabGroup = true;
  } else {
    newDock->isGroup = true;
  }
  newDock->eDocking = (target->eDocking & eDocking) & (~(int)DockCenter);

  newDock->applyToWidget( parentDock );

  if ( !parentDock ){
    newDock->move( target->frameGeometry().topLeft() );
    newDock->resize( target->geometry().size() );
    if ( target->isVisibleToTLW() ) newDock->show();
  }


  if ( dockPos == DockCenter )
  {
    DockTabCtl* tab = new DockTabCtl( newDock, "_dock_tab");
    newDock->widget = tab;
    connect( tab, SIGNAL(tabShowPopup(int,QPoint)), manager, SLOT(slotTabShowPopup(int,QPoint)));
    target->applyToWidget( tab );
    applyToWidget( tab );

    tab->insertPage( target, target->caption() );
    tab->setPixmap( target, *(target->pix) );

    tab->insertPage( this, caption() );
    tab->setPixmap( this, *pix );

    setDockTabName( tab );
    tab->show();
  } else {
    DockSplitter* panner = 0L;
    if ( dockPos == DockTop  || dockPos == DockBottom ) panner = new DockSplitter( newDock, "DockPaner", DockSplitter::Horizontal, KNewPanner::Percent, spliPos );
    if ( dockPos == DockLeft || dockPos == DockRight  ) panner = new DockSplitter( newDock, "DockPaner", DockSplitter::Vertical , KNewPanner::Percent, spliPos );
    newDock->widget = panner;
    panner->setFocusPolicy( NoFocus );
    target->applyToWidget( panner );
    applyToWidget( panner );
    if ( dockPos == DockRight || dockPos == DockBottom ) panner->activate( target, this );
    if ( dockPos == DockTop   || dockPos == DockLeft   ) panner->activate( this, target );
    target->show();
    show();
    panner->show();
  }

  if ( parentDock ){
    if ( parentDock->inherits("DockSplitter") ){
      DockSplitter* sp = (DockSplitter*)parentDock;
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

void DockWidget::unDock()
{
  QWidget* parentW = parentWidget();

  if ( !parentW || parentW == manager->main || eDocking == DockNone )
    return;

  manager->blockSignals(true);
  manager->undockProcess = true;

  bool isV = parentW->isVisibleToTLW();
  if ( parentW->inherits("DockTabCtl") ){
    ((DockTabCtl*)parentW)->removePage( this );
    applyToWidget( 0L );
    if ( ((DockTabCtl*)parentW)->pageCount() == 1 ){

      /* last subdock widget in the tab control*/
      DockWidget* lastTab = (DockWidget*)((DockTabCtl*)parentW)->getFirstPage();
      ((DockTabCtl*)parentW)->removePage( lastTab );
      lastTab->applyToWidget( 0L );
      lastTab->move( parentW->mapToGlobal(parentW->frameGeometry().topLeft()) );

      /* DockTabCtl always have a parent is DockWidget*/
      DockWidget* parentOfTab = (DockWidget*)parentW->parent();
      delete parentW; // DockTabCtl

      QWidget* parentOfDockWidget = parentOfTab->parentWidget();
      if ( parentOfDockWidget == 0L ){
          if ( isV ) lastTab->show();
      } else {
        if ( parentOfDockWidget->inherits("DockSplitter") ){
          DockSplitter* split = (DockSplitter*)parentOfDockWidget;
          lastTab->applyToWidget( split );
          split->deactivate();
          if ( split->getFirst() == parentOfTab ){
            split->activate( lastTab );
            if ( ((DockWidget*)split->parent())->splitterOrientation == (int)KNewPanner::Vertical )
              emit ((DockWidget*)split->getAnother(parentOfTab))->docking( parentOfTab, DockLeft );
            else
              emit ((DockWidget*)split->getAnother(parentOfTab))->docking( parentOfTab, DockTop );
          } else {
            split->activate( 0L, lastTab );
            if ( ((DockWidget*)split->parent())->splitterOrientation == (int)KNewPanner::Vertical )
              emit ((DockWidget*)split->getAnother(parentOfTab))->docking( parentOfTab, DockRight );
            else
              emit ((DockWidget*)split->getAnother(parentOfTab))->docking( parentOfTab, DockBottom );
          }
          split->show();
        } else {
          lastTab->applyToWidget( parentOfDockWidget );
        }
        lastTab->show();
      }
      manager->blockSignals(false);
      emit manager->replaceDock( parentOfTab, lastTab );
      manager->blockSignals(true);
      delete parentOfTab;

    } else {
      setDockTabName( (DockTabCtl*)parentW );
    }
  } else {
/*********************************************************************************************/
    if ( parentW->inherits("DockSplitter") ){
      DockSplitter* parentSplitterOfDockWidget = (DockSplitter*)parentW;

      applyToWidget( 0L );
      DockWidget* secondWidget = (DockWidget*)parentSplitterOfDockWidget->getAnother( this );
      DockWidget* group        = (DockWidget*)parentSplitterOfDockWidget->parentWidget();
      group->hide();

      if ( !group->parentWidget() ){
        secondWidget->applyToWidget( 0L, group->frameGeometry().topLeft() );
        secondWidget->resize( group->width(), group->height() );
      } else {
        QWidget* obj = group->parentWidget();
        secondWidget->applyToWidget( obj );
        if ( obj->inherits("DockSplitter") ){
          DockSplitter* parentOfGroup = (DockSplitter*)obj;
          parentOfGroup->deactivate();

          if ( parentOfGroup->getFirst() == group )
            parentOfGroup->activate( secondWidget );
          else
            parentOfGroup->activate( 0L, secondWidget );
        }
      }
      delete parentSplitterOfDockWidget;
      manager->blockSignals(false);
      emit manager->replaceDock( group, secondWidget );
      manager->blockSignals(true);
      delete group;

      if ( isV ) secondWidget->show();
    }
/*********************************************************************************************/
  }
  manager->blockSignals(false);
  emit manager->change();
  manager->undockProcess = false;
}

void DockWidget::setWidget( QWidget* mw )
{
  mw->recreate(this, 0, QPoint(0,0), false);
  QApplication::syncX();
}

void DockWidget::slotCloseButtonClick()
{
  if ( stayButton->isOn() ) return;
  if ( parent() ) unDock(); else hide();
}

void DockWidget::setDockTabName( DockTabCtl* tab )
{
  QString listOfName;
  QString listOfCaption;
  for ( QWidget* w = tab->getFirstPage(); w; w = tab->getNextPage( w ) ){
    listOfCaption.append( w->caption() ).append(",");
  }
  listOfCaption.remove( listOfCaption.length()-1, 1 );

  for ( QWidget* w = tab->getFirstPage(); w; w = tab->getNextPage( w ) ){
    listOfName.append( w->name() ).append(",");
  }
  listOfName.remove( listOfName.length()-1, 1 );

  tab->parentWidget()->setName( listOfName );
  tab->parentWidget()->setCaption( listOfCaption );

  tab->parentWidget()->repaint( false ); // DockWidget->repaint
  if ( tab->parentWidget()->parent() )
    if ( tab->parentWidget()->parent()->inherits("DockSplitter") )
      ((DockSplitter*)(tab->parentWidget()->parent()))->updateName();
}
/**************************************************************************************/

DockManager::DockManager( QWidget* mainWindow , const char* name )
:QObject( 0, name )
{
  main = mainWindow;
  main->installEventFilter( this );

  undockProcess = false;

  menuData = new QList<menuDockData>;
  menuData->setAutoDelete( true );
  menuData->setAutoDelete( true );

  menu = new QPopupMenu();

  connect( menu, SIGNAL(aboutToShow()), SLOT(slotMenuPopup()) );
  connect( menu, SIGNAL(activated(int)), SLOT(slotMenuActivated(int)) );

  childDock = new QObjectList();
  childDock->setAutoDelete( false );
  mg = 0L;
  draging = false;
  dropCancel = false;
}

DockManager::~DockManager()
{
  delete menuData;
  delete menu;

  QObjectListIt it( *childDock );
  DockWidget * obj;

  while ( (obj=(DockWidget*)it.current()) ) {
    delete obj;
  }
  delete childDock;
}

void DockManager::activate()
{
  QObjectListIt it( *childDock );
  DockWidget * obj;

  while ( (obj=(DockWidget*)it.current()) ) {
    ++it;
    if ( obj->widget ) obj->widget->show();
    if ( obj->parent() && !obj->parent()->inherits("DockTabCtl") ){
        obj->show();
    }
    if ( !obj->parent() ){
      obj->show();
    }
  }
  if ( !main->inherits("QDialog") ) main->show();
}

bool DockManager::eventFilter( QObject *obj, QEvent *event )
{
  if ( obj == main && event->type() == QEvent::Resize && main->children() ){
    QWidget* fc = (QWidget*)main->children()->getFirst();
    if ( fc )
      fc->setGeometry( QRect(QPoint(0,0), main->geometry().size()) );
  }

  if ( obj->inherits("DockWidget") ){
    DockWidget* ww;
    DockWidget* curdw = (DockWidget*)obj;
    switch ( event->type() )
    {
      case QEvent::CaptionChange:
        curdw->repaint( false );
        if ( curdw->parentWidget() ){
          if ( curdw->parentWidget()->inherits("DockSplitter") ){
            ((DockSplitter*)(curdw->parentWidget()))->updateName();
          }
          if ( curdw->parentWidget()->inherits("DockTabCtl") ){
            curdw->setDockTabName( ((DockTabCtl*)(curdw->parentWidget())) );
            ((DockTabCtl*)(curdw->parentWidget()))->setPageCaption( curdw, curdw->caption() );
          }
        }
        break;
      case QEvent::MouseButtonRelease:
        if ( draging && !dropCancel ){
          draging = false;
          drop();
        }
        dropCancel = false;
        break;
      case QEvent::MouseMove:
        if ( draging ) {
          ww = findDockWidgetAt( QCursor::pos() );
          DockWidget* oldMoveWidget = currentMoveWidget;
          if ( currentMoveWidget  && ww == currentMoveWidget ) { //move
            dragMove( currentMoveWidget, currentMoveWidget->mapFromGlobal( QCursor::pos() ) );
            break;
          }

          if ( !ww && (curdw->eDocking & (int)DockDesktop) == 0 ){
              currentMoveWidget = ww;
              curPos = DockDesktop;
              mg->movePause();
          } else {
            if ( oldMoveWidget && ww != currentMoveWidget ) { //leave
              currentMoveWidget = ww;
              curPos = DockDesktop;
              mg->resize( storeW, storeH );
              mg->moveContinue();
            }
          }

          if ( oldMoveWidget != ww && ww ) { //enter ww
            currentMoveWidget = ww;
            curPos = DockDesktop;
            storeW = mg->width();
            storeH = mg->height();
            mg->movePause();
          }
        } else {
          if ( (((QMouseEvent*)event)->state() == LeftButton) &&  !dropCancel ){
            if ( curdw->eDocking != (int)DockNone ){
              dropCancel = false;
              curdw->setFocus();
              qApp->processOneEvent();
              startDrag( curdw );
            }
          }
        }
        break;
      case 6/*QEvent::KeyPress*/:
        if ( ((QKeyEvent*)event)->key() == Qt::Key_Escape ){
          if ( draging ){
            dropCancel = true;
            draging = false;
            drop();
          }
        }
        break;
      case QEvent::Hide:
      case QEvent::Show:
        emit change();
        break;
      default:
        break;
    }
  }
  return QObject::eventFilter( obj, event );
}

DockWidget* DockManager::findDockWidgetAt( const QPoint& pos )
{
  if (currentDragWidget->eDocking == (int)DockNone ) return 0L;

  QWidget* p = QApplication::widgetAt( pos );
  if ( !p ) return 0L;
  QWidget* w = 0L;
  findChildDockWidget( w, p, p->mapFromParent(pos) );
  if ( !w ){
    if ( !p->inherits("DockWidget") ) return 0L;
    w = p;
  }
  if ( qt_find_obj_child( w, "DockSplitter", "DockPaner" ) ) return 0L;
  if ( qt_find_obj_child( w, "DockTabCtl", "_dock_tab" ) ) return 0L;
  if ( childDockWidgetList->find(w) != -1 ) return 0L;
  if ( w->parent() && currentDragWidget->isGroup && w->parent()->inherits("DockTabCtl") ) return 0L;

  DockWidget* www = (DockWidget*)w;
  if ( www->sDocking == (int)DockNone ) return 0L;

  DockPosition curPos = DockDesktop;
  QPoint cpos  = www->mapFromGlobal( pos );
  QRect r = www->crect();

  int ww = r.width() / 3;
  int hh = r.height() / 3;

	if ( cpos.y() <= hh ){
    curPos = DockTop;
	} else
    if ( cpos.y() >= 2*hh ){
      curPos = DockBottom;
    } else
      if ( cpos.x() <= ww ){
        curPos = DockLeft;
      } else
        if ( cpos.x() >= 2*ww ){
          curPos = DockRight;
        } else
            curPos = DockCenter;

  if ( !(www->sDocking & (int)curPos) ) return 0L;
  if ( !(currentDragWidget->eDocking & (int)curPos) ) return 0L;
  if ( www->manager != this ) return 0L;

  return www;
}

void DockManager::findChildDockWidget( QWidget*& ww, const QWidget* p, const QPoint& pos )
{
  if ( p->children() ) {
    QWidget *w;
    QObjectListIt it( *p->children() );
    it.toLast();
    while ( it.current() ) {
      if ( it.current()->isWidgetType() ) {
        w = (QWidget*)it.current();
        if ( w->isVisible() && w->geometry().contains(pos) ) {
          if ( w->inherits("DockWidget") ) ww = w;
          findChildDockWidget( ww, w, w->mapFromParent(pos) );
          return;
        }
      }
      --it;
    }
  }
  return;
}

void DockManager::findChildDockWidget( const QWidget* p, WidgetList*& list )
{
  if ( p->children() ) {
    QWidget *w;
    QObjectListIt it( *p->children() );
    it.toLast();
    while ( it.current() ) {
      if ( it.current()->isWidgetType() ) {
        w = (QWidget*)it.current();
        if ( w->isVisible() ) {
          if ( w->inherits("DockWidget") ) list->append( w );
          findChildDockWidget( w, list );
        }
      }
      --it;
    }
  }
  return;
}

void DockManager::startDrag( DockWidget* w )
{
  if ( w->stayButton->isOn() ) return;
  currentMoveWidget = 0L;
	currentDragWidget = w;
  childDockWidgetList = new WidgetList();
  childDockWidgetList->append( w );
  findChildDockWidget( w, childDockWidgetList );

	if ( mg ) delete mg;
  mg = new DockMoveManager( w );
  curPos = DockDesktop;
  draging = true;
  mg->doMove( true, true, false);
}

void DockManager::dragMove( DockWidget* dw, QPoint pos )
{
  QRect r = dw->crect();
	QPoint p = dw->mapToGlobal( r.topLeft() );
  DockPosition oldPos = curPos;

  if ( dw->parent() )
    if ( dw->parent()->inherits("DockTabCtl") ){
      curPos = DockCenter;
    	if ( oldPos != curPos ) mg->setGeometry( p.x()+2, p.y()+2, r.width()-4, r.height()-4 );
      return;
    }

  int w = r.width() / 3;
  int h = r.height() / 3;

	if ( pos.y() <= h ){
    curPos = DockTop;
    w = r.width();
	} else
    if ( pos.y() >= 2*h ){
      curPos = DockBottom;
      p.setY( p.y() + 2*h );
      w = r.width();
    } else
      if ( pos.x() <= w ){
        curPos = DockLeft;
        h = r.height();
      } else
        if ( pos.x() >= 2*w ){
          curPos = DockRight;
          p.setX( p.x() + 2*w );
          h = r.height();
        } else
          {
            curPos = DockCenter;
            p.setX( p.x() + w );
            p.setY( p.y() + h );
          }

	if ( oldPos != curPos ) mg->setGeometry( p.x(), p.y(), w, h );
}

void DockManager::drop()
{
  mg->stop();
  delete childDockWidgetList;
  if ( dropCancel ) return;
  if ( !currentMoveWidget && ((currentDragWidget->eDocking & (int)DockDesktop) == 0) ) return;

  if ( !currentMoveWidget && !currentDragWidget->parent() )
    currentDragWidget->move( mg->x(), mg->y() );
  else
    currentDragWidget->manualDock( currentMoveWidget, curPos , 50, QPoint(mg->x(), mg->y()) );
}

void DockManager::writeConfig( KConfig* c, QString group )
{
debug("BEGIN Write Config");
  if ( !c ) c = kapp->config();
  if ( group == QString::null ) group = "dock_setting_default";

  c->setGroup( group );
  c->writeEntry( "Version", DOCK_CONFIG_VERSION );

	QStrList nameList;
	QStrList findList;
  QObjectListIt it( *childDock );
  DockWidget * obj;

	// collect DockWidget's name
	QStrList nList;
  while ( (obj=(DockWidget*)it.current()) ) {
	  ++it;
    debug("  +Add subdock %s", obj->name());
		nList.append( obj->name() );
  }

	nList.first();
  while ( nList.current() ){
    debug("  -Try to save %s", nList.current());
		obj = getDockWidgetFromName( nList.current() );
    QString cname = obj->name();
    c->writeEntry( cname+":stayButton", obj->stayButton->isOn() );
/*************************************************************************************************/
    if ( obj->isGroup ){
      if ( findList.find( obj->firstName ) != -1 && findList.find( obj->lastName ) != -1 ){

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
        c->writeEntry( cname+":orientation", obj->splitterOrientation );
        c->writeEntry( cname+":sepPos", ((DockSplitter*)obj->widget)->separatorPos() );

        nameList.append( obj->name() );
        findList.append( obj->name() );
        debug("  Save %s", nList.current());
        nList.remove();
        nList.first();
      } else {
/*************************************************************************************************/
        debug("  Skip %s", nList.current());
        if ( findList.find( obj->firstName ) == -1 )
          debug("  ? Not found %s", obj->firstName);
        if ( findList.find( obj->lastName ) == -1 )
          debug("  ? Not found %s", obj->lastName);
        nList.next();
        if ( !nList.current() ) nList.first();
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
        for ( QWidget* w = ((DockTabCtl*)obj->widget)->getFirstPage(); w; w = ((DockTabCtl*)obj->widget)->getNextPage( w ) ){
          list.append( w->name() );
        }
        c->writeEntry( cname+":tabNames", list );
        c->writeEntry( cname+":curTab", ((DockTabCtl*)obj->widget)->visiblePageId() );

        nameList.append( obj->name() );
        findList.append( obj->name() ); // not realy need !!!
        debug("  Save %s", nList.current());
        nList.remove();
        nList.first();
      } else {
/*************************************************************************************************/
        if ( !obj->parent() ){
          c->writeEntry( cname+":type", "NULL_DOCK");
          c->writeEntry( cname+":geometry", QRect(obj->frameGeometry().topLeft(), obj->size()) );
          c->writeEntry( cname+":visible", obj->isVisible());
        } else {
          c->writeEntry( cname+":type", "DOCK");
        }
        nameList.append( cname );
        debug("  Save %s", nList.current());
        findList.append( obj->name() );
        nList.remove();
        nList.first();
      }
    }
  }
  c->writeEntry( "NameList", nameList );

  c->writeEntry( "Main:Geometry", QRect(main->frameGeometry().topLeft(), main->size()) );
  c->writeEntry( "Main:visible", main->isVisible()); // curently nou use

  if ( main->inherits("DockMainWindow") ){
    DockMainWindow* dmain = (DockMainWindow*)main;
    // for DockMainWindow->setDockView() in reafConfig()
    c->writeEntry( "Main:view", dmain->getMainViewDockWidget() ? dmain->getMainViewDockWidget()->name():"" );
    c->writeEntry( "Main:dock", dmain->getMainDockWidget()     ? dmain->getMainDockWidget()->name()    :"" );
  }

  c->sync();
  debug("END Write Config");
}

void DockManager::readConfig( KConfig* c, QString group )
{
  if ( !c ) c = kapp->config();
  if ( group == QString::null ) group = "dock_setting_default";

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

  main->hide();

  QObjectListIt it( *childDock );
  DockWidget * obj;

  while ( (obj=(DockWidget*)it.current()) ){
    ++it;
    if ( !obj->isGroup && !obj->isTabGroup )
    {
      if ( obj->parent() ) obj->unDock(); else obj->hide();
    }
  }

  nameList.first();
  while ( nameList.current() ){
    QString oname = nameList.current();
    QString type = c->readEntry( oname + ":type" );
    obj = 0L;

    if ( type == "GROUP" ){
      DockWidget* first = getDockWidgetFromName( c->readEntry( oname + ":first_name" ) );
      DockWidget* last  = getDockWidgetFromName( c->readEntry( oname + ":last_name"  ) );

      int p = c->readNumEntry( oname + ":orientation" );
      if ( first  && last ){
        obj = first->manualDock( last, ( p == 0 ) ? DockLeft : DockTop );
        if (obj){
          obj->setName( oname );
          ((DockSplitter*)obj->widget)->setSeparatorPos( c->readNumEntry( oname + ":sepPos" ) );
        }
      }
    }

    if ( type == "TAB_GROUP" ){
      QStrList list;
      DockWidget* tabDockGroup = 0L;
      c->readListEntry( oname+":tabNames", list );
      DockWidget* d1 = getDockWidgetFromName( list.first() );
      list.next();
      DockWidget* d2 = getDockWidgetFromName( list.current() );
      tabDockGroup = d2->manualDock( d1, DockCenter );
      if ( tabDockGroup ){
        DockTabCtl* tab = (DockTabCtl*)tabDockGroup->widget;
        list.next();
        while ( list.current() && tabDockGroup ){
          DockWidget* tabDock = getDockWidgetFromName( list.current() );
          tabDockGroup = tabDock->manualDock( d1, DockCenter );
          list.next();
        }
        if ( tabDockGroup ){
          tabDockGroup->setName( oname );
          tab->setVisiblePage( c->readNumEntry( oname+":curTab" ) );
        }
      }
    }

    if ( type == "NULL_DOCK" || c->readEntry( oname + ":parent") == "___null___" ){
      QRect r = c->readRectEntry( oname + ":geometry" );
      obj = getDockWidgetFromName( oname );
      obj->setGeometry(r);

      if ( c->readBoolEntry( oname + ":visible" ) ){
        obj->QWidget::show();
      }
    }

    if ( type == "DOCK"  ){
      obj = getDockWidgetFromName( oname );
    }

    if ( obj ) obj->stayButton->setOn( c->readBoolEntry( oname+":stayButton", false ) );

    nameList.next();
	}

  // delete all autocreate dock
  delete autoCreateDock;
  autoCreateDock = 0L;

  if ( main->inherits("DockMainWindow") ){
    DockMainWindow* dmain = (DockMainWindow*)main;

    QString mv = c->readEntry( "Main:view" );
    if ( !mv.isEmpty() && getDockWidgetFromName( mv ) ){
      DockWidget* mvd  = getDockWidgetFromName( mv );
      mvd->applyToWidget( dmain );
      mvd->show();
      dmain->setDockView( mvd );
    }
    QString md = c->readEntry( "Main:dock" );
    if ( !md.isEmpty() && getDockWidgetFromName( md ) ){
      DockWidget* mvd  = getDockWidgetFromName( md );
      dmain->setMainDockWidget( mvd );
    }
  }

  QRect mr = c->readRectEntry("Main:Geometry");
  main->setGeometry(mr);
  if ( !main->inherits("QDialog") ) main->show();
}

DockWidget* DockManager::getDockWidgetFromName( const char* dockName )
{
  QObjectListIt it( *childDock );
  DockWidget * obj;
  while ( (obj=(DockWidget*)it.current()) ) {
    ++it;
		if ( QString(obj->name()) == QString(dockName) ) return obj;
  }

  DockWidget* autoCreate = 0L;
  if ( autoCreateDock ){
    autoCreate = new DockWidget( this, dockName, QPixmap("") );
    autoCreateDock->append( autoCreate );
  }
	return autoCreate;
}

void DockManager::slotTabShowPopup( int id, QPoint pos )
{
  curTabDockWidget = (DockWidget*)((DockTabCtl*)sender())->page(id);
  QPopupMenu menu;
  menu.insertItem( "Undock", this, SLOT(slotUndockTab()) );
  menu.insertItem( "Hide", this, SLOT(slotHideTab()) );
  menu.exec( pos );
}

void DockManager::slotUndockTab()
{
  curTabDockWidget->manualDock( 0L, DockDesktop, 50, curTabDockWidget->mapToGlobal(curTabDockWidget->frameGeometry().topLeft()));
  curTabDockWidget->show();
}

void DockManager::slotHideTab()
{
  curTabDockWidget->unDock();
}

void DockManager::slotMenuPopup()
{
  menu->clear();
  menuData->clear();

  if ( main->isVisible() ){
    menu->insertItem( "Hide toplevel window", 0 );
    menuData->append( new menuDockData( main, true ) );
  } else {
    menu->insertItem( "Show toplevel window", 0 );
    menuData->append( new menuDockData( main, false ) );
  }
  menu->insertSeparator();

  QObjectListIt it( *childDock );
  DockWidget * obj;
  int numerator = 1;
  while ( (obj=(DockWidget*)it.current()) ) {
	  ++it;
    if ( obj->mayBeHide() )
    {
      menu->insertItem( *obj->pix, QString("Hide ") + obj->caption(), numerator++ );
      menuData->append( new menuDockData( obj, true ) );
    }

    if ( obj->mayBeShow() )
    {
      menu->insertItem( *obj->pix, QString("Show ") + obj->caption(), numerator++ );
      menuData->append( new menuDockData( obj, false ) );
    }
  }
}

bool DockWidget::mayBeHide()
{
  bool f = (parent() != manager->main);
  return ( !isGroup && !isTabGroup && f && isVisible() && ( eDocking != (int)DockNone ) );
}

bool DockWidget::mayBeShow()
{
  bool f = (parent() != manager->main);
  return ( !isGroup && !isTabGroup && f && !isVisible() );
}

void DockManager::slotMenuActivated( int id )
{
  menuDockData* data = menuData->at( id );

  QWidget * obj = data->dock;
  if ( obj->inherits("DockWidget") ){
    ((DockWidget*)(data->dock))->changeHideShowState();
  } else {
    if ( data->hide ){
      obj->hide();
    } else {
      obj->show();
    }
  }
}

void DockWidget::changeHideShowState()
{
  if ( mayBeHide() ){
    slotCloseButtonClick();
    return;
  }

  if ( mayBeShow() ){
    if ( manager->main->inherits("DockMainWindow") ){
      ((DockMainWindow*)manager->main)->makeDockVisible(this);
    } else {
      makeDockVisible();
    }
  }
}

void DockWidget::makeDockVisible()
{
  if ( isVisible() ) return;

  if ( parent() && parent()->inherits("DockTabCtl") ){
      ((DockTabCtl*)parent())->setVisiblePage( this );
  }
  QWidget* p = parentWidget();
  while ( p ){
    if ( !p->isVisible() ) p->show();
    p = p->parentWidget();
  }
  show();
}

DockWidget* DockManager::findWidgetParentDock( QWidget* w )
{
  QObjectListIt it( *childDock );
  DockWidget * dock;
  DockWidget * found = 0L;

  while ( (dock=(DockWidget*)it.current()) ) {
	  ++it;
    if ( dock->widget == w ){ found  = dock; break; }
  }
  return found;
}
