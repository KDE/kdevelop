//----------------------------------------------------------------------------
//    filename             : qextmdichildarea.cpp
//----------------------------------------------------------------------------
//    Project              : Qt MDI extension
//
//    begin                : 07/1999       by Szymon Stefanek as part of kvirc
//                                         (an IRC application)
//    changes              : 09/1999       by Falk Brettschneider to create an
//                           - 06/2000     stand-alone Qt extension set of
//                                         classes and a Qt-based library
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
   m_captionActiveBackColor = colorGroup().highlight();//QColor(0,0,128);
   m_captionActiveForeColor = colorGroup().highlightedText();//QColor(255,255,255);
   m_captionInactiveBackColor = QColor(160,160,160);
   m_captionInactiveForeColor = QColor( 55, 55, 55);
#if QT_VERSION < 300
   m_pZ = new QList<QextMdiChildFrm>;
#else
   m_pZ = new QPtrList<QextMdiChildFrm>;
#endif
   m_pZ->setAutoDelete(TRUE);
   setFocusPolicy(ClickFocus);
   m_defaultChildFrmSize = QSize(400,300);
}

//============ ~QextMdiChildArea ============//

QextMdiChildArea::~QextMdiChildArea()
{
   delete m_pZ; //This will destroy all the widgets inside.
}

//============ manageChild ============//

void QextMdiChildArea::manageChild(QextMdiChildFrm *lpC,bool bShow,bool bCascade)
{
   QextMdiChildFrm * top=topChild();
   if (bShow)
      m_pZ->append(lpC); //visible -> first in the Z order
   else
      m_pZ->insert(0,lpC); //hidden -> last in the Z order

   if(bCascade)lpC->move(getCascadePoint(m_pZ->count()-1));
   if(bShow){
      if(top){ //Maximize if needed
         if(top->state() == QextMdiChildFrm::Maximized){
            emit sysButtonConnectionsMustChange( top,lpC);
            top->setState(QextMdiChildFrm::Normal,FALSE);
            lpC->setState(QextMdiChildFrm::Maximized,FALSE);
         }
      }
      lpC->show();
      focusTopChild();
   }
}

//============ destroyChild ============//

void QextMdiChildArea::destroyChild(QextMdiChildFrm *lpC,bool bFocusTopChild)
{
   bool bWasMaximized = lpC->state() == QextMdiChildFrm::Maximized;

   // destroy the old one
   QObject::disconnect(lpC);
   lpC->blockSignals(TRUE);
   m_pZ->setAutoDelete(FALSE);
   m_pZ->removeRef(lpC);

   // focus the next new childframe
   QextMdiChildFrm* newTopChild = topChild();
   if (bWasMaximized){
      if (newTopChild) {
         newTopChild->setState(QextMdiChildFrm::Maximized,FALSE);
         emit sysButtonConnectionsMustChange(lpC, newTopChild);
      }
      else {
         emit noMaximizedChildFrmLeft(lpC); // last childframe removed
      }
   }
   delete lpC;
   m_pZ->setAutoDelete(TRUE);

   if (bFocusTopChild)
      focusTopChild();
}

//============ destroyChildButNotItsView ============//

void QextMdiChildArea::destroyChildButNotItsView(QextMdiChildFrm *lpC,bool bFocusTopChild)
{
   bool bWasMaximized = lpC->state() == QextMdiChildFrm::Maximized;

   // destroy the old one
   QObject::disconnect(lpC);
   lpC->unsetClient();
   m_pZ->setAutoDelete(FALSE);
   m_pZ->removeRef(lpC);

   // focus the next new childframe
   QextMdiChildFrm* newTopChild = topChild();
   if (bWasMaximized){
      if (newTopChild) {
         newTopChild->setState(QextMdiChildFrm::Maximized,FALSE);
         emit sysButtonConnectionsMustChange(lpC, newTopChild);
      }
      else {
         emit noMaximizedChildFrmLeft(lpC); // last childframe removed
      }
   }
   delete lpC;
   m_pZ->setAutoDelete(TRUE);

   if (bFocusTopChild)
      focusTopChild();
}

//============= setTopChlid ============//

void QextMdiChildArea::setTopChild(QextMdiChildFrm *lpC,bool bSetFocus)
{
   if(m_pZ->last() != lpC){
      m_pZ->setAutoDelete(FALSE);
      if (lpC) {
         m_pZ->removeRef(lpC);
      }
      //disable the labels of all the other children
      for(QextMdiChildFrm *pC=m_pZ->first();pC;pC=m_pZ->next()){
         pC->m_pCaption->setActive(FALSE);
      }
      if (!lpC) {
         return;
      }

      QextMdiChildFrm *pMaximizedChild = m_pZ->last();
      if (pMaximizedChild->m_state != QextMdiChildFrm::Maximized) {
         pMaximizedChild = 0L;
      }
      m_pZ->setAutoDelete(TRUE);
      m_pZ->append(lpC);
      int nChildAreaMinW = 0,               nChildAreaMinH = 0;
      int nChildAreaMaxW = QWIDGETSIZE_MAX, nChildAreaMaxH = QWIDGETSIZE_MAX;
      if ( (pMaximizedChild != 0L) && (lpC->m_pClient != 0L) ) {
         nChildAreaMinW = lpC->m_pClient->minimumWidth();
         nChildAreaMinH = lpC->m_pClient->minimumHeight();
         // XXX TODO: setting the maximum size doesn't work properly - fix this later
         // nChildAreaMaxW = lpC->m_pClient->maximumWidth();
         // nChildAreaMaxH = lpC->m_pClient->maximumHeight();
      }
      setMinimumSize(nChildAreaMinW, nChildAreaMinH);
      setMaximumSize(nChildAreaMaxW, nChildAreaMaxH);
      if (pMaximizedChild) {
         const bool bDontAnimate = FALSE;
         // first maximize the new view
         lpC->setState(QextMdiChildFrm::Maximized, bDontAnimate);
         qApp->sendPostedEvents();
         // then restore the old maximized view in background
         pMaximizedChild->setState(QextMdiChildFrm::Normal, bDontAnimate);
         qApp->processOneEvent();
         emit sysButtonConnectionsMustChange( pMaximizedChild, lpC);
      }
      else {
         lpC->raise();
      }
      if (bSetFocus) {
//TEST         if(!lpC->hasFocus())lpC->setFocus();
      }
      QFocusEvent::setReason(QFocusEvent::Other);
      lpC->m_pClient->setFocus();
   }
}

//============== resizeEvent ================//

void QextMdiChildArea::resizeEvent(QResizeEvent* e)
{
   //If we have a maximized children at the top , adjust its size
   QextMdiChildFrm *lpC=m_pZ->last();
   if(lpC){
      if(lpC->m_state == QextMdiChildFrm::Maximized) {
         int clientw = 0, clienth = 0;
         if (lpC->m_pClient != 0L) {
            clientw = lpC->m_pClient->width();
            clienth = lpC->m_pClient->height();
         }
         lpC->resize( width() + QEXTMDI_MDI_CHILDFRM_DOUBLE_BORDER,
                      height() + lpC->m_pCaption->heightHint() + QEXTMDI_MDI_CHILDFRM_SEPARATOR + QEXTMDI_MDI_CHILDFRM_DOUBLE_BORDER);
      }
   }
   layoutMinimizedChildren();
   QWidget::resizeEvent(e);
}

//=============== mousePressEvent =============//

void QextMdiChildArea::mousePressEvent(QMouseEvent *e)
{
   //Popup the window menu
   if(e->button() & RightButton)
      emit popupWindowMenu( mapToGlobal( e->pos()));
}

//=============== getCascadePoint ============//

QPoint QextMdiChildArea::getCascadePoint(int indexOfWindow)
{
   if (indexOfWindow < 0) {
      indexOfWindow = m_pZ->count();
   }
   QPoint pnt(0,0);
   if(indexOfWindow==0)return pnt;

   bool bTopLevelMode = FALSE;
   if( height() == 1)	// hacky?!
   		bTopLevelMode = TRUE;

   QextMdiChildFrm *lpC=m_pZ->first();
   int step=(lpC ? lpC->m_pCaption->heightHint()+QEXTMDI_MDI_CHILDFRM_BORDER : 20);
   int h=(bTopLevelMode ? QApplication::desktop()->height() : height());
   int w=(bTopLevelMode ? QApplication::desktop()->width() : width());

   int availableHeight=h-(lpC ? lpC->minimumSize().height() : m_defaultChildFrmSize.height());
   int availableWidth=w-(lpC ? lpC->minimumSize().width() : m_defaultChildFrmSize.width());
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

//================ childMinimized ===============//

void QextMdiChildArea::childMinimized(QextMdiChildFrm *lpC,bool bWasMaximized)
{
   if(m_pZ->findRef(lpC) == -1)return;
   if(m_pZ->count() > 1){
      m_pZ->setAutoDelete(FALSE);
      m_pZ->removeRef(lpC);
      m_pZ->setAutoDelete(TRUE);
      m_pZ->insert(0,lpC);
      if(bWasMaximized){
         // Need to maximize the top child
         lpC = m_pZ->last();
         if(!lpC)return; //??
         if(lpC->m_state==QextMdiChildFrm::Minimized)return;
         lpC->setState(QextMdiChildFrm::Maximized,FALSE); //do not animate the change
      }
      focusTopChild();
   } else {
      setFocus(); //Remove focus from the child
   }
}

//============= focusTopChild ===============//

void QextMdiChildArea::focusTopChild()
{
   QextMdiChildFrm *lpC=m_pZ->last();
   if(!lpC) {
      emit lastChildFrmClosed();
      return;
   }
   //disable the labels of all the other children
   for(QextMdiChildFrm *pC=m_pZ->first();pC;pC=m_pZ->next()){
      if(pC != lpC)pC->m_pCaption->setActive(FALSE);
   }
   lpC->raise();
   if(!lpC->hasFocus()) {
//TEST      lpC->setFocus();
   }
   if (!lpC->m_pClient->hasFocus()) {
      lpC->m_pClient->activate();
//TEST      lpC->m_pClient->setFocus();
   }
}

//============= cascadeWindows ===============//

void QextMdiChildArea::cascadeWindows()
{
   int idx=0;
#if QT_VERSION < 300
   QList<QextMdiChildFrm> list(*m_pZ);
#else
   QPtrList<QextMdiChildFrm> list(*m_pZ);
#endif
   list.setAutoDelete(FALSE);
   while(!list.isEmpty()){
      QextMdiChildFrm *lpC=list.first();
      if(lpC->m_state != QextMdiChildFrm::Minimized){
         if(lpC->m_state==QextMdiChildFrm::Maximized)lpC->restorePressed();
         lpC->move(getCascadePoint(idx));
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
#if QT_VERSION < 300
   QList<QextMdiChildFrm> list(*m_pZ);
#else
   QPtrList<QextMdiChildFrm> list(*m_pZ);
#endif

   list.setAutoDelete(FALSE);
   while(!list.isEmpty()){
      QextMdiChildFrm *lpC=list.first();
      if(lpC->m_state != QextMdiChildFrm::Minimized){
         if(lpC->m_state==QextMdiChildFrm::Maximized)lpC->restorePressed();
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
#if QT_VERSION < 300
   QList<QextMdiChildFrm> list(*m_pZ);
#else
   QPtrList<QextMdiChildFrm> list(*m_pZ);
#endif
   list.setAutoDelete(FALSE);
   while(!list.isEmpty()){
      QextMdiChildFrm *lpC=list.first();
      if(lpC->m_state != QextMdiChildFrm::Minimized){
         if(lpC->m_state==QextMdiChildFrm::Maximized)lpC->restorePressed();
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
#if QT_VERSION < 300
   QList<QextMdiChildFrm> list(*m_pZ);
#else
   QPtrList<QextMdiChildFrm> list(*m_pZ);
#endif
   list.setAutoDelete(FALSE);
   while(!list.isEmpty()){
      QextMdiChildFrm *lpC=list.first();
      if(lpC->m_state != QextMdiChildFrm::Minimized){
         if(lpC->m_state==QextMdiChildFrm::Maximized)lpC->restorePressed();
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
      if ((lpC->m_state != QextMdiChildFrm::Minimized) &&
          (lpC->isVisible())) cnt++;
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

   QextMdiChildFrm *lpTop = topChild();
   int numVisible = getVisibleChildCount();
   if (numVisible<1) return;
   int numToHandle = ((numVisible > maxWnds) ? maxWnds : numVisible);
   int xQuantum = width()/colstable[numToHandle-1];
   if (xQuantum < ((lpTop->minimumSize().width() > m_defaultChildFrmSize.width()) ? lpTop->minimumSize().width() : m_defaultChildFrmSize.width())) {
      if (colrecall[numToHandle-1] != 0) {
         tileAllInternal(colrecall[numToHandle-1]);
         return;
      }
   }
   int yQuantum=height()/rowstable[numToHandle-1];
   if (yQuantum < ((lpTop->minimumSize().height() > m_defaultChildFrmSize.height()) ? lpTop->minimumSize().height() : m_defaultChildFrmSize.height())) {
      if (rowrecall[numToHandle-1] != 0) {
         tileAllInternal(rowrecall[numToHandle-1]);
         return;
      }
   }
   int curX=0;
   int curY=0;
   int curRow=1;
   int curCol=1;
   int curWin=1;
   for (QextMdiChildFrm *lpC=m_pZ->first();lpC;lpC=m_pZ->next()) {
      if (lpC->m_state!=QextMdiChildFrm::Minimized) {
         //restore the window
         if (lpC->m_state==QextMdiChildFrm::Maximized)
            lpC->restorePressed();
         if ((curWin%numToHandle)==0)
            lpC->setGeometry(curX,curY,xQuantum * lastwindw[numToHandle-1],yQuantum);
         else 
            lpC->setGeometry(curX,curY,xQuantum,yQuantum);
         //example : 12 windows : 3 cols 3 rows
         if (curCol<colstable[numToHandle-1]) { //curCol<3
            curX+=xQuantum; //add a column in the same row
            curCol++;       //increase current column
         } 
         else {
            curX = 0;         //new row
            curCol = 1;       //column 1
            if (curRow < rowstable[numToHandle-1]) { //curRow<3
               curY += yQuantum; //add a row
               curRow++;       //
            } 
            else {
               curY = 0;         //restart from beginning
               curRow = 1;       //
            }
         }
         curWin++;
      }
   }
   if (lpTop)
//TEST      lpTop->setFocus();
      lpTop->m_pClient->activate();
}
//============ tileAnodine ============//
void QextMdiChildArea::tileAnodine()
{
   QextMdiChildFrm *lpTop=topChild();
   int numVisible=getVisibleChildCount(); // count visible windows
   if(numVisible<1)return;
   int numCols=int(sqrt((double)numVisible)); // set columns to square root of visible count
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
         if(lpC->m_state==QextMdiChildFrm::Maximized)lpC->restorePressed();
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
   if (lpTop) {
//TEST      lpTop->setFocus();
      lpTop->m_pClient->activate();
   }
}

//============ tileVertically===========//
void QextMdiChildArea::tileVertically()
{
   QextMdiChildFrm *lpTop=topChild();
   int numVisible=getVisibleChildCount(); // count visible windows
   if(numVisible<1)return;
   
   int w = width() / numVisible;
   int lastWidth = 0;
   if( numVisible > 1)
      lastWidth = width() - (w * (numVisible - 1));
   else
      lastWidth = w;
   int h = height();
   int posX = 0;
   int countVisible = 0;
   
   for(QextMdiChildFrm *lpC=m_pZ->first();lpC;lpC=m_pZ->next()){
      if(lpC->m_state != QextMdiChildFrm::Minimized){
         if(lpC->m_state==QextMdiChildFrm::Maximized)lpC->restorePressed();
         countVisible++;
         if( countVisible < numVisible) {
            lpC->setGeometry( posX, 0, w, h);
            posX += w;
         }
         else { // last visible childframe
            lpC->setGeometry( posX, 0, lastWidth, h);
         }
      }
   }
   if (lpTop) {
//TEST      lpTop->setFocus();
      lpTop->m_pClient->activate();
   }
}

//============ layoutMinimizedChildren ============//
void QextMdiChildArea::layoutMinimizedChildren()
{
   int posX = 0;
   int posY = height();
   for(QextMdiChildFrm* child = m_pZ->first(); child ; child = m_pZ->next())
   {
      if( child->state() == QextMdiChildFrm::Minimized) {
         if( ( posX > 0) && ( posX + child->width() > width()) )
         {
            posX = 0;
            posY -= child->height();
         }
         child->move( posX, posY - child->height());
         posX = child->geometry().right();
      }
   }
}


void QextMdiChildArea::setMdiCaptionFont(const QFont &fnt)
{
   m_captionFont = fnt;
   QFontMetrics fm(m_captionFont);
   m_captionFontLineSpacing = fm.lineSpacing();

   for (QextMdiChildFrm *pC = m_pZ->first(); pC; pC = m_pZ->next()) {
      pC->doResize();
   }
}

void QextMdiChildArea::setMdiCaptionActiveForeColor(const QColor &clr)
{
   m_captionActiveForeColor = clr;
}

void QextMdiChildArea::setMdiCaptionActiveBackColor(const QColor &clr)
{
   m_captionActiveBackColor = clr;
}

void QextMdiChildArea::setMdiCaptionInactiveForeColor(const QColor &clr)
{
   m_captionInactiveForeColor = clr;
}

void QextMdiChildArea::setMdiCaptionInactiveBackColor(const QColor &clr)
{
   m_captionInactiveBackColor = clr;
}

#ifndef NO_INCLUDE_MOCFILES
#include "qextmdichildarea.moc"
#endif
