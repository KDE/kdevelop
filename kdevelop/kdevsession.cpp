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

KDevSession::KDevSession(DocViewMan* pDocViewMan, const QString& /*fileName*/)
  : m_pDocViewMan(pDocViewMan)
{
}

//------------------------------------------------------------------------------
bool KDevSession::saveToFile(const QString& sessionFileName)
{
  QString section, keyword;

  // create a config file manager
  QDomDocument* pOutFile = new QDomDocument( "KDevelopSession");
  pOutFile->appendChild( pOutFile->createProcessingInstruction( "xml", "version=\"1.0\" encoding=\"UTF-8\"" ) );
  QDomElement session = pOutFile->createElement("KDevelopSession");
  pOutFile->appendChild( session);

  int nDocs = 0;
  QString docIdStr;

  if (m_pDocViewMan->docCount() > 0) {
    // loop over all docs using the doc ids
    QList<KWriteDoc> kWriteDocList = m_pDocViewMan->getKWriteDocList();
    for (kWriteDocList.first(); kWriteDocList.current() != 0; kWriteDocList.next()) {
      KWriteDoc* pDoc = kWriteDocList.current();
      if (!pDoc->fileName()) {
        // TODO: ask for filename
      }
      docIdStr.setNum(nDocs);
      QDomElement docEl = pOutFile->createElement("Doc" + docIdStr);
      docEl.setAttribute( "FileName", pDoc->fileName());
      session.appendChild( docEl);
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
          // save geometry of current view
          saveViewGeometry( pView, viewEl);
        }
      }
    }
  }

  session.setAttribute( "NumberOfDocuments", nDocs);

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
  // create a config file manager
  QDomDocument* pInFile = new QDomDocument( "KDevelopSession");

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
  if (pInFile->doctype().name() != "KDevelopSession") {
    KMessageBox::sorry(0L,
    i18n("The file %1 does not contain a valid KDevelop session ('KDevelopSession').\n").arg(sessionFileName)
    + i18n("The document type seems to be: '%1'.").arg(pInFile->doctype().name()));
    return false;
  }

  bool bFound = false;
  QDomElement session = pInFile->documentElement();
  // read the information about the documents

  int nNrOfDocs = session.attribute("NumberOfDocuments", "0").toInt();
  // loop over all docs
  int   nDoc;
  QDomElement docEl;
  for (docEl = session.firstChild().toElement(), nDoc = 0; nDoc < nNrOfDocs; nDoc++, docEl = docEl.nextSibling().toElement()) {
    // read the document name and type
    QString docName  = docEl.attribute( "FileName", "Unnamed");
    if (!docName.isEmpty()) {
      // create the document
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
        recreateKWriteViews( pDoc, docEl);
//???        pDoc->setModified(false);
        bFound = true;
        }
      }
    }
    if (bFound == false) {
    return false;
  }

  return true;
}

//------------------------------------------------------------------------------
void KDevSession::recreateKWriteViews( KWriteDoc* pDoc, QDomElement docEl)
{
  // read information about the views
  int nNrOfViews = docEl.attribute( "NumberOfViews", "0").toInt();
  // loop over all views of this document
  int   nView;
  QDomElement viewEl;
  QWidget* pFocusedView = 0L;
  for (viewEl = docEl.firstChild().toElement(), nView = 0; nView < nNrOfViews; nView++, viewEl = viewEl.nextSibling().toElement()) {
    // create the view
    CEditWidget* pView = m_pDocViewMan->createEditView( pDoc);
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
  QextMdiChildView* pMDICover = dynamic_cast<QextMdiChildView*>(pView->parentWidget()->parentWidget());
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
  QApplication::sendPostedEvents();
  if (nMinMaxMode == 0) {
    pMDICover->setInternalGeometry(QRect(nLeft, nTop, nWidth, nHeight));
  }
  else {
    if (nMinMaxMode == 1) {
      pMDICover->minimize();
    }
    if (nMinMaxMode == 2) {
      pMDICover->maximize();
    }
    pMDICover->setRestoreGeometry(QRect(nLeft, nTop, nWidth, nHeight));
  }
}

#include "kdevsession.moc"

