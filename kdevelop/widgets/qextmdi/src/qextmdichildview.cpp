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
//                           02/2001       by Eva Brucherseifer (eva@rt.e-technik.tu-darmstadt.de)
//
//    copyright            : (C) 1999-2001 by Szymon Stefanek (stefanek@tin.it)
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
  ,m_stateChanged(TRUE)
  ,m_bToolView(FALSE)
  ,m_bInterruptActivation(FALSE)
  ,m_bMainframesActivateViewIsPending(FALSE)
  ,m_bFocusInEventIsPending(FALSE)
{
   setGeometry( 0, 0, 0, 0);  // reset
   if(caption) {
      m_szCaption = caption;
   }
   else {
      m_szCaption = QString(tr("Unnamed"));
   }
   m_sTabCaption = m_szCaption;

   setFocusPolicy(ClickFocus);
}

//============ QextMdiChildView ============//

QextMdiChildView::QextMdiChildView( QWidget* parentWidget, const char* name, WFlags f)
: QWidget(parentWidget, name, f)
  ,m_focusedChildWidget(0L)
  ,m_firstFocusableChildWidget(0L)
  ,m_lastFocusableChildWidget(0L)
  ,m_stateChanged(TRUE)
  ,m_bToolView(FALSE)
{
   setGeometry( 0, 0, 0, 0);  // reset
   m_szCaption = QString(tr("Unnamed"));
   m_sTabCaption = m_szCaption;

   setFocusPolicy(ClickFocus);
}

//============ ~QextMdiChildView ============//

QextMdiChildView::~QextMdiChildView()
{
}

//============== internal geometry ==============//

QRect QextMdiChildView::internalGeometry() const
{
   if(mdiParent()) {
      // get the client area coordinates inside the MDI child frame
      QRect    posInFrame = geometry();
      // map these values to the parent of the MDI child frame
      // (this usually is the MDI child area) and return
      QPoint ptTopLeft = mdiParent()->mapToParent(posInFrame.topLeft());
      QSize sz = size();
      return QRect(ptTopLeft, sz);
   }
   else {
      return geometry();
   }
}

//============== set internal geometry ==============//

void QextMdiChildView::setInternalGeometry(const QRect& newGeometry)
{
   if(mdiParent()) {
      // retrieve the frame size
      QRect    geo      = internalGeometry();
      QRect    frameGeo = externalGeometry();
      int      nFrameSizeTop  = geo.y() - frameGeo.y();
      int      nFrameSizeLeft = geo.x() - frameGeo.x();

      // create the new geometry that is accepted by the QWidget::setGeometry() method
      QRect    newGeoQt;
      newGeoQt.setX(newGeometry.x()-nFrameSizeLeft);
      newGeoQt.setY(newGeometry.y()-nFrameSizeTop);
      newGeoQt.setWidth(newGeometry.width()+QEXTMDI_MDI_CHILDFRM_DOUBLE_BORDER);
      newGeoQt.setHeight(newGeometry.height()+mdiParent()->captionHeight()-QEXTMDI_MDI_CHILDFRM_DOUBLE_BORDER);

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

QRect QextMdiChildView::externalGeometry() const
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
   if(mdiParent()) {
      if(!isMinimized()) {
         mdiParent()->setState(QextMdiChildFrm::Minimized,bAnimate);
      }
   } 
   else {
      showMinimized();
   }
}

void QextMdiChildView::showMinimized()
{
   //qDebug("is minimized now");
   emit isMinimizedNow();
   QWidget::showMinimized();
}

void QextMdiChildView::minimize() { minimize(TRUE); }

//============= maximize ==============//

void QextMdiChildView::maximize(bool bAnimate)
{
   if(mdiParent()) {
      if(!isMaximized()) {
         mdiParent()->setState(QextMdiChildFrm::Maximized,bAnimate);
         emit mdiParentNowMaximized(TRUE);
      }
   } else {
      showMaximized();
   }
}

void QextMdiChildView::showMaximized()
{
   //qDebug("is maximized now");
   emit isMaximizedNow();
   QWidget::showMaximized();
}

void QextMdiChildView::maximize() { maximize(TRUE); }

//============== restoreGeometry ================//

QRect QextMdiChildView::restoreGeometry()
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

void  QextMdiChildView::setRestoreGeometry(const QRect& newRestGeo)
{
   if(mdiParent()) {
      mdiParent()->setRestoreGeometry(newRestGeo);
   }
   else {
      // XXX not supported, may be we must use Windows or X11 funtions
   }
}

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

bool QextMdiChildView::isMinimized() const
{
   if(mdiParent()) {
      return (mdiParent()->state() == QextMdiChildFrm::Minimized);
   }
   else {
      return QWidget::isMinimized();
   }
}

//============== isMaximized ? ==================//

bool QextMdiChildView::isMaximized() const
{
   if(mdiParent()) {
      return (mdiParent()->state() == QextMdiChildFrm::Maximized);
   }
   else {
      return QWidget::isMaximized();
   }
}

//============== restore ================//

void QextMdiChildView::restore()
{
   if(mdiParent()) {
      if(isMaximized()) {
         emit mdiParentNowMaximized(FALSE);
      }
      if(isMinimized()||isMaximized()) {
         mdiParent()->setState(QextMdiChildFrm::Normal);
      }
   } else {
      showNormal();
   }
}

void QextMdiChildView::showNormal()
{
   //qDebug("is restored now");
   emit isRestoredNow();
   QWidget::showNormal();
}

//=============== youAreAttached ============//

void QextMdiChildView::youAreAttached(QextMdiChildFrm *lpC)
{
   lpC->setCaption(m_szCaption);

   emit isAttachedNow();
}

//================ youAreDetached =============//

void QextMdiChildView::youAreDetached()
{
   setCaption(m_szCaption);

   setTabCaption(m_sTabCaption);
   if(myIconPtr())setIcon(*(myIconPtr()));
   setFocusPolicy(QWidget::StrongFocus);

   emit isDetachedNow();
}

//================ setCaption ================//
// this set the caption of only the window
void QextMdiChildView::setCaption(const QString& szCaption)
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

void QextMdiChildView::closeEvent(QCloseEvent *e)
{
   e->ignore(); //we ignore the event , and then close later if needed.
   emit childWindowCloseRequest(this);
}

//================ myIconPtr =================//

QPixmap* QextMdiChildView::myIconPtr()
{
   return 0;
}

//============= focusInEvent ===============//

void QextMdiChildView::focusInEvent(QFocusEvent *e)
{
   QWidget::focusInEvent(e);

   // every widget get a focusInEvent when a popup menu is opened!?! -> maybe bug of QT
   if(e && ((e->reason())==QFocusEvent::Popup)) {
      return;
   }

   m_bFocusInEventIsPending = TRUE;
   activate();
   m_bFocusInEventIsPending = FALSE;

   emit gotFocus(this);
}

//============= activate ===============//

void QextMdiChildView::activate()
{
   // avoid circularity
   static bool s_bActivateIsPending = FALSE;
   if(s_bActivateIsPending) {
      return;
   }
   s_bActivateIsPending = TRUE;

   // raise the view and push the taskbar button
   if(!m_bMainframesActivateViewIsPending) {
     emit focusInEventOccurs( this);
   }

   // check if someone wants that we interrupt the method (because it's probably unnecessary)
   if(m_bInterruptActivation) {
      m_bInterruptActivation = FALSE;
   }
   else {
      if(!m_bFocusInEventIsPending) {
         setFocus();
      }
      qDebug("QextMdiChildView::activate() called!");
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
   s_bActivateIsPending = FALSE;
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

   if(m_stateChanged) {
      m_stateChanged = FALSE;
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

//============= eventFilter ===============//

bool QextMdiChildView::eventFilter(QObject *obj, QEvent *e )
{
   if(e->type() == QEvent::KeyPress && isAttached()) {
      QKeyEvent* ke = (QKeyEvent*) e;
      if(ke->key() == Qt::Key_Tab) {
         //qDebug("ChildView %i::eventFilter - TAB from %s (%s)", this, obj->name(), obj->className());
         QWidget* w = (QWidget*) obj;
         if((w->focusPolicy() == QWidget::StrongFocus) || 
            (w->focusPolicy() == QWidget::TabFocus   ) || 
            (w->focusPolicy() == QWidget::WheelFocus ))
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
      if(obj->inherits("QWidget")) {
         QObjectList *list = queryList( "QWidget" );
         if(list->find(obj) != -1) {
            m_focusedChildWidget = (QWidget*)obj;
         }
      }
      if (!isAttached()) {   // is toplevel, for attached views activation is done by frame event filter
         static bool m_bActivationIsPending = FALSE;
         if(!m_bActivationIsPending) {
            m_bActivationIsPending = TRUE;
            activate(); // sets the focus
            m_bActivationIsPending = FALSE;
         }
      }
   }
   else if (e->type() == QEvent::ChildRemoved) {
      // if we lost a child we uninstall ourself as event filter for the lost 
      // child and its children
      QObject* pLostChild = ((QChildEvent*)e)->child();
      if((pLostChild != 0L) /*&& (pLostChild->inherits("QWidget"))*/) {
         QObjectList *list = pLostChild->queryList();
         list->insert(0, pLostChild);        // add the lost child to the list too, just to save code
         QObjectListIt it( *list );          // iterate over all lost child widgets
         QObject * obj;
         while ( (obj=it.current()) != 0 ) { // for each found object...
            QWidget* widg = (QWidget*)obj;
            ++it;
            widg->removeEventFilter(this);
            if((widg->focusPolicy() == QWidget::StrongFocus) || 
               (widg->focusPolicy() == QWidget::TabFocus   ) ||
               (widg->focusPolicy() == QWidget::WheelFocus ))
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
   else if (e->type() == QEvent::ChildInserted) {
      // if we got a new child and we are attached to the MDI system we
      // install ourself as event filter for the new child and its children
      // (as we did when we were added to the MDI system).
      QObject* pNewChild = ((QChildEvent*)e)->child();
      if((pNewChild != 0L) && (pNewChild->inherits("QWidget")) &&
         !(pNewChild->inherits("QMessageBox")) && !(pNewChild->inherits("QFileDialog")) &&
         !(pNewChild->inherits("KMessageBox")) && !(pNewChild->inherits("KDialogBase")) && 
         !(pNewChild->inherits("QPopupMenu")) )
      {
         QWidget* pNewWidget = (QWidget*)pNewChild;
         QObjectList *list = pNewWidget->queryList( "QWidget" );
         list->insert(0, pNewChild);         // add the new child to the list too, just to save code
         QObjectListIt it( *list );          // iterate over all new child widgets
         QObject * obj;
         while ( (obj=it.current()) != 0 ) { // for each found object...
            if (!obj->inherits("QWidget")) {
              ++it;
              continue;
            }
            QWidget* widg = (QWidget*)obj;
            ++it;
            widg->installEventFilter(this);
            if((widg->focusPolicy() == QWidget::StrongFocus) ||
               (widg->focusPolicy() == QWidget::TabFocus   ) ||
               (widg->focusPolicy() == QWidget::WheelFocus ))
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
   return FALSE;                           // standard event processing
}

//============= insertChild ===============//
void QextMdiChildView::insertChild(QObject *pChild)
{
   QWidget::insertChild(pChild);

   QObjectList *list = pChild->queryList( "QWidget" );
   list->append(pChild);               // care for the new child too
   QObjectListIt it( *list );          // iterate over all child widgets
   QObject * obj;
   while ( (obj=it.current()) != 0 ) { // for each found object...
      if (!obj->inherits("QWidget")) {
         ++it;
         continue;
      }
      QWidget* widg = (QWidget*)obj;
      ++it;
      if (!(widg->inherits("QPopupMenu"))) {
         widg->installEventFilter(this);
         if((widg->focusPolicy() == QWidget::StrongFocus) ||
            (widg->focusPolicy() == QWidget::TabFocus   ) ||
            (widg->focusPolicy() == QWidget::WheelFocus ))
         {
            if(m_firstFocusableChildWidget == 0) {
               m_firstFocusableChildWidget = widg;  // first widget
            }
            m_lastFocusableChildWidget = widg; // last widget
         }
      }
   }
   if(m_lastFocusableChildWidget != 0) {
      if(QString(m_lastFocusableChildWidget->name()) == QString("qt_viewport")) {
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
   delete list;                        // delete the list, not the objects
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
      if (!obj->inherits("QWidget")) {
         ++it;
         continue;
      }
      QWidget* widg = (QWidget*)obj;
      ++it;
      if (!(widg->inherits("QPopupMenu"))) {
         widg->installEventFilter(this);
         if((widg->focusPolicy() == QWidget::StrongFocus) ||
            (widg->focusPolicy() == QWidget::TabFocus   ) ||
            (widg->focusPolicy() == QWidget::WheelFocus ))
         {
            if(m_firstFocusableChildWidget == 0) {
               m_firstFocusableChildWidget = widg;  // first widget
            }
            m_lastFocusableChildWidget = widg; // last widget
            //qDebug("*** %s (%s)",widg->name(),widg->className());
         }
      }
   }
   //qDebug("### |%s|", m_lastFocusableChildWidget->name());
   if(m_lastFocusableChildWidget != 0) {
      if(QString(m_lastFocusableChildWidget->name()) == QString("qt_viewport")) {
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
void QextMdiChildView::setTabCaption (const QString& stbCaption) {

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
   if(mdiParent() != 0L) {
      mdiParent()->setMinimumSize( minw + QEXTMDI_MDI_CHILDFRM_DOUBLE_BORDER,
                                   minh + QEXTMDI_MDI_CHILDFRM_DOUBLE_BORDER
                                        + QEXTMDI_MDI_CHILDFRM_SEPARATOR
                                        + mdiParent()->captionHeight());
   }
}

//============= setMaximumSize ===============//

void QextMdiChildView::setMaximumSize( int maxw, int maxh)
{
   if(mdiParent() != 0L) {
      int w = maxw + QEXTMDI_MDI_CHILDFRM_DOUBLE_BORDER;
      if(w > QWIDGETSIZE_MAX) { w = QWIDGETSIZE_MAX; }
      int h = maxh + QEXTMDI_MDI_CHILDFRM_DOUBLE_BORDER + QEXTMDI_MDI_CHILDFRM_SEPARATOR + mdiParent()->captionHeight();
      if(h > QWIDGETSIZE_MAX) { h = QWIDGETSIZE_MAX; }
      mdiParent()->setMaximumSize( w, h);
   }
   QWidget::setMaximumSize( maxw, maxh);
}

//============= show ===============//

void QextMdiChildView::show()
{
   QWidget* pParent = mdiParent();
   if(pParent != 0L) {
      pParent->show();
   }
   QWidget::show();
}

//============= hide ===============//

void QextMdiChildView::hide()
{
   if(mdiParent() != 0L) {
      mdiParent()->hide();
   }
   QWidget::hide();
}

#ifndef NO_INCLUDE_MOCFILES
#include "qextmdichildview.moc"
#endif
