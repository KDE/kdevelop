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

#include "qextmdimainfrm.h"
//#include "kvi_menubar.h"
//#include "kvi_toolbar.h"
//#include "kvi_statusbar.h"
#include "qextmditaskbar.h"
#include "qextmdichildfrm.h"
#include "qextmdichildarea.h"
#include "qextmdichildview.h"

#include <qcursor.h>
#include <qclipboard.h>
#include <qobjcoll.h>
#include <qtoolbutton.h>

//###########################################
//
// CREATION PROCESS
//
//###########################################
/*F.B.
//This comes from kvi_app.cpp
extern QPopupMenu *g_pTaskBarPopup;
extern QPopupMenu *g_pViewSelectPopup;
extern QPopupMenu *g_pWindowPopup;
F.B.*/
//============ constructor ============//

QextMdiMainFrm::QextMdiMainFrm(QWidget* parentWidget, const char* name, WFlags flags)
: QMainWindow( parentWidget, name, flags),
m_pCurrentWindow(0),
m_pCurrentActiveWindow( 0)
{
	// Create the local list of windows
	m_pWinList = new QList<QextMdiChildView>;
	m_pWinList->setAutoDelete(false);
	// This seems to be needed (re-check it after Qt2.0 comed out)
	setFocusPolicy(ClickFocus);
	
	// And start creating self
	createMdiManager();
	createMenuBar();
	createToolBar();
	createStatusBar();
	createTaskBar();
	
	// Apply options for the MDI manager
	applyOptions();

	// No active window yet (when shown , the console should be the active one)
//F.B.	updateCaption();

	// Set the geometry
//F.B.	setGeometry(g_pOptions->m_rectFrameGeometry); //does not work with KWM... ???

/*F.B.	bool bHalt = false;

	if(g_pEventManager->m_bEventEnabled[KviEvent_OnStartup]){
		bHalt = m_pUserParser->callEvent(KviEvent_OnStartup,m_pConsole,KviStr(KVI_VERSION));
	}

	if(!bHalt)m_pConsole->output(KVI_OUT_KVIRC,KVI_VERSION);
F.B.*/
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

//F.B.	m_pToolBar->m_pSysTray->repaint();
}

//============ createMdiManager ============//
void QextMdiMainFrm::createMdiManager()
{
	m_pMdi=new QextMdiChildArea(this);
	setCentralWidget(m_pMdi);
	QObject::connect( m_pMdi, SIGNAL(topChildChanged(QextMdiChildView*)), this, SLOT(pushNewTaskBarButton(QextMdiChildView*)) );
}

//============ createMenuBar ============//
void QextMdiMainFrm::createMenuBar()
{
	//Here we create the menu bar
//F.B.	m_pMenuBar = new KviMenuBar(this);
}

//============ createToolBar ============//
void QextMdiMainFrm::createToolBar()
{
	//Here we create the tool bar
/*F.B.	m_pToolBar = new KviToolBar(this,QMainWindow::Top);
	if(!g_pOptions->m_bToolBarVisible)m_pToolBar->hide();
	setRightJustification(true);
	applyToolbarOptions(); F.B.*/
}

void QextMdiMainFrm::applyToolbarOptions()
{
/*F.B.	setUsesBigPixmaps(g_pOptions->m_bUseBigToolbarPixmaps);
	QObjectList * l = m_pToolBar->queryList("QToolButton",0,false);
	QObjectListIt it(*l);
	while(it.current()){
		((QToolButton *)it.current())->setUsesTextLabel(g_pOptions->m_bUseTextToolbarComment);
		++it;
	}
	delete l;   F.B.*/
}

void QextMdiMainFrm::slot_toggleToolBar()
{
/*F.B.	if(m_pToolBar->isVisible()){
		m_pToolBar->hide();
		g_pOptions->m_bToolBarVisible = false;
	} else {
		m_pToolBar->show();
		g_pOptions->m_bToolBarVisible = true;
	}  F.B.*/
}

//============ createStatusBar ============//
void QextMdiMainFrm::createStatusBar()
{
/*F.B.	m_pStatusBar = new KviStatusBar(this);
	if(g_pOptions->m_bStatusBarVisible)m_pStatusBar->show();
	else m_pStatusBar->hide();
F.B.*/
}

void QextMdiMainFrm::slot_toggleStatusBar()
{
/*F.B.	if(m_pStatusBar->isVisible()){
		m_pStatusBar->hide();
		g_pOptions->m_bStatusBarVisible = false;
	} else {
		m_pStatusBar->show();
		g_pOptions->m_bStatusBarVisible = true;
	}  F.B.*/
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

/*F.B. void QextMdiMainFrm::fillRecentPopup(QPopupMenu * popup,KviStr &data)
{
	popup->clear();
	const char *aux = data.ptr();
	KviStr token;
	while(*aux){
		aux = kvi_extractToken(token,aux,',');
		if(token.hasData()){
			popup->insertItem(token.ptr());
		}
	}
} F.B.*/

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
	while((pWnd = m_pWinList->first()))closeWindow(pWnd);
	delete m_pWinList;
//F.B.	g_pApp->removeFrame(this);
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
void QextMdiMainFrm::addWindow(QextMdiChildView *pWnd,bool bShow,bool bAttach)
{
	QObject::connect( pWnd, SIGNAL(attachWindow(QextMdiChildView*,bool,bool,QRect*)), this, SLOT(attachWindow(QextMdiChildView*,bool,bool,QRect*)) );
	QObject::connect( pWnd, SIGNAL(detachWindow(QextMdiChildView*)), this, SLOT(detachWindow(QextMdiChildView*)) );
	QObject::connect( pWnd, SIGNAL(focusInEventOccurs(QextMdiChildView*)), this, SLOT(pushNewTaskBarButton(QextMdiChildView*)) );
	QObject::connect( pWnd, SIGNAL(childWindowCloseRequest(QextMdiChildView*)), this, SLOT(childWindowCloseRequest(QextMdiChildView*)) );
	
	//The window can be added only once :)
	m_pWinList->append(pWnd);
	QextMdiTaskBarButton* but = m_pTaskBar->addWinButton(pWnd);
	QObject::connect( pWnd, SIGNAL(windowCaptionChanged(const QString&)), but, SLOT(setNewText(const QString&)) );

//F.B.   m_pTaskBar->setActiveButton(pWnd);
/*F.B.	QextMdiChildViewPackedProperty * prop = g_pOptions->m_pWinPropertiesList->findProperty(pWnd->caption());
	if(prop){
		QextMdiChildViewProperty p;
		g_pOptions->m_pWinPropertiesList->unpackProperty(&p,prop);
		pWnd->setProperties(&p);
		if(p.isDocked){
			attachWindow(pWnd,bShow,false,&(p.rect)); //attach , do not cascade
			if(p.isMaximized)pWnd->maximize();
		} else {
			pWnd->setGeometry(p.rect);
			if(bShow)pWnd->show();
			pWnd->youAreDetached();
			if(bShow)pWnd->setFocus();
		}
	} else F.B.*/
	if( bAttach)
   	attachWindow(pWnd,bShow,true,0);
}

//============ attachWindow ============//
void QextMdiMainFrm::attachWindow(QextMdiChildView *pWnd,bool bShow,bool overrideGeometry,QRect *r)
{
	//Creates a new QextMdiChildFrm in m_pMdi manager
	//Attaches the QextMdiChildView to that child and starts the manager
	m_pCurrentWindow = m_pCurrentActiveWindow = pWnd;
	QextMdiChildFrm *lpC=new QextMdiChildFrm(m_pMdi);
	lpC->setClient(pWnd);
	pWnd->youAreAttached(lpC);
//F.B.	m_pTaskBar->windowAttached(pWnd,true);
//F.B.	m_pTaskBar->setActiveButton(pWnd);  //F.B.
	if(!overrideGeometry && r)lpC->setGeometry(*r);
	m_pMdi->manageChild(lpC,bShow,overrideGeometry);
   pushNewTaskBarButton( pWnd);
//--	m_pMdi->setFocus();
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
//F.B.	m_pTaskBar->windowAttached(pWnd,false);
	m_pTaskBar->setActiveButton(pWnd);  //F.B.
	m_pMdi->destroyChild(lpC,false); //Do not focus the new top child , we loose focus...
//--	pWnd->setFocus();
}

//============== removeWindowFromMdi ==============//
void QextMdiMainFrm::removeWindowFromMdi(QextMdiChildView *pWnd)
{
	QObject::disconnect( pWnd, SIGNAL(attachWindow(QextMdiChildView*,bool,bool,QRect*)), this, SLOT(attachWindow(QextMdiChildView*,bool,bool,QRect*)) );
	QObject::disconnect( pWnd, SIGNAL(detachWindow(QextMdiChildView*)), this, SLOT(detachWindow(QextMdiChildView*)) );
	QObject::disconnect( pWnd, SIGNAL(focusInEventOccurs(QextMdiChildView*)), this, SLOT(pushNewTaskBarButton(QextMdiChildView*)) );
	QObject::disconnect( pWnd, SIGNAL(childWindowCloseRequest(QextMdiChildView*)), this, SLOT(childWindowCloseRequest(QextMdiChildView*)) );
	
	//Closes a child window. sends no close event : simply deletes it
//F.B.	pWnd->saveProperties();
	m_pWinList->removeRef(pWnd);
	if(pWnd == m_pCurrentActiveWindow)
	m_pCurrentActiveWindow = ((pWnd == m_pCurrentWindow) ? 0 : m_pCurrentWindow);
	
	QextMdiTaskBarButton* but = m_pTaskBar->getButton(pWnd);
	QObject::disconnect( pWnd, SIGNAL(windowCaptionChanged(const QString&)), but, SLOT(setNewText(const QString&)) );
	m_pTaskBar->removeWinButton(pWnd);
	
	if(pWnd->isAttached())m_pMdi->destroyChildButNotItsView(pWnd->mdiParent());
}

//============== closeWindow ==============//
void QextMdiMainFrm::closeWindow(QextMdiChildView *pWnd)
{
	QObject::disconnect( pWnd, SIGNAL(attachWindow(QextMdiChildView*,bool,bool,QRect*)), this, SLOT(attachWindow(QextMdiChildView*,bool,bool,QRect*)) );
	QObject::disconnect( pWnd, SIGNAL(detachWindow(QextMdiChildView*)), this, SLOT(detachWindow(QextMdiChildView*)) );
	QObject::disconnect( pWnd, SIGNAL(focusInEventOccurs(QextMdiChildView*)), this, SLOT(pushNewTaskBarButton(QextMdiChildView*)) );
	QObject::disconnect( pWnd, SIGNAL(childWindowCloseRequest(QextMdiChildView*)), this, SLOT(childWindowCloseRequest(QextMdiChildView*)) );
	
	//Closes a child window. sends no close event : simply deletes it
//F.B.	pWnd->saveProperties();
	m_pWinList->removeRef(pWnd);
	if(pWnd == m_pCurrentActiveWindow)
	m_pCurrentActiveWindow = ((pWnd == m_pCurrentWindow) ? 0 : m_pCurrentWindow);
	
	QextMdiTaskBarButton* but = m_pTaskBar->getButton(pWnd);
	QObject::disconnect( pWnd, SIGNAL(windowCaptionChanged(const QString&)), but, SLOT(setNewText(const QString&)) );
	m_pTaskBar->removeWinButton(pWnd);
	
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
	return m_pCurrentActiveWindow ? m_pCurrentActiveWindow : m_pCurrentWindow;
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

//################################################
// The following functions are called DIRECTLY
// from the KviInput class.
//################################################
void QextMdiMainFrm::switchWindows(bool bRight)
{
	QextMdiChildFrm * pActMdi = m_pMdi->topChild();
	if(!pActMdi)return;
	QextMdiChildView * pAct = (QextMdiChildView *)pActMdi->m_pClient;
	if(!pAct)return;
//F.B.	QToolButton * pNext = m_pTaskBar->getNextWindowButton(bRight,pAct);
//F.B.	if(!pNext)return;
//F.B.	if(pNext->m_pWindow->isAttached())taskbarButtonLeftClicked(pNext->m_pWindow,pNext);
//F.B.	else pNext->m_pWindow->attach();
	bRight = false; // dummy!, only to avoid "unused parameter"
}
//################################################
// The following functions are called DIRECTLY
// from the KviTaskBar class.
//################################################
void QextMdiMainFrm::taskbarButtonLeftClicked(QextMdiChildView *pWnd)
{
	if(pWnd->isAttached()){
		if(pWnd->hasFocus() && (pWnd->mdiParent() == m_pMdi->topChild())){
			//F.B. btn->setOn(false);
			//F.B. pWnd->minimize();
		} else {
			//F.B. btn->setOn(true);
			pWnd->restore();
			pWnd->setFocus();
		}
	} //F.B.else { //not so cool...bu can not do more...
//F.B.	btn->setOn(true);
		/*F.B. g_pApp-> F.B.*/raiseTopLevelWidget(pWnd);
//F.B.	}
   childWindowGainFocus(pWnd);
}
void QextMdiMainFrm::taskbarButtonRightClicked(QextMdiChildView *pWnd)
{
	taskbarButtonLeftClicked( pWnd); // set focus
	taskBarPopup( pWnd, true)->popup( QCursor::pos());
}

//################################################
// The following functions are called DIRECTLY
// from the QextMdiChildView class.
//################################################

void QextMdiMainFrm::childWindowGainFocus(QextMdiChildView *pWnd)
{
	m_pCurrentActiveWindow = pWnd;
	m_pTaskBar->setActiveButton(pWnd);
}

void QextMdiMainFrm::childWindowCloseRequest(QextMdiChildView *pWnd)
{
	if(pWnd==m_pCurrentWindow) {
		//closeWindow(pWnd);
		QextMdiViewCloseEvent* ce = new QextMdiViewCloseEvent( pWnd);
		QApplication::postEvent( this, ce);
	}
}
/*F.B.
bool QextMdiMainFrm::checkHighlight(const char *msg)
{
	for(KviStr * s=g_pOptions->m_pHighlightWords->first();s;s=g_pOptions->m_pHighlightWords->next()){
		const char *aux = msg;
		char c = *(s->ptr()); //newer \0 !
		// Run thru the string
		while(*aux){
			while(*aux  && (*aux != c))aux++; //run until we find a probable match
			if(*aux){
				// probable match...check
				if(kvi_strEqualCIN(s->ptr(),aux,s->len()))return true;
				else aux++;
			} // else finished the string
		}
	}
	return false;
}
F.B.*/
//##################################################
// Internal handlers
//##################################################

void QextMdiMainFrm::focusInEvent(QFocusEvent *)
{
	m_pMdi->setFocus();
}

void QextMdiMainFrm::raiseTopLevelWidget(QWidget * ptr)
{
   if(ptr->hasFocus())return;
   if(ptr->isActiveWindow())return;
   ptr->show();
   ptr->raise();
   ptr->setActiveWindow();
//!F.B.   ptr->setFocus();
}

void QextMdiMainFrm::pushNewTaskBarButton( QextMdiChildView* pWnd)
{
   m_pCurrentWindow = m_pCurrentActiveWindow = pWnd;
   taskbarButtonLeftClicked( m_pCurrentActiveWindow);
}

bool QextMdiMainFrm::event( QEvent* e)
{
#if QT_VERSION >= 200
	if( e->type() == QEvent::User) {
#else
	if( e->type() == Event_User) {
#endif
		QextMdiChildView* pWnd = (QextMdiChildView*)((QextMdiViewCloseEvent*)e)->data();
		if( pWnd != 0)
			closeWindow( pWnd);
		return true;
	}
	return QMainWindow::event( e);		
}

