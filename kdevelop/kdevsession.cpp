/***************************************************************************
                          KDevSession.cpp  -  description
                             -------------------
    begin                : Mon Mar 26 2001
    copyright            : (C) 2001 by Falk Brettschneider
    email                : falk.brettschneider@gmx.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <qmessagebox.h>
#include <qdom.h>
#include <kconfig.h>
#include <kmessagebox.h>
#include <klocale.h>

#include "docviewman.h"
#include "kwdoc.h"
#include "kdevsession.h"
#include "ceditwidget.h"
#include "cdocbrowser.h"
#include "khtmlview.h"
#include "qextmdimainfrm.h"

KDevSession::KDevSession(DocViewMan* pDocViewMan, const QString& /*fileName*/)
  : m_pDocViewMan(pDocViewMan)
{
}

//------------------------------------------------------------------------------
bool KDevSession::saveToFile(const QString& sessionFileName)
{
  QString section, keyword;

  QDomDocument* pOutFile = new QDomDocument( "KDevPrjSession");
  pOutFile->appendChild( pOutFile->createProcessingInstruction( "xml", "version=\"1.0\" encoding=\"UTF-8\"" ) );
  QDomElement session = pOutFile->createElement("KDevPrjSession");
  pOutFile->appendChild( session);

  int nDocs = 0;
  QString docIdStr;

  // read the information about the mainframe widget
  QDomElement mainframeEl = pOutFile->createElement("Mainframe");
  session.appendChild( mainframeEl);
  bool bMaxMode = ((QextMdiMainFrm*)m_pDocViewMan->parent())->isInMaximizedChildFrmMode();
  mainframeEl.setAttribute("MaximizeMode", bMaxMode);


  // read the information about the documents
  QDomElement docsAndViewsEl = pOutFile->createElement("DocsAndViews");
  session.appendChild( docsAndViewsEl);
  if (m_pDocViewMan->docCount() > 0) {
    QList<KWriteDoc> kWriteDocList = m_pDocViewMan->getKWriteDocList();
    for (kWriteDocList.first(); kWriteDocList.current() != 0; kWriteDocList.next()) {
      KWriteDoc* pDoc = kWriteDocList.current();
      if (!pDoc->fileName()) {
        // TODO: ask for filename
      }
      docIdStr.setNum(nDocs);
      QDomElement docEl = pOutFile->createElement("Doc" + docIdStr);
      docEl.setAttribute( "FileName", pDoc->fileName());
      docsAndViewsEl.appendChild( docEl);
      // save the document itself
//???         if (pDoc->bIsModified())
      nDocs++;
//???            pDoc->saveFile(docFileName, 0L, "Synchronization");
      docEl.setAttribute( "Type", "KWriteDoc");
      // get the view list
      QList<KWriteView> viewList = pDoc->viewList();
      // write the number of views
      docEl.setAttribute( "NumberOfViews", viewList.count());
      // loop over all views of this document
      int nView;
      KWriteView* pView;
      QString viewIdStr;
      for (viewList.first(), nView = 0; viewList.current() != 0; viewList.next(), nView++) {
        pView = viewList.current();
        if (pView != 0L) {
          viewIdStr.setNum( nView);
          QDomElement viewEl = pOutFile->createElement( "View"+viewIdStr);
          docEl.appendChild( viewEl);
          // focus?
          viewEl.setAttribute("Focus", (((CEditWidget*)pView->parentWidget()) == m_pDocViewMan->currentEditView()));
          viewEl.setAttribute("Type", "KWriteView");
          // save geometry of current view
          saveViewGeometry( pView->parentWidget(), viewEl);
        }
      }
    }
    QList<CDocBrowser> kDocBrowserList = m_pDocViewMan->getDocBrowserList();
    for (kDocBrowserList.first(); kDocBrowserList.current() != 0; kDocBrowserList.next()) {
      CDocBrowser* pDoc = kDocBrowserList.current();
      if (!pDoc->currentURL()) {
        // TODO: ask for filename
      }
      docIdStr.setNum(nDocs);
      QDomElement docEl = pOutFile->createElement("Doc" + docIdStr);
      docEl.setAttribute( "FileName", pDoc->currentURL());
      docsAndViewsEl.appendChild( docEl);
      // save the document itself
//???         if (pDoc->bIsModified())
      nDocs++;
//???            pDoc->saveFile(docFileName, 0L, "Synchronization");
      docEl.setAttribute( "Type", "CDocBrowser");
      // get the view
      KHTMLView* pView = pDoc->view();
      // write the number of views
      docEl.setAttribute( "NumberOfViews", 1);  // there's only one view possible for a CDocBrowser
      QDomElement viewEl = pOutFile->createElement( "View");
      docEl.appendChild( viewEl);
      // focus?
      viewEl.setAttribute("Focus", (((KHTMLView*)pView->parentWidget()) == m_pDocViewMan->currentBrowserView()));
      viewEl.setAttribute("Type", "KHTMLView");
      // save geometry of current view
      saveViewGeometry( pView, viewEl);
    }
  }

  docsAndViewsEl.setAttribute( "NumberOfDocuments", nDocs);

  // Write it out to a tmp file
  QFile f(sessionFileName);
  if ( f.open(IO_WriteOnly) ) {    // file opened successfully
    QTextStream t( &f );        // use a text stream
    t << pOutFile->toString();
    f.close();
  }

  return true;
}

//------------------------------------------------------------------------------
bool KDevSession::restoreFromFile(const QString& sessionFileName)
{
  QDomDocument* pInFile = new QDomDocument( "KDevPrjSession");

  // Write it out to a tmp file
  QFile f(sessionFileName);
  if ( f.open(IO_ReadOnly) ) {  // file opened successfully
    bool ok = pInFile->setContent( &f);
    f.close();
    if (!ok) {
      KMessageBox::sorry(0L,
                         i18n("The file %1 does not contain valid XML.\n"
                         "The loading of the session failed.").arg(sessionFileName));
      return false;
    }	  	
  }

  // Check for proper document type.
  if (pInFile->doctype().name() != "KDevPrjSession") {
    KMessageBox::sorry(0L,
    i18n("The file %1 does not contain a valid KDevelop project session ('KDevPrjSession').\n").arg(sessionFileName)
    + i18n("The document type seems to be: '%1'.").arg(pInFile->doctype().name()));
    return false;
  }

  QDomElement session = pInFile->documentElement();

  // read the information about the mainframe widget
  QDomElement mainframeEl = session.namedItem("Mainframe").toElement();
  bool bMaxMode = (bool) mainframeEl.attribute("MaximizeMode", "0").toInt();
  ((QextMdiMainFrm*)m_pDocViewMan->parent())->setEnableMaximizedChildFrmMode(bMaxMode);

  // read the information about the documents
  QDomElement docsAndViewsEl = session.namedItem("DocsAndViews").toElement();
  int nNrOfDocs = docsAndViewsEl.attribute("NumberOfDocuments", "0").toInt();
  // loop over all docs
  int   nDoc;
  QDomElement docEl;
  for (docEl = docsAndViewsEl.firstChild().toElement(), nDoc = 0; nDoc < nNrOfDocs; nDoc++, docEl = docEl.nextSibling().toElement()) {
    // read the document name and type
    QString docName = docEl.attribute( "FileName", "");
    QString docType = docEl.attribute( "Type", "Unknown");
    if (!docName.isEmpty()) {
      // create the document
      if (docType == QString("KWriteDoc")) {
        KWriteDoc* pDoc = m_pDocViewMan->createKWriteDoc(docName);
        if (pDoc) {
          // load contents from file
          if(QFile::exists(docName)) {
            QFile f(docName);
            if (f.open(IO_ReadOnly)) {
              pDoc->loadFile(f);
              f.close();
            }
          }
          // views
          recreateViews( pDoc, docEl);
          if (pDoc->viewCount() == 0)
            m_pDocViewMan->closeKWriteDoc(pDoc);
        }
      }
      else if (docType == QString("CDocBrowser")) {
        CDocBrowser* pDoc = m_pDocViewMan->createCDocBrowser(docName);
        if (pDoc) {
          // views
          recreateViews( pDoc, docEl);
        }
      }
    }
  }

  QextMdiIterator<QextMdiChildView*>* winListIter = ((QextMdiMainFrm*)m_pDocViewMan->parent())->createIterator();
  for(winListIter->first(); !winListIter->isDone(); winListIter->next()){
    winListIter->currentItem()->show();
  }

  return true;
}

//------------------------------------------------------------------------------
void KDevSession::recreateViews( QObject* pDoc, QDomElement docEl)
{
  // read information about the views
  int nNrOfViews = docEl.attribute( "NumberOfViews", "0").toInt();
  // loop over all views of this document
  int   nView;
  QDomElement viewEl;
  QWidget* pFocusedView = 0L;
  const bool HIDE = false;
  for (viewEl = docEl.firstChild().toElement(), nView = 0; nView < nNrOfViews; nView++, viewEl = viewEl.nextSibling().toElement()) {
    // create the view
    QWidget* pView = 0L;
    QString viewType = viewEl.attribute( "Type", "Unknown");
    if (viewType == QString("KWriteView"))
      pView = m_pDocViewMan->createEditView( (KWriteDoc*) pDoc, HIDE);
    else if (viewType == QString("KHTMLView"))
      pView = m_pDocViewMan->createBrowserView( (CDocBrowser*) pDoc, HIDE);
    if (pView != 0L) {
      // is it the focused view? (XXX well, this only refers to the module instance)
      if (viewEl.attribute( "Focus", "0").toInt()) {
        // yes, memorize for later use
        pFocusedView = pView;
      }
      // read geometry of current view
      loadViewGeometry( pView, viewEl);
    }
  }

  // restore focus
  if (pFocusedView != 0L) {
    pFocusedView->setFocus();
  }

}

void KDevSession::saveViewGeometry( QWidget* pView, QDomElement viewEl)
{
  if (!pView) return;
  if (!pView->parentWidget()) return;
  QextMdiChildView* pMDICover = dynamic_cast<QextMdiChildView*>(pView->parentWidget());
  if (!pMDICover) return;

  // write the view position and size
  QRect geom;
  int nMinMaxMode = 0;
  if (pMDICover->isMinimized()) {
    nMinMaxMode = 1;
  }
  if (pMDICover->isMaximized()) {
    nMinMaxMode = 2;
  }
  if (nMinMaxMode == 0) {
    geom = pMDICover->internalGeometry();
  }
  else {
    geom = pMDICover->restoreGeometry();
  }
  viewEl.setAttribute( "MinMaxMode", nMinMaxMode);
  viewEl.setAttribute( "Left", geom.left());
  viewEl.setAttribute( "Top", geom.top());
  viewEl.setAttribute( "Width", geom.width());
  viewEl.setAttribute( "Height", geom.height());

  // MDI stuff
  viewEl.setAttribute( "Attach", pMDICover->isAttached());
}

void KDevSession::loadViewGeometry( QWidget* pView, QDomElement viewEl)
{
  if (!pView) return;
  if (!pView->parentWidget()) return;
  QextMdiChildView* pMDICover = dynamic_cast<QextMdiChildView*>(pView->parentWidget());
  if (!pMDICover) return;

  // read the view position and size
  int nMinMaxMode = viewEl.attribute( "MinMaxMode", "0").toInt();
  int   nLeft     = viewEl.attribute( "Left", "-10000").toInt(); // XXX hack: value -10000 wouldn't be restored correctly
  int   nTop      = viewEl.attribute( "Top", "-10000").toInt();
  int   nWidth    = viewEl.attribute( "Width", "-1").toInt();
  int   nHeight   = viewEl.attribute( "Height", "-1").toInt();

  // MDI stuff
  bool bAttached = (bool) viewEl.attribute( "Attach", "1").toInt();

  // restore appearence
  if ( (!pMDICover->isAttached()) && (bAttached) ) {
    pMDICover->attach();
  }
  if ( (pMDICover->isAttached()) && (!bAttached) ) {
    pMDICover->detach();
  }
//  QApplication::sendPostedEvents();
  if (nMinMaxMode == 0) {
    pMDICover->setInternalGeometry(QRect(nLeft, nTop, nWidth, nHeight));
  }
  else {
    if (nMinMaxMode == 1) {
      pMDICover->minimize();
    }
    if (nMinMaxMode == 2) {
//      pMDICover->maximize();
//      QApplication::sendPostedEvents();
    }
    pMDICover->setRestoreGeometry(QRect(nLeft, nTop, nWidth, nHeight));
  }
}

//#include "kdevsession.moc"
