//----------------------------------------------------------------------------
//    filename             : qextmdichildview.cpp
//----------------------------------------------------------------------------
//    Project              : Qt MDI extension
//
//    begin                : 07/1999       by Szymon Stefanek as part of kvirc
//                                         (an IRC application)
//    changes              : 09/1999       by Falk Brettschneider to create a
//                           -06/2000      stand-alone Qt extension set of
//                                         classes and a Qt-based library
//    patches              : 02/2000       by Massimo Morin (mmorin@schedsys.com)
//                           */2000        by Lars Beikirch (Lars.Beikirch@gmx.net)
//
//    copyright            : (C) 1999-2000 by Szymon Stefanek (stefanek@tin.it)
//                                         and
//                                         Falk Brettschneider
//    email                :  gigafalk@yahoo.com (Falk Brettschneider)
//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
//
//    This program is free software; you can redistribute it and/or modify
//    it under the terms of the GNU Library General Public License as
//    published by the Free Software Foundation; either version 2 of the
//    License, or (at your option) any later version.
//
//----------------------------------------------------------------------------

#include <qdatetime.h>
#include <qobjectlist.h>

#include "qextmdichildview.h"
#include "qextmdimainfrm.h"
#include "qextmdichildfrm.h"
#include "qextmdidefines.h"

//============ QextMdiChildView ============//

QextMdiChildView::QextMdiChildView( const QString& caption, QWidget* parentWidget, const char* name, WFlags f)
: QWidget(parentWidget, name, f)
  ,m_focusedChildWidget(0L)
  ,m_firstFocusableChildWidget(0L)
  ,m_lastFocusableChildWidget(0L)
  ,m_stateChanged(true)
  ,m_bToolView(false)
{
   setGeometry( 0, 0, 0, 0);  // reset
   if( caption) 
      m_szCaption = caption;
   else
      m_szCaption = QString(tr("Unnamed"));

   // mmorin
   m_sTabCaption = m_szCaption;

   setFocusPolicy(ClickFocus);
}

//============ ~QextMdiChildView ============//

QextMdiChildView::~QextMdiChildView()
{
}

//============== internal geometry ==============//

QRect QextMdiChildView::internalGeometry()
{
   if (mdiParent()) {
      // get the client area coordinates inside the MDI child frame
      QRect    posInFrame = geometry();
      // map these values to the parent of the MDI child frame 
      // (this usually is the MDI child area) and return
      QPoint   ptTopLeft     = mdiParent()->mapToParent(posInFrame.topLeft());
      QPoint   ptBottomRight = mdiParent()->mapToParent(posInFrame.bottomRight());
      return QRect(ptTopLeft, ptBottomRight);
   }
   else {
      return geometry();
   }
}

//============== set internal geometry ==============//

void QextMdiChildView::setInternalGeometry(const QRect& newGeometry)
{
   if (mdiParent()) {
      // retrieve the frame size
      QRect    geo      = internalGeometry();
      QRect    frameGeo = externalGeometry();
      int      nFrameSizeTop  = geo.y() - frameGeo.y();
      int      nFrameSizeLeft = geo.x() - frameGeo.x();

      // create the new geometry that is accepted by the QWidget::setGeometry() method
      QRect    newGeoQt;
      newGeoQt.setX(newGeometry.x()-nFrameSizeLeft);
      newGeoQt.setY(newGeometry.y()-nFrameSizeTop);
      newGeoQt.setWidth(newGeometry.width()+frameGeo.width()-geo.width());
      newGeoQt.setHeight(newGeometry.height()+frameGeo.height()-geo.height());

      // set the geometry
      mdiParent()->setGeometry(newGeoQt);
   }
   else {
      // retrieve the frame size
      QRect    geo      = internalGeometry();
      QRect    frameGeo = externalGeometry();
      int      nFrameSizeTop  = geo.y() - frameGeo.y();
      int      nFrameSizeLeft = geo.x() - frameGeo.x();

      // create the new geometry that is accepted by the QWidget::setGeometry() method
      QRect    newGeoQt;
      newGeoQt.setX(newGeometry.x()-nFrameSizeLeft);
      newGeoQt.setY(newGeometry.y()-nFrameSizeTop);
      newGeoQt.setWidth(newGeometry.width());
      newGeoQt.setHeight(newGeometry.height());

      // set the geometry
      setGeometry(newGeoQt);
   }
}

//============== external geometry ==============//

QRect QextMdiChildView::externalGeometry()
{
   return mdiParent() ? mdiParent()->frameGeometry() : frameGeometry();
}

//============== set external geometry ==============//

void QextMdiChildView::setExternalGeometry(const QRect& newGeometry)
{
   // retrieve the frame size
   QRect    geo      = internalGeometry();
   QRect    frameGeo = externalGeometry();
   int      nTotalFrameWidth = frameGeo.width() - geo.width();
   int      nTotalFrameHeight = frameGeo.height() - geo.height();

   // create the new geometry that is accepted by the QWidget::setGeometry() method
   QRect    newGeoQt;
   newGeoQt.setX(newGeometry.x());
   newGeoQt.setY(newGeometry.y());
   newGeoQt.setWidth(newGeometry.width()-nTotalFrameWidth);
   newGeoQt.setHeight(newGeometry.height()-nTotalFrameHeight);

   // set the geometry
   mdiParent() ? mdiParent()->setGeometry(newGeoQt) : setGeometry(newGeoQt);
}

//============== minimize ==============//

void QextMdiChildView::minimize(bool bAnimate)
{
   if(mdiParent()){
      if(!isMinimized()){
         mdiParent()->setState(QextMdiChildFrm::Minimized,bAnimate);
      }
   } else showMinimized();
}

void QextMdiChildView::showMinimized()
{
   qDebug("is minimized now");
   emit isMinimizedNow();
   QWidget::showMinimized();
}

void QextMdiChildView::minimize(){ minimize(TRUE); }

//============= maximize ==============//

void QextMdiChildView::maximize(bool bAnimate)
{
   if(mdiParent()){
      if(!isMaximized()){
         mdiParent()->setState(QextMdiChildFrm::Maximized,bAnimate);
         emit mdiParentNowMaximized();
      }
   } else showMaximized();
}

void QextMdiChildView::showMaximized()
{
   qDebug("is maximized now");
   emit isMaximizedNow();
   QWidget::showMaximized();
}

void QextMdiChildView::maximize(){ maximize(TRUE); }

//============== attach ================//

void QextMdiChildView::attach()
{
   emit attachWindow(this,TRUE);
}

//============== detach =================//

void QextMdiChildView::detach()
{
   emit detachWindow(this, TRUE);
}

//=============== isMinimized ? =================//

bool QextMdiChildView::isMinimized()
{
   if(mdiParent())return (mdiParent()->state() == QextMdiChildFrm::Minimized);
   else return QWidget::isMinimized();
}

//============== isMaximized ? ==================//

bool QextMdiChildView::isMaximized()
{
   if(mdiParent())return (mdiParent()->state() == QextMdiChildFrm::Maximized);
   if( size() == maximumSize()) return TRUE;
   else return FALSE;
}

//============== restore ================//

void QextMdiChildView::restore()
{
   if(mdiParent()){
      if(isMaximized())
         emit mdiParentNoLongerMaximized(mdiParent());
      if(isMinimized()||isMaximized())mdiParent()->setState(QextMdiChildFrm::Normal);
   } else showNormal();
}

void QextMdiChildView::showNormal()
{
   qDebug("is restored now");
   emit isRestoredNow();
   QWidget::showNormal();
}

//=============== youAreAttached ============//

void QextMdiChildView::youAreAttached(QextMdiChildFrm *lpC)
{
   lpC->setCaption(m_szCaption);
   //F.B. lpC->setIconPointer(myIconPtr()); //It is ok to set a NULL icon too here
}

//================ youAreDetached =============//

void QextMdiChildView::youAreDetached()
{
   setCaption(m_szCaption);
   
   setTabCaption(m_sTabCaption);
   if(myIconPtr())setIcon(*(myIconPtr()));
   setFocusPolicy(QWidget::StrongFocus);
}

//================ setCaption ================//
// this set the caption of only the window
void QextMdiChildView::setCaption(const QString& szCaption)
{
  // this will work only for window
   m_szCaption=szCaption;
   if (mdiParent()) {
     mdiParent()->setCaption(m_szCaption);
   } else {
     //  sorry have to call the parent one
     QWidget::setCaption(m_szCaption);
   }
   emit windowCaptionChanged(m_szCaption);
}

//============== closeEvent ================//

void QextMdiChildView::closeEvent(QCloseEvent *e)
{
   e->ignore(); //we ignore the event , and then close later if needed.
   emit childWindowCloseRequest(this);
}

//================ myIconPtr =================//

QPixmap * QextMdiChildView::myIconPtr()
{
   return 0;
}

//============= focusInEvent ===============//

void QextMdiChildView::focusInEvent(QFocusEvent *)
{
   // XXX TODO: call QWidget::focusInEvent() ???

   //qDebug("ChildView::focusInEvent");
   emit focusInEventOccurs( this);

   if( m_focusedChildWidget != 0L) {
      //qDebug("ChildView::focusInEvent 2");
      m_focusedChildWidget->setFocus();
   }
   else {
      if( m_firstFocusableChildWidget != 0L) {
         //qDebug("ChildView::focusInEvent 3");
         m_firstFocusableChildWidget->setFocus();
         m_focusedChildWidget = m_firstFocusableChildWidget;
      }
   }

   emit gotFocus(this);
}

//============= focusOutEvent ===============//

void QextMdiChildView::focusOutEvent(QFocusEvent* e)
{
   QWidget::focusOutEvent(e);

   emit lostFocus( this);
}

//============= resizeEvent ===============//

void QextMdiChildView::resizeEvent(QResizeEvent* e)
{
   QWidget::resizeEvent( e);

   if( m_stateChanged) {
      m_stateChanged = false;
      if( isMaximized()) {
         qDebug("is maximized now");
         emit isMaximizedNow();
      }
      else if( isMinimized()) {
         qDebug("is minimized now");
         emit isMinimizedNow();
      }
      else {   // is restored
         qDebug("is restored now");
         emit isRestoredNow();
      }
   }
}

//============= eventFilter ===============//

bool QextMdiChildView::eventFilter(QObject *obj, QEvent *e )
{
   if( e->type() == QEvent::KeyPress) {
      QKeyEvent* ke = (QKeyEvent*) e;
      if( ke->key() == Qt::Key_Tab) {
         //qDebug("ChildView %i::eventFilter - TAB from %s (%s)", this, obj->name(), obj->className());
         QWidget* w = (QWidget*) obj;
         if( (w->focusPolicy() == QWidget::StrongFocus) || (w->focusPolicy() == QWidget::TabFocus)) {
            //qDebug("  accept TAB as setFocus change");
            if( m_lastFocusableChildWidget != 0) {
               if( w == m_lastFocusableChildWidget) {
                  if( w != m_firstFocusableChildWidget) {
                     //qDebug("  TAB: setFocus to first");
                     m_firstFocusableChildWidget->setFocus();
                     //qDebug("  TAB: focus is set to first");
                  }
                  return TRUE;
                }
            }
         }
         else {
            if( w->focusPolicy() == QWidget::WheelFocus) {
               //qDebug("  accept TAB as setFocus change");
               if( m_lastFocusableChildWidget != 0) {
                  if( w == m_lastFocusableChildWidget) {
                     if( w != m_firstFocusableChildWidget) {
                        //qDebug("  TAB: setFocus to first");
                        m_firstFocusableChildWidget->setFocus();
                        //qDebug("  TAB: focus is set to first");
                     }
                     return TRUE;
                   }
               }
            }
         }
      }
   }
   else if( e->type() == QEvent::FocusIn) {
      if (obj->inherits("QWidget")) {
         QObjectList *list = queryList( "QWidget" );
         if (list->find(obj) != -1) {
            m_focusedChildWidget = (QWidget*)obj;
         }
      }
   }
   return FALSE;                           // standard event processing
}

/** Interpose in event loop of all current child widgets. Must be recalled after dynamic adding of new child widgets!
  * and
  * get first and last TAB-focusable widget of this child frame
  */
void QextMdiChildView::installEventFilterForAllChildren()
{
   QObjectList *list = queryList( "QWidget" );
   QObjectListIt it( *list );          // iterate over all child widgets
   QObject * obj;
   while ( (obj=it.current()) != 0 ) { // for each found object...
      QWidget* widg = (QWidget*)obj;
      ++it;
      widg->installEventFilter(this);
      if( (widg->focusPolicy() == QWidget::StrongFocus) || (widg->focusPolicy() == QWidget::TabFocus)) {
         if( m_firstFocusableChildWidget == 0)
            m_firstFocusableChildWidget = widg;  // first widget
         m_lastFocusableChildWidget = widg; // last widget
         //qDebug("*** %s (%s)",widg->name(),widg->className());
      }
      else {
         if( widg->focusPolicy() == QWidget::WheelFocus) {
            if( m_firstFocusableChildWidget == 0)
               m_firstFocusableChildWidget = widg;  // first widget
            m_lastFocusableChildWidget = widg; // last widget
            //qDebug("*** %s (%s)",widg->name(),widg->className());
         }
      }
   }
   //qDebug("### |%s|", m_lastFocusableChildWidget->name());
   if( m_lastFocusableChildWidget != 0) {
      if( QString(m_lastFocusableChildWidget->name()) == QString("qt_viewport")) {
         // bad Qt hack :-( to avoid setting a listbox viewport as last focusable widget
         it.toFirst();
         // search widget
         while( (obj=it.current()) != m_lastFocusableChildWidget) ++it;
         --it;
         --it;
         --it;// three steps back
         m_lastFocusableChildWidget = (QWidget*) it.current();
         //qDebug("Qt hack");
      }
   }
   //qDebug("### |%s|", m_lastFocusableChildWidget->name());
   delete list;                        // delete the list, not the objects
}

/** Switches interposing in event loop of all current child widgets off. */
void QextMdiChildView::removeEventFilterForAllChildren()
{
   QObjectList *list = queryList( "QWidget" );
   QObjectListIt it( *list );          // iterate over all child widgets
   QObject * obj;
   while ( (obj=it.current()) != 0 ) { // for each found object...
      QWidget* widg = (QWidget*)obj;
      ++it;
      widg->removeEventFilter(this);
   }
   delete list;                        // delete the list, not the objects
}

QWidget* QextMdiChildView::focusedChildWidget()
{
   return m_focusedChildWidget;
}

void QextMdiChildView::setFirstFocusableChildWidget(QWidget* firstFocusableChildWidget)
{
   m_firstFocusableChildWidget = firstFocusableChildWidget;
}

void QextMdiChildView::setLastFocusableChildWidget(QWidget* lastFocusableChildWidget)
{
   m_lastFocusableChildWidget = lastFocusableChildWidget;
}
/** Set a new value of  the task bar button caption  */
void QextMdiChildView::setTabCaption (const QString& stbCaption){

  m_sTabCaption = stbCaption;
  emit tabCaptionChanged(m_sTabCaption);

}
void QextMdiChildView::setMDICaption (const QString& caption) {
  setCaption(caption);
  setTabCaption(caption);
}

/** sets an ID  */
void QextMdiChildView::setWindowMenuID( int id)
{
   m_windowMenuID = id;
}

//============= slot_clickedInWindowMenu ===============//

/** called if someone click on the "Window" menu item for this child frame window */
void QextMdiChildView::slot_clickedInWindowMenu()
{
   emit clickedInWindowMenu( m_windowMenuID);
}

//============= slot_clickedInDockMenu ===============//

/** called if someone click on the "Dock/Undock..." menu item for this child frame window */
void QextMdiChildView::slot_clickedInDockMenu()
{
   emit clickedInDockMenu( m_windowMenuID);
}

//============= setMinimumSize ===============//

void QextMdiChildView::setMinimumSize( int minw, int minh)
{
   QWidget::setMinimumSize( minw, minh);
   if( mdiParent() != 0L)
      mdiParent()->setMinimumSize( minw + QEXTMDI_MDI_CHILDFRM_DOUBLE_BORDER,
                                   minh + QEXTMDI_MDI_CHILDFRM_DOUBLE_BORDER
                                        + QEXTMDI_MDI_CHILDFRM_SEPARATOR
                                        + mdiParent()->captionHeight());
}

//============= setMaximumSize ===============//

void QextMdiChildView::setMaximumSize( int maxw, int maxh)
{
   if( mdiParent() != 0L) {
      int w = maxw + QEXTMDI_MDI_CHILDFRM_DOUBLE_BORDER;
      if( w > QWIDGETSIZE_MAX) w = QWIDGETSIZE_MAX;
      int h = maxh + QEXTMDI_MDI_CHILDFRM_DOUBLE_BORDER + QEXTMDI_MDI_CHILDFRM_SEPARATOR + mdiParent()->captionHeight();
      if( h > QWIDGETSIZE_MAX) h = QWIDGETSIZE_MAX;
      mdiParent()->setMaximumSize( w, h);
   }
   QWidget::setMaximumSize( maxw, maxh);
}

//============= show ===============//

void QextMdiChildView::show()
{
   if( mdiParent() != 0L)
      mdiParent()->show();
   QWidget::show();
}
