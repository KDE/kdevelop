//----------------------------------------------------------------------------
//    $Id$
//----------------------------------------------------------------------------
//    Project              : Qt MDI extension
//
//    begin                : 07/1999       by Szymon Stefanek as part of kvirc
//                                         (an IRC application)
//    changes              : 09/1999       by Falk Brettschneider to create an
//                                         stand-alone Qt extension set of
//                                         classes and a Qt-based library
//
//    copyright            : (C) 1999-2000 by Szymon Stefanek (stefanek@tin.it)
//                                         and
//                                         Falk Brettschneider
//    email                :  gigafalk@geocities.com (Falk Brettschneider)
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

#include <qcursor.h>
#include <qclipboard.h>
#include <qobjcoll.h>
#include <qtoolbutton.h>

#include "qextmdimainfrm.h"
#include "qextmditaskbar.h"
#include "qextmdichildfrm.h"
#include "qextmdichildarea.h"
#include "qextmdichildview.h"

//###########################################
//
// CREATION PROCESS
//
//###########################################

//============ constructor ============//

QextMdiMainFrm::QextMdiMainFrm(QWidget* parentWidget, const char* name, WFlags flags)
: QMainWindow( parentWidget, name, flags)
	,m_pMdi(0)
   ,m_pTaskBar(0)
	,m_pWinList(0)
	,m_pCurrentWindow(0)
   ,m_pWindowPopup(0)
   ,m_pTaskBarPopup(0)
   ,m_pMainMenuBar(0)
{
   setRightJustification( true);

	// Create the local list of windows
	m_pWinList = new QList<QextMdiChildView>;
	m_pWinList->setAutoDelete(false);
	// This seems to be needed (re-check it after Qt2.0 comed out)
	setFocusPolicy(ClickFocus);
	
	// And start creating self
	createMdiManager();
	createTaskBar();
	
	// Apply options for the MDI manager
	applyOptions();

	// No active window yet (when shown , the console should be the active one)
//F.B.	updateCaption();

	// Set the geometry
//F.B.	setGeometry(g_pOptions->m_rectFrameGeometry); //does not work with KWM... ???

	m_pTaskBarPopup=new QPopupMenu();   //F.B.
	m_pWindowPopup=new QPopupMenu();    //F.B.
}

//============ applyOptions ============//
void QextMdiMainFrm::applyOptions()
{
//F.B.	m_pMdi->setMdiCaptionInactiveBackColor(g_pOptions->m_clrMdiCaptionInactiveBack);
//F.B.	m_pMdi->setMdiCaptionInactiveForeColor(g_pOptions->m_clrMdiCaptionInactiveFore);
//F.B.	m_pMdi->setMdiCaptionActiveBackColor(g_pOptions->m_clrMdiCaptionActiveBack);
//F.B.	m_pMdi->setMdiCaptionActiveForeColor(g_pOptions->m_clrMdiCaptionActiveFore);
//F.B.	m_pMdi->setMdiCaptionFont(g_pOptions->m_fntMdiCaption);

//F.B.	if(!g_pOptions->m_pixMdiBack->isNull())m_pMdi->setBackgroundPixmap(*(g_pOptions->m_pixMdiBack));
//F.B.	else m_pMdi->setBackgroundColor(g_pOptions->m_clrMdiBack);

	for(QextMdiChildView *w = m_pWinList->first();w;w= m_pWinList->next()){
//F.B.		w->applyOptions();
		QWidget *wdgt = w;
		if(w->mdiParent())wdgt =w->mdiParent();
		// Really ugly hack to FORCE the resize event
		// a resize(width(),height()) won't work...
		wdgt->resize(wdgt->width()+1,wdgt->height()+1);
		wdgt->resize(wdgt->width()-1,wdgt->height()-1);
	}
}

//============ createMdiManager ============//
void QextMdiMainFrm::createMdiManager()
{
	m_pMdi=new QextMdiChildArea(this);
	setCentralWidget(m_pMdi);
//	QObject::connect( m_pMdi, SIGNAL(topChildChanged(QextMdiChildView*)), this, SLOT(pushNewTaskBarButton(QextMdiChildView*)) );
	QObject::connect( m_pMdi, SIGNAL(closeActiveView()), this, SLOT(closeActiveView()) );
	QObject::connect( m_pMdi, SIGNAL(closeAllViews()), this, SLOT(closeAllViews()) );
	QObject::connect( m_pMdi, SIGNAL(switchToToplevelMode()), this, SLOT(switchToToplevelMode()) );
	QObject::connect( m_pMdi, SIGNAL(switchToChildframeMode()), this, SLOT(switchToChildframeMode()) );
	QObject::connect( m_pMdi, SIGNAL(insertSysButtonsInMainMenu(const QPixmap*, const QPixmap*, const QPixmap*, const QPixmap*, const QPixmap*, const QObject*, const char*, const char*, const char*, const char*, const char*)),
	                  this, SIGNAL(insertSysButtonsInMainMenu(const QPixmap*, const QPixmap*, const QPixmap*, const QPixmap*, const QPixmap*, const QObject*, const char*, const char*, const char*, const char*, const char*)) );
	QObject::connect( m_pMdi, SIGNAL(updateSysButtonsInMainMenu(const QObject*, const char*, const char*, const char*, const char*, const char*)),
	                  this, SIGNAL(updateSysButtonsInMainMenu(const QObject*, const char*, const char*, const char*, const char*, const char*)) );
   QObject::connect( m_pMdi, SIGNAL(removeSysButtonsFromMainMenu()), this, SIGNAL(removeSysButtonsFromMainMenu()) );
}

//============ createTaskBar ==============//
void QextMdiMainFrm::createTaskBar()
{
	m_pTaskBar = new QextMdiTaskBar(this,QMainWindow::Bottom);
//F.B.	if(!g_pOptions->m_bTaskBarVisible)m_pTaskBar->hide();
  m_pTaskBar->show();
}

void QextMdiMainFrm::slot_toggleTaskBar()
{
	if(m_pTaskBar->isVisible()){
		m_pTaskBar->hide();
//F.B.		g_pOptions->m_bTaskBarVisible = false;
	} else {
		m_pTaskBar->show();
//F.B.		g_pOptions->m_bTaskBarVisible = true;
	}
}

//============ ~QextMdiMainFrm ============//
QextMdiMainFrm::~QextMdiMainFrm()
{
/*F.B.	// Save the last geometry
	if(g_pOptions->m_bUseHackedFrameGeometry){
		g_pOptions->m_rectFrameGeometry.moveTopLeft(frameGeometry().topLeft());
		g_pOptions->m_rectFrameGeometry.setSize(size());
	} else g_pOptions->m_rectFrameGeometry = geometry();
F.B.*/
	
	// safely close the windows so properties are saved...
	QextMdiChildView *pWnd = 0;
	while((pWnd = m_pWinList->first()))closeWindow(pWnd, false); // without re-layout taskbar!
	delete m_pWinList;
   delete m_pTaskBarPopup;
   delete m_pWindowPopup;
}

//============ closeEvent ============//
void QextMdiMainFrm::closeEvent(QCloseEvent *e)
{
	e->accept();
	delete this;
}

//================ addWindow ================//
void QextMdiMainFrm::addWindow(QextMdiChildView *pWnd,bool bShow,bool bAttach, bool bMaximized, QRect* pNormalSizeRect)
{
	QObject::connect( pWnd, SIGNAL(attachWindow(QextMdiChildView*,bool,bool,QRect*)), this, SLOT(attachWindow(QextMdiChildView*,bool,bool,QRect*)) );
	QObject::connect( pWnd, SIGNAL(detachWindow(QextMdiChildView*)), this, SLOT(detachWindow(QextMdiChildView*)) );
	QObject::connect( pWnd, SIGNAL(focusInEventOccurs(QextMdiChildView*)), this, SLOT(activateView(QextMdiChildView*)) );
	QObject::connect( pWnd, SIGNAL(childWindowCloseRequest(QextMdiChildView*)), this, SLOT(childWindowCloseRequest(QextMdiChildView*)) );
	
	//The window can be added only once :)
	m_pWinList->append(pWnd);
	QextMdiTaskBarButton* but = m_pTaskBar->addWinButton(pWnd);
	QObject::connect( pWnd, SIGNAL(windowCaptionChanged(const QString&)), but, SLOT(setNewText(const QString&)) );

	if( bAttach) {
		if( pNormalSizeRect == 0)
	   	attachWindow( pWnd, bShow, true, 0);
		else
			attachWindow( pWnd, bShow, false, pNormalSizeRect);	// do not cascade
	}
	else {	// don't attach
		if( pNormalSizeRect != 0)
			pWnd->setGeometry( *pNormalSizeRect);
	}			
	if( bMaximized) pWnd->maximize();
}

//============ attachWindow ============//
void QextMdiMainFrm::attachWindow(QextMdiChildView *pWnd,bool bShow,bool overrideGeometry,QRect *r)
{
	//Creates a new QextMdiChildFrm in m_pMdi manager
	//Attaches the QextMdiChildView to that child and starts the manager
	m_pCurrentWindow  = pWnd;
	QextMdiChildFrm *lpC=new QextMdiChildFrm(m_pMdi);
	lpC->setClient(pWnd);
	pWnd->youAreAttached(lpC);
	if(!overrideGeometry && r)lpC->setGeometry(*r);
	m_pMdi->manageChild(lpC,bShow,overrideGeometry);
//   pushNewTaskBarButton( pWnd);
}

//============= detachWindow ==============//
void QextMdiMainFrm::detachWindow(QextMdiChildView *pWnd)
{
	//Reparents pWnd to 0
	//Destroys the QextMdiChildFrm that contained it
	if(!pWnd->parent())return;
	QextMdiChildFrm *lpC=pWnd->mdiParent();
	lpC->unsetClient();
	pWnd->youAreDetached();
	m_pTaskBar->setActiveButton(pWnd);  //F.B.
	m_pMdi->destroyChild(lpC,false); //Do not focus the new top child , we loose focus...
}

//============== removeWindowFromMdi ==============//
void QextMdiMainFrm::removeWindowFromMdi(QextMdiChildView *pWnd)
{
	QObject::disconnect( pWnd, SIGNAL(attachWindow(QextMdiChildView*,bool,bool,QRect*)), this, SLOT(attachWindow(QextMdiChildView*,bool,bool,QRect*)) );
	QObject::disconnect( pWnd, SIGNAL(detachWindow(QextMdiChildView*)), this, SLOT(detachWindow(QextMdiChildView*)) );
	QObject::disconnect( pWnd, SIGNAL(focusInEventOccurs(QextMdiChildView*)), this, SLOT(activateView(QextMdiChildView*)) );
	QObject::disconnect( pWnd, SIGNAL(childWindowCloseRequest(QextMdiChildView*)), this, SLOT(childWindowCloseRequest(QextMdiChildView*)) );
	
	//Closes a child window. sends no close event : simply deletes it
//F.B.	pWnd->saveProperties();
	m_pWinList->removeRef(pWnd);
	
	QextMdiTaskBarButton* but = m_pTaskBar->getButton(pWnd);
	QObject::disconnect( pWnd, SIGNAL(windowCaptionChanged(const QString&)), but, SLOT(setNewText(const QString&)) );
	m_pTaskBar->removeWinButton(pWnd);
	
	if(pWnd->isAttached())m_pMdi->destroyChildButNotItsView(pWnd->mdiParent());
}

//============== closeWindow ==============//
void QextMdiMainFrm::closeWindow(QextMdiChildView *pWnd, bool layoutTaskBar)
{
	QObject::disconnect( pWnd, SIGNAL(attachWindow(QextMdiChildView*,bool,bool,QRect*)), this, SLOT(attachWindow(QextMdiChildView*,bool,bool,QRect*)) );
	QObject::disconnect( pWnd, SIGNAL(detachWindow(QextMdiChildView*)), this, SLOT(detachWindow(QextMdiChildView*)) );
	QObject::disconnect( pWnd, SIGNAL(focusInEventOccurs(QextMdiChildView*)), this, SLOT(activateView(QextMdiChildView*)) );
	QObject::disconnect( pWnd, SIGNAL(childWindowCloseRequest(QextMdiChildView*)), this, SLOT(childWindowCloseRequest(QextMdiChildView*)) );
	
	//Closes a child window. sends no close event : simply deletes it
//F.B.	pWnd->saveProperties();
	m_pWinList->removeRef(pWnd);
	if( m_pWinList->count() == 0)
		m_pCurrentWindow = 0;
	
	QextMdiTaskBarButton* but = m_pTaskBar->getButton(pWnd);
	QObject::disconnect( pWnd, SIGNAL(windowCaptionChanged(const QString&)), but, SLOT(setNewText(const QString&)) );
	m_pTaskBar->removeWinButton(pWnd, layoutTaskBar);
	
	if(pWnd->isAttached())m_pMdi->destroyChild(pWnd->mdiParent());
	else delete pWnd;
}

//================== findWindow =================//
QextMdiChildView * QextMdiMainFrm::findWindow(const QString& caption)
{
	for(QextMdiChildView *w=m_pWinList->first();w;w=m_pWinList->next()){
		if(QString(w->caption()) == QString(caption))return w;   //F.B.
	}
	return 0;
}

//================== activeWindow ===================//
QextMdiChildView * QextMdiMainFrm::activeWindow()
{
	return m_pCurrentWindow;
}
//================== windowExists ? =================//
bool QextMdiMainFrm::windowExists(QextMdiChildView *pWnd)
{
	for(QextMdiChildView *w=m_pWinList->first();w;w=m_pWinList->next()){
		if(w == pWnd)return true;
	}
	return false;
}

QPopupMenu * QextMdiMainFrm::windowPopup(QextMdiChildView * pWnd,bool bIncludeTaskbarPopup)
{
	m_pWindowPopup->clear();
	if(bIncludeTaskbarPopup){
		m_pWindowPopup->insertItem(tr("Window"),taskBarPopup(pWnd,false));
		m_pWindowPopup->insertSeparator();
	}
	return m_pWindowPopup;
}

//================ taskBarPopup =================//
QPopupMenu * QextMdiMainFrm::taskBarPopup(QextMdiChildView *pWnd,bool bIncludeWindowPopup)
{
	//returns the g_pTaskBarPopup filled according to the QextMdiChildView state
	m_pTaskBarPopup->clear();
	if(pWnd->isAttached()){
		m_pTaskBarPopup->insertItem(tr("Undock"),pWnd,SLOT(detach()));
		m_pTaskBarPopup->insertSeparator();
		if(pWnd->isMinimized() || pWnd->isMaximized())
			m_pTaskBarPopup->insertItem(tr("Restore"),pWnd,SLOT(restore()));
		if(!pWnd->isMaximized())m_pTaskBarPopup->insertItem(tr("Maximize"),pWnd,SLOT(maximize()));
		if(!pWnd->isMinimized())m_pTaskBarPopup->insertItem(tr("Minimize"),pWnd,SLOT(minimize()));
	} else m_pTaskBarPopup->insertItem(tr("Dock"),pWnd,SLOT(attach()));
	m_pTaskBarPopup->insertSeparator();
	m_pTaskBarPopup->insertItem(tr("Close"),pWnd,SLOT(close()));
//F.B.	if(pWnd->m_pView && bIncludeWindowPopup){
		// the window has a view...get the window popup
		m_pTaskBarPopup->insertSeparator();
		m_pTaskBarPopup->insertItem(tr("Operations"),windowPopup(pWnd,false));  //alvoid recursion
//F.B.	}
	return m_pTaskBarPopup;
	bIncludeWindowPopup = false; // dummy!, only to avoid "unused parameter"
}

void QextMdiMainFrm::switchWindows(bool bRight)
{
	QextMdiChildFrm * pActMdi = m_pMdi->topChild();
	if(!pActMdi)return;
	QextMdiChildView * pAct = (QextMdiChildView *)pActMdi->m_pClient;
	if(!pAct)return;
	bRight = false; // dummy!, only to avoid "unused parameter"
}

//void QextMdiMainFrm::pushNewTaskBarButton( QextMdiChildView* pWnd)
//{
//// TODO: if maximised --> exchange sys menu buttons
//   m_pCurrentWindow = pWnd;
//   taskbarButtonLeftClicked( m_pCurrentWindow);
//}

void QextMdiMainFrm::activateView(QextMdiChildView *pWnd)
{
  	if(pWnd->isAttached()){
      if( !(pWnd->hasFocus()) ) {
         if( m_pCurrentWindow->isMaximized()) {
            pWnd->mdiParent()->updateSysButtonsInMainMenu();
         }
         pWnd->setFocus();
      }
	}
   else { //not so cool...bu can not do more...
	  	raiseTopLevelWidget(pWnd);
   }
	m_pCurrentWindow = pWnd;
	m_pTaskBar->setActiveButton(pWnd);
}

void QextMdiMainFrm::taskbarButtonRightClicked(QextMdiChildView *pWnd)
{
	//taskbarButtonLeftClicked( pWnd); // set focus
	activateView( pWnd); // set focus
	taskBarPopup( pWnd, true)->popup( QCursor::pos());
}

void QextMdiMainFrm::childWindowCloseRequest(QextMdiChildView *pWnd)
{
	QextMdiViewCloseEvent* ce = new QextMdiViewCloseEvent( pWnd);
	QApplication::postEvent( this, ce);
}

//##################################################
// Internal handlers
//##################################################

void QextMdiMainFrm::focusInEvent(QFocusEvent *)
{
   //qDebug("QextMdiMainFrm::focusInEvent");
	m_pMdi->setFocus();
}

void QextMdiMainFrm::raiseTopLevelWidget(QWidget * ptr)
{
   if(ptr->hasFocus())return;
   if(ptr->isActiveWindow())return;
   ptr->show();
   ptr->raise();
   ptr->setActiveWindow();
}

bool QextMdiMainFrm::event( QEvent* e)
{
	if( e->type() == QEvent::User) {
		QextMdiChildView* pWnd = (QextMdiChildView*)((QextMdiViewCloseEvent*)e)->data();
		if( pWnd != 0)
			closeWindow( pWnd);
		return true;
	}
	return QMainWindow::event( e);		
}

/**
 * close all views
 */
void QextMdiMainFrm::closeAllViews()
{
	for(QextMdiChildView *w = m_pWinList->first();w;w= m_pWinList->next()){
		w->close();
	}
}

/**
 * closes the view of the active (topchild) window
 */
void QextMdiMainFrm::closeActiveView()
{
	if( m_pCurrentWindow != 0)
		m_pCurrentWindow->close();
}

/**
 * undocks all view windows (unix-like)
 */
void QextMdiMainFrm::switchToToplevelMode()
{
   int i = 0;
	for(QextMdiChildView *w = m_pWinList->first();w;w= m_pWinList->next()){
		if( w->isAttached())
		   i++;
			detachWindow(w);
	}
	if( i > 1)
      slot_removeSysButtonsFromMainMenu();
}

/**
 * docks all view windows (Windows-like)
 */
void QextMdiMainFrm::switchToChildframeMode()
{
	for(QextMdiChildView *w = m_pWinList->first();w;w= m_pWinList->next()){
		if( !w->isAttached())
			attachWindow(w, true, true, 0);
	}
}

/**
 * redirect the signal for insertion of buttons to an own slot
 * that means: If the menubar (where the buttons should be inserted) is given,
 *             QextMDI can insert them automatically.
 *             Otherwise only signals can be emitted to tell the outside that
 *             someone must do this job itself.
 */
void QextMdiMainFrm::setMenuForSDIModeSysButtons( QMenuBar* pMenuBar)
{
   m_pMainMenuBar = pMenuBar;
 	QObject::disconnect( m_pMdi, SIGNAL(insertSysButtonsInMainMenu(const QPixmap*, const QPixmap*, const QPixmap*, const QPixmap*, const QPixmap*, const QObject*, const char*, const char*, const char*, const char*, const char*)),
	                     this, SIGNAL(insertSysButtonsInMainMenu(const QPixmap*, const QPixmap*, const QPixmap*, const QPixmap*, const QPixmap*, const QObject*, const char*, const char*, const char*, const char*, const char*)) );
 	QObject::connect( m_pMdi, SIGNAL(insertSysButtonsInMainMenu(const QPixmap*, const QPixmap*, const QPixmap*, const QPixmap*, const QPixmap*, const QObject*, const char*, const char*, const char*, const char*, const char*)),
	                  this, SLOT(slot_insertSysButtonsInMainMenu(const QPixmap*, const QPixmap*, const QPixmap*, const QPixmap*, const QPixmap*, const QObject*, const char*, const char*, const char*, const char*, const char*)) );
 	QObject::disconnect( m_pMdi, SIGNAL(updateSysButtonsInMainMenu(const QObject*, const char*, const char*, const char*, const char*, const char*)),
	                     this, SIGNAL(updateSysButtonsInMainMenu(const QObject*, const char*, const char*, const char*, const char*, const char*)) );
 	QObject::connect( m_pMdi, SIGNAL(updateSysButtonsInMainMenu(const QObject*, const char*, const char*, const char*, const char*, const char*)),
	                  this, SLOT(slot_updateSysButtonsInMainMenu(const QObject*, const char*, const char*, const char*, const char*, const char*)) );
   QObject::disconnect( m_pMdi, SIGNAL(removeSysButtonsFromMainMenu()), this, SIGNAL(removeSysButtonsFromMainMenu()) );
   QObject::connect( m_pMdi, SIGNAL(removeSysButtonsFromMainMenu()), this, SLOT(slot_removeSysButtonsFromMainMenu()) );

}

void QextMdiMainFrm::slot_insertSysButtonsInMainMenu(const QPixmap* pSystemMenuPM, const QPixmap* pUndockPM, const QPixmap* pMinPM, const QPixmap* pRestorePM, const QPixmap* pClosePM, const QObject* receiver, const char* sysMenuFunc, const char* undockFunc, const char* minFunc, const char* restoreFunc, const char* closeFunc)
{
   m_pMainMenuBar->insertItem( *pSystemMenuPM, receiver, sysMenuFunc, /*accel*/0, /*id*/-1, 0);
   if( style().guiStyle() == Qt::MotifStyle) {
      m_pMainMenuBar->insertSeparator();
      m_pMainMenuBar->insertItem( *pUndockPM, receiver, undockFunc);
      m_pMainMenuBar->insertItem( *pMinPM, receiver, minFunc);
      m_pMainMenuBar->insertItem( *pRestorePM, receiver, restoreFunc);
      m_pMainMenuBar->insertItem( *pClosePM, receiver, closeFunc);
   }
}

void QextMdiMainFrm::slot_updateSysButtonsInMainMenu(const QObject* receiver, const char* sysMenuFunc, const char* undockFunc, const char* minFunc, const char* restoreFunc, const char* closeFunc)
{
   int itemcount = m_pMainMenuBar->count();
   // disconnect the old child frame
   m_pMainMenuBar->disconnectItem( m_pMainMenuBar->idAt(0), m_pCurrentWindow->mdiParent(), sysMenuFunc);
   if( style().guiStyle() == Qt::MotifStyle) {
      m_pMainMenuBar->disconnectItem( m_pMainMenuBar->idAt(itemcount-4), m_pCurrentWindow->mdiParent(), undockFunc);
      m_pMainMenuBar->disconnectItem( m_pMainMenuBar->idAt(itemcount-3), m_pCurrentWindow->mdiParent(), minFunc);
      m_pMainMenuBar->disconnectItem( m_pMainMenuBar->idAt(itemcount-2), m_pCurrentWindow->mdiParent(), restoreFunc);
      m_pMainMenuBar->disconnectItem( m_pMainMenuBar->idAt(itemcount-1), m_pCurrentWindow->mdiParent(), closeFunc);
   }
   // connect the new child frame
   m_pMainMenuBar->connectItem( m_pMainMenuBar->idAt(0), receiver, sysMenuFunc);
   if( style().guiStyle() == Qt::MotifStyle) {
      m_pMainMenuBar->connectItem( m_pMainMenuBar->idAt(itemcount-4), receiver, undockFunc);
      m_pMainMenuBar->connectItem( m_pMainMenuBar->idAt(itemcount-3), receiver, minFunc);
      m_pMainMenuBar->connectItem( m_pMainMenuBar->idAt(itemcount-2), receiver, restoreFunc);
      m_pMainMenuBar->connectItem( m_pMainMenuBar->idAt(itemcount-1), receiver, closeFunc);
   }
}

void QextMdiMainFrm::slot_removeSysButtonsFromMainMenu()
{
   int itemcount = m_pMainMenuBar->count();
   if( style().guiStyle() == Qt::MotifStyle) {
      m_pMainMenuBar->removeItemAt( itemcount - 1); // close
      m_pMainMenuBar->removeItemAt( itemcount - 2); // restore
      m_pMainMenuBar->removeItemAt( itemcount - 3); // min
      m_pMainMenuBar->removeItemAt( itemcount - 4); // undock
      m_pMainMenuBar->removeItemAt( itemcount - 5); // separator
   }
   m_pMainMenuBar->removeItemAt( 0); // system menu
}

/** Shows the view taskbar. This should be connected with your "View" menu. */
void QextMdiMainFrm::showViewTaskBar()
{
   m_pTaskBar->show();
}

/** Hides the view taskbar. This should be connected with your "View" menu. */
void QextMdiMainFrm::hideViewTaskBar()
{
   m_pTaskBar->hide();
}
