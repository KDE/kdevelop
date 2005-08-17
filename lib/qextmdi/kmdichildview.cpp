//----------------------------------------------------------------------------
//    filename             : kmdichildview.cpp
//----------------------------------------------------------------------------
//    Project              : KDE MDI extension
//
//    begin                : 07/1999       by Szymon Stefanek as part of kvirc
//                                         (an IRC application)
//    changes              : 09/1999       by Falk Brettschneider to create a
//                           -06/2000      stand-alone Qt extension set of
//                                         classes and a Qt-based library
//                           2000-2003     maintained by the KDevelop project
//    patches              : 02/2000       by Massimo Morin (mmorin@schedsys.com)
//                           */2000        by Lars Beikirch (Lars.Beikirch@gmx.net)
//                           02/2001       by Eva Brucherseifer (eva@rt.e-technik.tu-darmstadt.de)
//                           01/2003       by Jens Zurheide (jens.zurheide@gmx.de)
//
//    copyright            : (C) 1999-2003 by Szymon Stefanek (stefanek@tin.it)
//                                         and
//                                         Falk Brettschneider
//    email                :  falkbr@kdevelop.org (Falk Brettschneider)
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

#include "kmdichildview.h"
#include "kmdichildview.moc"

#include <qdatetime.h>
#include <qobject.h>
//Added by qt3to4:
#include <QPixmap>
#include <QFocusEvent>
#include <QCloseEvent>
#include <QChildEvent>
#include <QKeyEvent>
#include <QEvent>
#include <QResizeEvent>

#include "kmdimainfrm.h"
#include "kmdichildfrm.h"
#include "kmdidefines.h"
#include <kdebug.h>
#include <klocale.h>
#include <qicon.h>

//============ KMdiChildView ============//

KMdiChildView::KMdiChildView( const QString& caption, QWidget* parentWidget, const char* name, Qt::WFlags f)
: QWidget(parentWidget, name, f)
  ,m_focusedChildWidget(0L)
  ,m_firstFocusableChildWidget(0L)
  ,m_lastFocusableChildWidget(0L)
  ,m_stateChanged(true)
  ,m_bToolView(false)
  ,m_bInterruptActivation(false)
  ,m_bMainframesActivateViewIsPending(false)
  ,m_bFocusInEventIsPending(false)
  ,m_trackChanges(0)
{
   setGeometry( 0, 0, 0, 0);  // reset
   if(caption != 0L) {
      m_szCaption = caption;
   }
   else {
      m_szCaption = i18n("Unnamed");
   }
   m_sTabCaption = m_szCaption;

   setFocusPolicy(Qt::ClickFocus);

   installEventFilter(this);

   // store the current time
   updateTimeStamp();
}


//============ KMdiChildView ============//

KMdiChildView::KMdiChildView( QWidget* parentWidget, const char* name, Qt::WFlags f)
: QWidget(parentWidget, name, f)
  ,m_focusedChildWidget(0L)
  ,m_firstFocusableChildWidget(0L)
  ,m_lastFocusableChildWidget(0L)
  ,m_stateChanged(true)
  ,m_bToolView(false)
  ,m_bInterruptActivation(false)
  ,m_bMainframesActivateViewIsPending(false)
  ,m_bFocusInEventIsPending(false)
{
   setGeometry( 0, 0, 0, 0);  // reset
   m_szCaption = i18n("Unnamed");
   m_sTabCaption = m_szCaption;

   setFocusPolicy(Qt::ClickFocus);

   installEventFilter(this);

   // store the current time
   updateTimeStamp();
}

//============ ~KMdiChildView ============//

KMdiChildView::~KMdiChildView()
{
  kdDebug(760)<<"~KMdiChildView()"<<endl;
}

void KMdiChildView::trackIconAndCaptionChanges(QWidget *view) {
	m_trackChanges=view;
}


//============== internal geometry ==============//

QRect KMdiChildView::internalGeometry() const
{
   if(mdiParent()) { // is attached
      // get the client area coordinates inside the MDI child frame
      QRect    posInFrame = geometry();
      // map these values to the parent of the MDI child frame
      // (this usually is the MDI child area) and return
      QPoint ptTopLeft = mdiParent()->mapToParent(posInFrame.topLeft());
      QSize sz = size();
      return QRect(ptTopLeft, sz);
   }
   else {
      QRect    geo = geometry();
      QRect    frameGeo = externalGeometry();
      return QRect(frameGeo.x(), frameGeo.y(), geo.width(), geo.height());
//      return geometry();
   }
}

//============== set internal geometry ==============//

void KMdiChildView::setInternalGeometry(const QRect& newGeometry)
{
   if(mdiParent()) { // is attached
      // retrieve the frame size
      QRect    geo      = internalGeometry();
      QRect    frameGeo = externalGeometry();
      int      nFrameSizeTop  = geo.y() - frameGeo.y();
      int      nFrameSizeLeft = geo.x() - frameGeo.x();

      // create the new geometry that is accepted by the QWidget::setGeometry() method
      QRect    newGeoQt;
      newGeoQt.setX(newGeometry.x()-nFrameSizeLeft);
      newGeoQt.setY(newGeometry.y()-nFrameSizeTop);

      newGeoQt.setWidth(newGeometry.width()+nFrameSizeLeft+KMDI_CHILDFRM_DOUBLE_BORDER/2);
      newGeoQt.setHeight(newGeometry.height()+nFrameSizeTop+KMDI_CHILDFRM_DOUBLE_BORDER/2);
//      newGeoQt.setWidth(newGeometry.width()+KMDI_MDI_CHILDFRM_DOUBLE_BORDER);
//      newGeoQt.setHeight(newGeometry.height()+mdiParent()->captionHeight()+KMDI_MDI_CHILDFRM_DOUBLE_BORDER);

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

QRect KMdiChildView::externalGeometry() const
{
   return mdiParent() ? mdiParent()->frameGeometry() : frameGeometry();
}

//============== set external geometry ==============//

void KMdiChildView::setExternalGeometry(const QRect& newGeometry)
{
   if(mdiParent()) { // is attached
       mdiParent()->setGeometry(newGeometry);
   }
   else {
      // retrieve the frame size
      QRect    geo      = internalGeometry();
      QRect    frameGeo = externalGeometry();
      int      nTotalFrameWidth = frameGeo.width() - geo.width();
      int      nTotalFrameHeight = frameGeo.height() - geo.height();
      int      nFrameSizeTop  = geo.y() - frameGeo.y();
      int      nFrameSizeLeft = geo.x() - frameGeo.x();

      // create the new geometry that is accepted by the QWidget::setGeometry() method
      // not attached => the window system makes the frame
      QRect    newGeoQt;
      newGeoQt.setX(newGeometry.x()+nFrameSizeLeft);
      newGeoQt.setY(newGeometry.y()+nFrameSizeTop);
      newGeoQt.setWidth(newGeometry.width()-nTotalFrameWidth);
      newGeoQt.setHeight(newGeometry.height()-nTotalFrameHeight);

      // set the geometry
      setGeometry(newGeoQt);
   }
}

//============== minimize ==============//

void KMdiChildView::minimize(bool bAnimate)
{
   if(mdiParent()) {
      if(!isMinimized()) {
         mdiParent()->setState(KMdiChildFrm::Minimized,bAnimate);
      }
   }
   else {
      showMinimized();
   }
}

void KMdiChildView::showMinimized()
{
   //qDebug("is minimized now");
   emit isMinimizedNow();
   QWidget::showMinimized();
}

//slot:
void KMdiChildView::minimize() { minimize(true); }

//============= maximize ==============//

void KMdiChildView::maximize(bool bAnimate)
{
   if(mdiParent()) {
      if(!isMaximized()) {
         mdiParent()->setState(KMdiChildFrm::Maximized,bAnimate);
         emit mdiParentNowMaximized(true);
      }
   }
   else {
      showMaximized();
   }
}

void KMdiChildView::showMaximized()
{
   //qDebug("is maximized now");
   emit isMaximizedNow();
   QWidget::showMaximized();
}

//slot:
void KMdiChildView::maximize() { maximize(true); }

//============== restoreGeometry ================//

QRect KMdiChildView::restoreGeometry()
{
   if(mdiParent()) {
      return mdiParent()->restoreGeometry();
   }
   else {
      // XXX not really supported, may be we must use Windows or X11 funtions
      return geometry();
   }
}

//============== setRestoreGeometry ================//

void  KMdiChildView::setRestoreGeometry(const QRect& newRestGeo)
{
   if(mdiParent()) {
      mdiParent()->setRestoreGeometry(newRestGeo);
   }
   else {
      // XXX not supported, may be we must use Windows or X11 funtions
   }
}

//============== attach ================//

void KMdiChildView::attach()
{
   emit attachWindow(this,true);
}

//============== detach =================//

void KMdiChildView::detach()
{
   emit detachWindow(this, true);
}

//=============== isMinimized ? =================//

bool KMdiChildView::isMinimized() const
{
   if(mdiParent()) {
      return (mdiParent()->state() == KMdiChildFrm::Minimized);
   }
   else {
      return QWidget::isMinimized();
   }
}

//============== isMaximized ? ==================//

bool KMdiChildView::isMaximized() const
{
   if(mdiParent()) {
      return (mdiParent()->state() == KMdiChildFrm::Maximized);
   }
   else {
      return QWidget::isMaximized();
   }
}

//============== restore ================//

void KMdiChildView::restore()
{
   if(mdiParent()) {
      if(isMaximized()) {
         emit mdiParentNowMaximized(false);
      }
      if(isMinimized()||isMaximized()) {
         mdiParent()->setState(KMdiChildFrm::Normal);
      }
   }
   else {
      showNormal();
   }
}

void KMdiChildView::showNormal()
{
   //qDebug("is restored now");
   emit isRestoredNow();
   QWidget::showNormal();
}

//=============== youAreAttached ============//

void KMdiChildView::youAreAttached(KMdiChildFrm *lpC)
{
   lpC->setCaption(m_szCaption);

   emit isAttachedNow();
}

//================ youAreDetached =============//

void KMdiChildView::youAreDetached()
{
   setCaption(m_szCaption);

   setTabCaption(m_sTabCaption);
   if(myIconPtr())setIcon(*(myIconPtr()));
   setFocusPolicy(Qt::StrongFocus);

   emit isDetachedNow();
}

//================ setCaption ================//
// this set the caption of only the window
void KMdiChildView::setCaption(const QString& szCaption)
{
  // this will work only for window
   m_szCaption=szCaption;
   if(mdiParent()) {
     mdiParent()->setCaption(m_szCaption);
   }
   else {
     //  sorry have to call the parent one
     QWidget::setCaption(m_szCaption);
   }
   emit windowCaptionChanged(m_szCaption);
}

//============== closeEvent ================//

void KMdiChildView::closeEvent(QCloseEvent *e)
{
   e->ignore(); //we ignore the event , and then close later if needed.
   emit childWindowCloseRequest(this);
}

//================ myIconPtr =================//

QPixmap* KMdiChildView::myIconPtr()
{
   return 0;
}

//============= focusInEvent ===============//

void KMdiChildView::focusInEvent(QFocusEvent *e)
{
   QWidget::focusInEvent(e);

   // every widget get a focusInEvent when a popup menu is opened!?! -> maybe bug of QT
   if(e && ((e->reason())==QFocusEvent::Popup)) {
      return;
   }

   m_bFocusInEventIsPending = true;
   activate();
   m_bFocusInEventIsPending = false;

   emit gotFocus(this);
}

//============= activate ===============//

void KMdiChildView::activate()
{
   // avoid circularity
   static bool s_bActivateIsPending = false;
   if(s_bActivateIsPending) {
      return;
   }
   s_bActivateIsPending = true;

   // raise the view and push the taskbar button
   if(!m_bMainframesActivateViewIsPending) {
     emit focusInEventOccurs( this);
   }

   // if this method was called directly, check if the mainframe wants that we interrupt
   if(m_bInterruptActivation) {
      m_bInterruptActivation = false;
   }
   else {
      if(!m_bFocusInEventIsPending) {
         setFocus();
      }
      kdDebug(760) << "KMdiChildView::activate() called!" << endl;
      emit activated(this);
   }

   if(m_focusedChildWidget != 0L) {
      m_focusedChildWidget->setFocus();
   }
   else {
      if(m_firstFocusableChildWidget != 0L) {
         m_firstFocusableChildWidget->setFocus();
         m_focusedChildWidget = m_firstFocusableChildWidget;
      }
   }
   s_bActivateIsPending = false;
}

//============= focusOutEvent ===============//

void KMdiChildView::focusOutEvent(QFocusEvent* e)
{
   QWidget::focusOutEvent(e);

   emit lostFocus( this);
}

//============= resizeEvent ===============//

void KMdiChildView::resizeEvent(QResizeEvent* e)
{
   QWidget::resizeEvent( e);

   if(m_stateChanged) {
      m_stateChanged = false;
      if(isMaximized()) {
         //qDebug("is maximized now");
         emit isMaximizedNow();
      }
      else if(isMinimized()) {
         //qDebug("is minimized now");
         emit isMinimizedNow();
      }
      else {   // is restored
         //qDebug("is restored now");
         emit isRestoredNow();
      }
   }
}

void KMdiChildView::slot_childDestroyed()
{
   // do what we do if a child is removed

   // if we lost a child we uninstall ourself as event filter for the lost
   // child and its children
   const QObject* pLostChild = QObject::sender();
   if ((pLostChild != 0L) && (pLostChild->isWidgetType())) {
      QObjectList *list = ((QObject*)(pLostChild))->queryList("QWidget");
      list->insert(0, pLostChild);        // add the lost child to the list too, just to save code
      QObjectListIt it( *list );          // iterate over all lost child widgets
      QObject * obj;
      while ( (obj=it.current()) != 0 ) { // for each found object...
         QWidget* widg = (QWidget*)obj;
         ++it;
         widg->removeEventFilter(this);
         if(m_firstFocusableChildWidget == widg) {
            m_firstFocusableChildWidget = 0L;   // reset first widget
         }
         if(m_lastFocusableChildWidget == widg) {
            m_lastFocusableChildWidget = 0L;    // reset last widget
         }
         if(m_focusedChildWidget == widg) {
            m_focusedChildWidget = 0L;          // reset focused widget
         }
      }
      delete list;                        // delete the list, not the objects
   }
}

//============= eventFilter ===============//
bool KMdiChildView::eventFilter(QObject *obj, QEvent *e )
{
   if(e->type() == QEvent::KeyPress && isAttached()) {
      QKeyEvent* ke = (QKeyEvent*) e;
      if(ke->key() == Qt::Key_Tab) {
         //qDebug("ChildView %i::eventFilter - TAB from %s (%s)", this, obj->name(), obj->className());
         QWidget* w = (QWidget*) obj;
         if((w->focusPolicy() == Qt::StrongFocus) ||
            (w->focusPolicy() == Qt::TabFocus   ) ||
            (w->focusPolicy() == Qt::WheelFocus ))
         {
            //qDebug("  accept TAB as setFocus change");
            if(m_lastFocusableChildWidget != 0) {
               if(w == m_lastFocusableChildWidget) {
                  if(w != m_firstFocusableChildWidget) {
                     //qDebug("  TAB: setFocus to first");
                     m_firstFocusableChildWidget->setFocus();
                     //qDebug("  TAB: focus is set to first");
                  }
               }
            }
         }
      }
   }
   else if(e->type() == QEvent::FocusIn) {
      if(obj->isWidgetType()) {
         QObjectList *list = queryList( "QWidget" );
         if(list->find(obj) != -1) {
            m_focusedChildWidget = (QWidget*)obj;
         }
         delete list;   // delete the list, not the objects
      }
      if (!isAttached()) {   // is toplevel, for attached views activation is done by main frame event filter
         static bool m_bActivationIsPending = false;
         if(!m_bActivationIsPending) {
            m_bActivationIsPending = true;
            activate(); // sets the focus
            m_bActivationIsPending = false;
         }
      }
   }
   else if (e->type() == QEvent::ChildRemoved) {
      // if we lost a child we uninstall ourself as event filter for the lost
      // child and its children
      QObject* pLostChild = ((QChildEvent*)e)->child();
      if ((pLostChild != 0L) && (pLostChild->isWidgetType())) {
         QObjectList *list = pLostChild->queryList( "QWidget" );
         list->insert(0, pLostChild);        // add the lost child to the list too, just to save code
         QObjectListIt it( *list );          // iterate over all lost child widgets
         QObject * o;
         while ( (o=it.current()) != 0 ) { // for each found object...
            QWidget* widg = (QWidget*)o;
            ++it;
            widg->removeEventFilter(this);
            if((widg->focusPolicy() == Qt::StrongFocus) ||
               (widg->focusPolicy() == Qt::TabFocus   ) ||
               (widg->focusPolicy() == Qt::WheelFocus ))
            {
               if(m_firstFocusableChildWidget == widg) {
                  m_firstFocusableChildWidget = 0L;   // reset first widget
               }
               if(m_lastFocusableChildWidget == widg) {
                  m_lastFocusableChildWidget = 0L;    // reset last widget
               }
            }
         }
         delete list;                        // delete the list, not the objects
      }
   }
   else if (e->type() == QEvent::ChildAdded) {
      // if we got a new child and we are attached to the MDI system we
      // install ourself as event filter for the new child and its children
      // (as we did when we were added to the MDI system).
      QObject* pNewChild = ((QChildEvent*)e)->child();
      if ((pNewChild != 0L) && (pNewChild->isWidgetType()))
      {
         QWidget* pNewWidget = (QWidget*)pNewChild;
         if (pNewWidget->testWFlags(Qt::WType_Dialog | Qt::WShowModal))
             return false;
         QObjectList *list = pNewWidget->queryList( "QWidget" );
         list->insert(0, pNewChild);         // add the new child to the list too, just to save code
         QObjectListIt it( *list );          // iterate over all new child widgets
         QObject * o;
         while ( (o=it.current()) != 0 ) { // for each found object...
            QWidget* widg = (QWidget*)o;
            ++it;
            widg->installEventFilter(this);
            connect(widg, SIGNAL(destroyed()), this, SLOT(slot_childDestroyed()));
            if((widg->focusPolicy() == Qt::StrongFocus) ||
               (widg->focusPolicy() == Qt::TabFocus   ) ||
               (widg->focusPolicy() == Qt::WheelFocus ))
            {
               if(m_firstFocusableChildWidget == 0) {
                  m_firstFocusableChildWidget = widg;  // first widge
               }
               m_lastFocusableChildWidget = widg; // last widget
               //qDebug("*** %s (%s)",widg->name(),widg->className());
            }
         }
         delete list;                        // delete the list, not the objects
      }
   }
   else
   {
       if (e->type()==QEvent::WindowIconChange) {
//            qDebug("KMDiChildView:: QEvent:IconChange intercepted\n");
          if  (obj==this)
             iconUpdated(this,icon()?(*icon()):QPixmap());
          else if (obj==m_trackChanges)
             setIcon(m_trackChanges->icon()?(*(m_trackChanges->icon())):QPixmap());
       }
       if (e->type()==QEvent::WindowTitleChange) {
          if (obj==this)
             captionUpdated(this,caption());
       }
   }

   return false;                           // standard event processing
}

/** Switches interposing in event loop of all current child widgets off. */
void KMdiChildView::removeEventFilterForAllChildren()
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

QWidget* KMdiChildView::focusedChildWidget()
{
   return m_focusedChildWidget;
}

void KMdiChildView::setFirstFocusableChildWidget(QWidget* firstFocusableChildWidget)
{
   m_firstFocusableChildWidget = firstFocusableChildWidget;
}

void KMdiChildView::setLastFocusableChildWidget(QWidget* lastFocusableChildWidget)
{
   m_lastFocusableChildWidget = lastFocusableChildWidget;
}
/** Set a new value of  the task bar button caption  */
void KMdiChildView::setTabCaption (const QString& stbCaption) {

  m_sTabCaption = stbCaption;
  emit tabCaptionChanged(m_sTabCaption);

}
void KMdiChildView::setMDICaption (const QString& caption) {
  setCaption(caption);
  setTabCaption(caption);
}

/** sets an ID  */
void KMdiChildView::setWindowMenuID( int id)
{
   m_windowMenuID = id;
}

//============= slot_clickedInWindowMenu ===============//

/** called if someone click on the "Window" menu item for this child frame window */
void KMdiChildView::slot_clickedInWindowMenu()
{
   updateTimeStamp();
   emit clickedInWindowMenu( m_windowMenuID);
}

//============= slot_clickedInDockMenu ===============//

/** called if someone click on the "Dock/Undock..." menu item for this child frame window */
void KMdiChildView::slot_clickedInDockMenu()
{
   emit clickedInDockMenu( m_windowMenuID);
}

//============= setMinimumSize ===============//

void KMdiChildView::setMinimumSize( int minw, int minh)
{
   QWidget::setMinimumSize( minw, minh);
   if ( (mdiParent() != 0L) && (mdiParent()->state() != KMdiChildFrm::Minimized) ) {
      mdiParent()->setMinimumSize( minw + KMDI_CHILDFRM_DOUBLE_BORDER,
                                   minh + KMDI_CHILDFRM_DOUBLE_BORDER
                                        + KMDI_CHILDFRM_SEPARATOR
                                        + mdiParent()->captionHeight());
   }
}

//============= setMaximumSize ===============//

void KMdiChildView::setMaximumSize( int maxw, int maxh)
{
   if ( (mdiParent() != 0L) && (mdiParent()->state() == KMdiChildFrm::Normal) ) {
      int w = maxw + KMDI_CHILDFRM_DOUBLE_BORDER;
      if(w > QWIDGETSIZE_MAX) { w = QWIDGETSIZE_MAX; }
      int h = maxh + KMDI_CHILDFRM_DOUBLE_BORDER + KMDI_CHILDFRM_SEPARATOR + mdiParent()->captionHeight();
      if(h > QWIDGETSIZE_MAX) { h = QWIDGETSIZE_MAX; }
      mdiParent()->setMaximumSize( w, h);
   }
   QWidget::setMaximumSize( maxw, maxh);
}

//============= show ===============//

void KMdiChildView::show()
{
   QWidget* pParent = mdiParent();
   if(pParent != 0L) {
      pParent->show();
   }
   QWidget::show();
}

//============= hide ===============//

void KMdiChildView::hide()
{
   if(mdiParent() != 0L) {
      mdiParent()->hide();
   }
   QWidget::hide();
}

//============= raise ===============//

void KMdiChildView::raise()
{
   if(mdiParent() != 0L) {
      mdiParent()->raise();
      // XXXCTL what's about Z-order? L.B. 2002/02/10
   }
   QWidget::raise();
}

// kate: space-indent on; indent-width 2; replace-tabs on;
