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

#include <math.h>
#include <qpopupmenu.h>

#include "qextmdidefines.h"
#include "qextmdichildarea.h"

///////////////////////////////////////////////////////////////////////////////
// QextMdiChildArea
///////////////////////////////////////////////////////////////////////////////

//============ QextMdiChildArea ============//

QextMdiChildArea::QextMdiChildArea(QWidget *parent)
:QFrame(parent, "qextmdi_childarea")
{
	setFrameStyle(QFrame::Panel|QFrame::Sunken);
	m_captionFont = QFont();//F.B.QFont("clean",16);
	QFontMetrics fm(m_captionFont);
	m_captionFontLineSpacing = fm.lineSpacing();
	m_captionActiveBackColor = QColor(0,0,0);
	m_captionActiveForeColor = QColor(80,255,0);
	m_captionInactiveBackColor = QColor(80,80,80);
	m_captionInactiveForeColor = QColor(200,200,200);
	m_pZ = new QList<QextMdiChildFrm>;
	m_pZ->setAutoDelete(true);
	setFocusPolicy(ClickFocus);
	m_pWindowMenu = new QPopupMenu();
	m_pWindowMenu->setCheckable( true);
	//QObject::connect(m_pWindowMenu,SIGNAL(activated(int)),this,SLOT(menuActivated(int)));
	fillWindowMenu();
}

void QextMdiChildArea::setMdiCaptionFont(const QFont &fnt)
{
	m_captionFont = fnt;
	QFontMetrics fm(m_captionFont);
	m_captionFontLineSpacing = fm.lineSpacing();
}
void QextMdiChildArea::setMdiCaptionActiveForeColor(const QColor &clr)
{ m_captionActiveForeColor = clr; }
void QextMdiChildArea::setMdiCaptionActiveBackColor(const QColor &clr)
{ m_captionActiveBackColor = clr; }
void QextMdiChildArea::setMdiCaptionInactiveForeColor(const QColor &clr)
{ m_captionInactiveForeColor = clr; }
void QextMdiChildArea::setMdiCaptionInactiveBackColor(const QColor &clr)
{ m_captionInactiveBackColor = clr; }

//============ ~QextMdiChildArea ============//

QextMdiChildArea::~QextMdiChildArea()
{
	delete m_pWindowMenu;
	delete m_pZ; //This will destroy all the widgets inside.
}

//============ manageChild ============//

void QextMdiChildArea::manageChild(QextMdiChildFrm *lpC,bool bShow,bool bCascade)
{
	QextMdiChildFrm * top=topChild();
	if(bShow)m_pZ->append(lpC); //visible -> first in the Z order
	else m_pZ->insert(0,lpC); //hidden -> last in the Z order
	if(bCascade)lpC->move(getCascadePoint(m_pZ->count()-1));
	if(bShow){
		if(top){ //Maximize if needed
			if(top->state() == QextMdiChildFrm::Maximized){
				top->setState(QextMdiChildFrm::Normal,false);
				lpC->setState(QextMdiChildFrm::Maximized,false);
			}
		}
		lpC->show();
	}
	fillWindowMenu();
	focusTopChild();  //F.B.
	//if(bShow) lpC->setFocus();
}

//============= focusInEvent ===============//

void QextMdiChildArea::focusInEvent(QFocusEvent *)
{
   //qDebug("ChildArea::focusInEvent");
	focusTopChild();
}

//============ destroyChild ============//

void QextMdiChildArea::destroyChild(QextMdiChildFrm *lpC,bool bFocusTopChild)
{
	bool bWasMaximized = lpC->state() == QextMdiChildFrm::Maximized;
	QObject::disconnect(lpC);
	lpC->blockSignals(true);
	m_pZ->removeRef(lpC);
	if(bWasMaximized){
		QextMdiChildFrm * c=topChild();
		if(c)c->setState(QextMdiChildFrm::Maximized,false);
	}
	if(bFocusTopChild)focusTopChild();
	fillWindowMenu();
}

//============ destroyChildButNotItsView ============//

void QextMdiChildArea::destroyChildButNotItsView(QextMdiChildFrm *lpC,bool bFocusTopChild)
{
	bool bWasMaximized = lpC->state() == QextMdiChildFrm::Maximized;
	QObject::disconnect(lpC);

   lpC->unsetClient();

	lpC->blockSignals(true);
	m_pZ->removeRef(lpC);
	if(bWasMaximized){
		QextMdiChildFrm * c=topChild();
		if(c)c->setState(QextMdiChildFrm::Maximized,false);
	}
	if(bFocusTopChild)focusTopChild();
	fillWindowMenu();
}

//============= setTopChlid ============//

void QextMdiChildArea::setTopChild(QextMdiChildFrm *lpC,bool bSetFocus)
{
	if(m_pZ->last() != lpC){
      //qDebug("setTopChild");
		m_pZ->setAutoDelete(false);
		m_pZ->removeRef(lpC);
		//disable the labels of all the other children
		for(QextMdiChildFrm *pC=m_pZ->first();pC;pC=m_pZ->next()){
			pC->m_pCaption->setActive(false);
		}
		QextMdiChildFrm *pMaximizedChild=m_pZ->last();
		if(pMaximizedChild->m_state != QextMdiChildFrm::Maximized)pMaximizedChild=0;
		m_pZ->setAutoDelete(true);
		m_pZ->append(lpC);
		if(pMaximizedChild)lpC->setState(QextMdiChildFrm::Maximized,false); //do not animate the change
		lpC->raise();
		if(pMaximizedChild){
			pMaximizedChild->setState(QextMdiChildFrm::Normal,false);
		}
		if(bSetFocus){
			if(!lpC->hasFocus())lpC->setFocus();
		}
		fillWindowMenu();
		///emit topChildChanged( lpC->m_pClient);
		lpC->m_pClient->setFocus();
	}
}

//============== resizeEvent ================//

void QextMdiChildArea::resizeEvent(QResizeEvent *e)
{
	//If we have a maximized children at the top , adjust its size
//	QScrollView::resizeEvent(e);
	QextMdiChildFrm *lpC=m_pZ->last();
	if(lpC){
		if(lpC->m_state == QextMdiChildFrm::Maximized)
			lpC->resize( width() + lpC->width() - lpC->m_pClient->width(),
							 height() + lpC->height() - lpC->m_pClient->height());
	}
	e = 0; // dummy!, only to avoid "unused parameters"
}

//=============== mousePressEvent =============//

void QextMdiChildArea::mousePressEvent(QMouseEvent *e)
{
	//Popup the window menu
	if(e->button() & RightButton)m_pWindowMenu->popup(mapToGlobal(e->pos()));
}

//=============== getCascadePoint ============//

QPoint QextMdiChildArea::getCascadePoint(int indexOfWindow)
{
	QPoint pnt(0,0);
	if(indexOfWindow==0)return pnt;
	QextMdiChildFrm *lpC=m_pZ->first();
	int step=(lpC ? lpC->m_pCaption->heightHint()+QEXTMDI_MDI_CHILDFRM_BORDER : 20);
	int availableHeight=height()-(lpC ? lpC->minimumSize().height() : QEXTMDI_MDI_CHILDFRM_MIN_HEIGHT);
	int availableWidth=width()-(lpC ? lpC->minimumSize().width() : QEXTMDI_MDI_CHILDFRM_MIN_WIDTH);
	int ax=0;
	int ay=0;
	for(int i=0;i<indexOfWindow;i++){
		ax+=step;
		ay+=step;
		if(ax>availableWidth)ax=0;
		if(ay>availableHeight)ay=0;
	}
	pnt.setX(ax);
	pnt.setY(ay);
	return pnt;
}

//=============== fillWindowMenu ===============//
#include "iostream.h"
void QextMdiChildArea::fillWindowMenu()
{
	m_pWindowMenu->clear();
	m_pWindowMenu->insertItem(tr("&Close Window"),this, SIGNAL(closeActiveView()));
	m_pWindowMenu->insertItem(tr("Close &All Windows"), this, SIGNAL(closeAllViews()));
	m_pWindowMenu->insertSeparator();
	m_pWindowMenu->insertItem(tr("Ca&scade windows"),this,SLOT(cascadeWindows()));
	m_pWindowMenu->insertItem(tr("Cascade &maximized"),this,SLOT(cascadeMaximized()));
	m_pWindowMenu->insertItem(tr("Expand &vertical"),this,SLOT(expandVertical()));
	m_pWindowMenu->insertItem(tr("Expand &horizontal"),this,SLOT(expandHorizontal()));
	m_pWindowMenu->insertItem(tr("A&nodine's tile"),this,SLOT(tileAnodine()));
	m_pWindowMenu->insertItem(tr("&Pragma's tile"),this,SLOT(tilePragma()));
	m_pWindowMenu->insertSeparator();
	m_pWindowMenu->insertItem(tr("&Toplevel mode"),this, SIGNAL(switchToToplevelMode()));
	m_pWindowMenu->insertItem(tr("C&hildframe mode"), this, SIGNAL(switchToChildframeMode()));
	m_pWindowMenu->insertSeparator();

	// for all child frame windows: give an ID to every window and connect them in the end with menuActivated()
	int i=100;
	bool isTheActiveOne;
	for(QextMdiChildFrm *lpC=m_pZ->first();lpC;lpC=m_pZ->next())
	{
	   isTheActiveOne = false;
		QString szItem;
		//szItem.setNum(((uint)i)-99);
		//szItem+=". ";
		if(lpC->m_state==QextMdiChildFrm::Minimized){
			szItem+="(";
			szItem+=lpC->caption();
			szItem+=")";
		}
		else
			if(((uint)i)==(m_pZ->count()+99)){ //Active item :)
				szItem+=" ";
				szItem+=lpC->caption();
				isTheActiveOne = true;
			}
			else {
				szItem+=" ";
				szItem+=lpC->caption();
			}

		// insert the window entry sorted in alphabetical order
		unsigned int indx;
		unsigned int windowItemCount = m_pWindowMenu->count() - 13;
		bool inserted = false;
		QString tmpString;
		for( indx = 0; indx <= windowItemCount; indx++) {
			tmpString = m_pWindowMenu->text( m_pWindowMenu->idAt( indx+13));
			if( tmpString.right( tmpString.length()-2) > szItem.right( szItem.length()-2)) {
				m_pWindowMenu->insertItem(szItem,lpC,SLOT(slot_clickedInWindowMenu()),0,-1,indx+13);
      		if( isTheActiveOne)
      		   m_pWindowMenu->setItemChecked( m_pWindowMenu->idAt( indx+13), true);
				lpC->setWindowMenuID( i);
				inserted = true;
				indx = windowItemCount+1;	// break the loop
			}
		}
		if( !inserted) {	// append it
			m_pWindowMenu->insertItem( szItem,lpC,SLOT(slot_clickedInWindowMenu()),0,-1,windowItemCount+13);
     		if( isTheActiveOne)
     		   m_pWindowMenu->setItemChecked( m_pWindowMenu->idAt( windowItemCount+13), true);
			lpC->setWindowMenuID( i);
		}
		i++;
	}
}

//================ menuActivated ===============//

void QextMdiChildArea::menuActivated(int id)
{
	if(id<100)return;
	id-=100;
	QextMdiChildFrm *lpC=m_pZ->at(id);
	if(!lpC)return;
	if(lpC->m_state==QextMdiChildFrm::Minimized)lpC->minimizePressed();
	if(lpC==m_pZ->last())return;
	setTopChild(lpC,true);
}

//================ childMinimized ===============//

void QextMdiChildArea::childMinimized(QextMdiChildFrm *lpC,bool bWasMaximized)
{
	if(m_pZ->findRef(lpC) == -1)return;
	if(m_pZ->count() > 1){
		m_pZ->setAutoDelete(false);
		m_pZ->removeRef(lpC);
		m_pZ->setAutoDelete(true);
		m_pZ->insert(0,lpC);
		if(bWasMaximized){
			// Need to maximize the top child
			lpC = m_pZ->last();
			if(!lpC)return; //??
			if(lpC->m_state==QextMdiChildFrm::Minimized)return;
			lpC->setState(QextMdiChildFrm::Maximized,false); //do not animate the change
		}
		focusTopChild();
	} else {
		setFocus(); //Remove focus from the child
	}
	fillWindowMenu();
}

//============= focusTopChild ===============//

void QextMdiChildArea::focusTopChild()
{
	QextMdiChildFrm *lpC=m_pZ->last();
	if(!lpC)return;
	if(lpC->m_state==QextMdiChildFrm::Minimized)return;
	//disable the labels of all the other children
	for(QextMdiChildFrm *pC=m_pZ->first();pC;pC=m_pZ->next()){
		if(pC != lpC)pC->m_pCaption->setActive(false);
	}
	lpC->raise();
	if(!lpC->hasFocus())lpC->setFocus();
	///emit topChildChanged(lpC->m_pClient);
	lpC->m_pClient->setFocus();
}

//============= cascadeWindows ===============//

void QextMdiChildArea::cascadeWindows()
{
	int idx=0;
	QList<QextMdiChildFrm> list(*m_pZ);
	list.setAutoDelete(false);
	while(!list.isEmpty()){
		QextMdiChildFrm *lpC=list.first();
		if(lpC->m_state != QextMdiChildFrm::Minimized){
			if(lpC->m_state==QextMdiChildFrm::Maximized)lpC->setState(QextMdiChildFrm::Normal,false);
			lpC->move(getCascadePoint(idx));
			lpC->resize(lpC->minimumSize());
			idx++;
		}
		list.removeFirst();
	}
	focusTopChild();
}

//============= cascadeMaximized ===============//

void QextMdiChildArea::cascadeMaximized()
{
	int idx=0;
	QList<QextMdiChildFrm> list(*m_pZ);

	list.setAutoDelete(false);
	while(!list.isEmpty()){
		QextMdiChildFrm *lpC=list.first();
		if(lpC->m_state != QextMdiChildFrm::Minimized){
			if(lpC->m_state==QextMdiChildFrm::Maximized)lpC->setState(QextMdiChildFrm::Normal,false);
			QPoint pnt(getCascadePoint(idx));
			lpC->move(pnt);
			QSize curSize(width()-pnt.x(),height()-pnt.y());
			if((lpC->minimumSize().width() > curSize.width()) ||
				(lpC->minimumSize().height() > curSize.height()))lpC->resize(lpC->minimumSize());
			else lpC->resize(curSize);
			idx++;
		}
		list.removeFirst();
	}
	focusTopChild();
}

void QextMdiChildArea::expandVertical()
{
	int idx=0;
	QList<QextMdiChildFrm> list(*m_pZ);
	list.setAutoDelete(false);
	while(!list.isEmpty()){
		QextMdiChildFrm *lpC=list.first();
		if(lpC->m_state != QextMdiChildFrm::Minimized){
			if(lpC->m_state==QextMdiChildFrm::Maximized)lpC->setState(QextMdiChildFrm::Normal,false);
			lpC->setGeometry(lpC->x(),0,lpC->width(),height());
			idx++;
		}
		list.removeFirst();
	}
	focusTopChild();
}

void QextMdiChildArea::expandHorizontal()
{
	int idx=0;
	QList<QextMdiChildFrm> list(*m_pZ);
	list.setAutoDelete(false);
	while(!list.isEmpty()){
		QextMdiChildFrm *lpC=list.first();
		if(lpC->m_state != QextMdiChildFrm::Minimized){
			if(lpC->m_state==QextMdiChildFrm::Maximized)lpC->setState(QextMdiChildFrm::Normal,false);
			lpC->setGeometry(0,lpC->y(),width(),lpC->height());
			idx++;
		}
		list.removeFirst();
	}
	focusTopChild();
}

//============= getVisibleChildCount =============//

int QextMdiChildArea::getVisibleChildCount()
{
	int cnt=0;
	for(QextMdiChildFrm *lpC=m_pZ->first();lpC;lpC=m_pZ->next()){
		if(lpC->m_state != QextMdiChildFrm::Minimized)cnt++;
	}
	return cnt;
}

//============ tilePragma ============//

void QextMdiChildArea::tilePragma()
{
	tileAllInternal(9);
}

//============ tileAllInternal ============//

void QextMdiChildArea::tileAllInternal(int maxWnds)
{
	//NUM WINDOWS =           1,2,3,4,5,6,7,8,9
	static int colstable[9]={ 1,1,1,2,2,2,3,3,3 }; //num columns
	static int rowstable[9]={ 1,2,3,2,3,3,3,3,3 }; //num rows
	static int lastwindw[9]={ 1,1,1,1,2,1,3,2,1 }; //last window multiplier
	static int colrecall[9]={ 0,0,0,3,3,3,6,6,6 }; //adjust self
	static int rowrecall[9]={ 0,0,0,0,4,4,4,4,4 }; //adjust self

	QextMdiChildFrm *lpTop=topChild();
	int numVisible=getVisibleChildCount();
	if(numVisible<1)return;
	int numToHandle=((numVisible > maxWnds) ? maxWnds : numVisible);
	int xQuantum=width()/colstable[numToHandle-1];
	if(xQuantum < ((lpTop->minimumSize().width() > QEXTMDI_MDI_CHILDFRM_MIN_WIDTH) ? lpTop->minimumSize().width() : QEXTMDI_MDI_CHILDFRM_MIN_WIDTH)){
		if(colrecall[numToHandle-1]==0)debug("Tile : Not enouh space");
		else tileAllInternal(colrecall[numToHandle-1]);
		return;
	}
	int yQuantum=height()/rowstable[numToHandle-1];
	if(yQuantum < ((lpTop->minimumSize().height() > QEXTMDI_MDI_CHILDFRM_MIN_HEIGHT) ? lpTop->minimumSize().height() : QEXTMDI_MDI_CHILDFRM_MIN_HEIGHT)){
		if(rowrecall[numToHandle-1]==0)debug("Tile : Not enough space");
		else tileAllInternal(rowrecall[numToHandle-1]);
		return;
	}
	int curX=0;
	int curY=0;
	int curRow=1;
	int curCol=1;
	int curWin=1;
	for(QextMdiChildFrm *lpC=m_pZ->first();lpC;lpC=m_pZ->next()){
		if(lpC->m_state!=QextMdiChildFrm::Minimized){
			//restore the window
			if(lpC->m_state==QextMdiChildFrm::Maximized)lpC->setState(QextMdiChildFrm::Normal,false);
			if((curWin%numToHandle)==0)lpC->setGeometry(curX,curY,xQuantum * lastwindw[numToHandle-1],yQuantum);
			else lpC->setGeometry(curX,curY,xQuantum,yQuantum);
			//example : 12 windows : 3 cols 3 rows
			if(curCol<colstable[numToHandle-1]){ //curCol<3
				curX+=xQuantum; //add a column in the same row
				curCol++;       //increase current column
			} else {
				curX=0;         //new row
				curCol=1;       //column 1
				if(curRow<rowstable[numToHandle-1]){ //curRow<3
					curY+=yQuantum; //add a row
					curRow++;       //
				} else {
					curY=0;         //restart from beginning
					curRow=1;       //
				}
			}
			curWin++;
		}
	}
	if(lpTop)lpTop->setFocus();
}
//============ tileAnodine ============//
void QextMdiChildArea::tileAnodine()
{
	QextMdiChildFrm *lpTop=topChild();
	int numVisible=getVisibleChildCount(); // count visible windows
	if(numVisible<1)return;
	int numCols=int(sqrt(numVisible)); // set columns to square root of visible count
	// create an array to form grid layout
	int *numRows=new int[numCols];
	int numCurCol=0;
	while(numCurCol<numCols){
		numRows[numCurCol]=numCols; // create primary grid values
		numCurCol++;
	}
	int numDiff=numVisible-(numCols*numCols); // count extra rows
	int numCurDiffCol=numCols; // set column limiting for grid updates
	while(numDiff>0){
		numCurDiffCol--;
		numRows[numCurDiffCol]++; // add extra rows to column grid
		if(numCurDiffCol<1)numCurDiffCol=numCols; // rotate through the grid
		numDiff--;
	}
	numCurCol=0;
	int numCurRow=0;
	int curX=0;
	int curY=0;
	// the following code will size everything based on my grid above
	// there is no limit to the number of windows it will handle
	// it's great when a kick-ass theory works!!!                      // Pragma :)
	int xQuantum=width()/numCols;
	int yQuantum=height()/numRows[numCurCol];
	for(QextMdiChildFrm *lpC=m_pZ->first();lpC;lpC=m_pZ->next()){
		if(lpC->m_state != QextMdiChildFrm::Minimized){
			if(lpC->m_state==QextMdiChildFrm::Maximized)lpC->setState(QextMdiChildFrm::Normal,false);
			lpC->setGeometry(curX,curY,xQuantum,yQuantum);
			numCurRow++;
			curY+=yQuantum;
			if(numCurRow==numRows[numCurCol]){
				numCurRow=0;
				numCurCol++;
				curY=0;
				curX+=xQuantum;
				if(numCurCol!=numCols)yQuantum=height()/numRows[numCurCol];
			}
		}
	}
	delete[] numRows;
	if(lpTop)lpTop->setFocus();
}

void QextMdiChildArea::undockWindow(QWidget* pWidget)   // added by F.B.
{
   QextMdiChildView* pWnd = (QextMdiChildView*) pWidget;   // bad solution??? F.B.

	if(!pWnd->parent())return;
	QextMdiChildFrm *lpC=pWnd->mdiParent();
	lpC->unsetClient();
	pWnd->youAreDetached();
   //m_pTaskBar->windowAttached(pWnd,false);
	destroyChild(lpC,false); //Do not focus the new top child , we loose focus...
	pWnd->setFocus();
}
