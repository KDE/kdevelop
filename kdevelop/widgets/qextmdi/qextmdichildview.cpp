//----------------------------------------------------------------------------
//    filename             : qextmdichildview.cpp
//----------------------------------------------------------------------------
//    Project              : Qt MDI extension
//
//    begin                : 07/1999       by Szymon Stefanek as part of kvirc
//                                         (an IRC application)
//    changes              : 09/1999       by Falk Brettschneider to create a
//                                         stand-alone Qt extension set of
//                                         classes and a Qt-based library
//                           02/2000       by Massimo Morin (mmorin@schedsys.com)
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

//============== external geometry ==============//

QRect QextMdiChildView::externalGeometry()
{
   return mdiParent() ? mdiParent()->geometry() : geometry();
}

//============== set external geometry ==============//

void QextMdiChildView::setExternalGeometry(const QRect& newGeomety)
{
   mdiParent() ? mdiParent()->setGeometry(newGeomety) : setGeometry(newGeomety);
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

void QextMdiChildView::minimize(){ minimize(TRUE); }

//============= maximize ==============//

void QextMdiChildView::maximize(bool bAnimate)
{
	if(mdiParent()){
		if(!isMaximized()){
		   emit mdiParentNowMaximized();
			mdiParent()->setState(QextMdiChildFrm::Maximized,bAnimate);
		}
	} else showMaximized();
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
   //qDebug("ChildView::focusInEvent");
   emit focusInEventOccurs( this);

   if( m_focusedChildWidget != 0L) {
      //qDebug("ChildView::focusInEvent 2");
      m_focusedChildWidget->setFocus();
   }
   else
      if( m_firstFocusableChildWidget != 0L) {
         //qDebug("ChildView::focusInEvent 3");
         m_firstFocusableChildWidget->setFocus();
         m_focusedChildWidget = m_firstFocusableChildWidget;
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
   QWidget::setMaximumSize( maxw, maxh);
   if( mdiParent() != 0L)
      mdiParent()->setMaximumSize( maxw + QEXTMDI_MDI_CHILDFRM_DOUBLE_BORDER,
                                   maxh + QEXTMDI_MDI_CHILDFRM_DOUBLE_BORDER
                                        + QEXTMDI_MDI_CHILDFRM_SEPARATOR
                                        + mdiParent()->captionHeight());
}
