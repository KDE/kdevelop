/***************************************************************************
                          projectsession.cpp  -  description
                             -------------------
    begin                : 30 Nov 2002
    copyright            : (C) 2002 by Falk Brettschneider
    email                : falk@kdevelop.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "projectsession.h"

#include <qdom.h>
#include <qptrlist.h>
#include <qfile.h>

#include <kparts/part.h>
#include <kurl.h>
#include <kmessagebox.h>
#include <klocale.h>

#include "api.h"
#include "partcontroller.h"
#include "domutil.h"
#include "documentationpart.h"

//---------------------------------------------------------------------------
ProjectSession::ProjectSession()
{
  initXMLTree();
}

//---------------------------------------------------------------------------
ProjectSession::~ProjectSession()
{
}

//---------------------------------------------------------------------------
void ProjectSession::initXMLTree()
{
  // initializes the XML tree on startup of kdevelop and when a project
  // has been closed to ensure that the XML tree exists already including
  // doctype when a project gets opened that doesn't have a kdevses file
  // or a new project gets generated (which doesn't have a kdevses file
  // either as the project has never been closed before opening it).
  domdoc.clear();
  QDomDocument doc("KDevPrjSession");
  domdoc=doc;
  domdoc.appendChild( domdoc.createProcessingInstruction( "xml", "version=\"1.0\" encoding=\"UTF-8\"" ) );
  // KDevPrjSession is the root element of the XML file
  QDomElement session = domdoc.documentElement();
  session = domdoc.createElement("KDevPrjSession");
  domdoc.appendChild( session);
}

//---------------------------------------------------------------------------
bool ProjectSession::restoreFromFile(const QString& sessionFileName)
{
  bool bFileOpenOK = true;

  QFile f(sessionFileName);
  if ( f.open(IO_ReadOnly) ) {  // file opened successfully
    bool ok = domdoc.setContent( &f);
    f.close();
    if (!ok) {
      KMessageBox::sorry(0L,
                         i18n("The file %1 does not contain valid XML.\n"
                         "The loading of the session failed.").arg(sessionFileName));
      initXMLTree(); // because it was now broken after failed setContent()
      return false;
    }
  }
  else {
    bFileOpenOK = false;
  }

  // Check for proper document type.
  if (domdoc.doctype().name() != "KDevPrjSession") {
    KMessageBox::sorry(0L,
    i18n("The file %1 does not contain a valid KDevelop project session ('KDevPrjSession').\n").arg(sessionFileName)
    + i18n("The document type seems to be: '%1'.").arg(domdoc.doctype().name()));
    return false;
  }

  QDomElement session = domdoc.documentElement();

  // read the information about the mainframe widget
  if (bFileOpenOK) {
    recreateDocs(session);
  }
  return true;
}

//---------------------------------------------------------------------------
void ProjectSession::recreateDocs(QDomElement& el)
{
////  QDomElement mainframeEl = el.namedItem("Mainframe").toElement();
////  bool bMaxMode = (bool) mainframeEl.attribute("MaximizeMode", "0").toInt();
////  QextMdiMainFrm* pMainWidget = (QextMdiMainFrm*) qApp->mainWidget();
////  pMainWidget->setEnableMaximizedChildFrmMode(bMaxMode);
////  bool bTaskBarWasOn = pMainWidget->isViewTaskBarOn();
////  pMainWidget->hideViewTaskBar();

  // read the information about the documents
  QDomElement docsAndViewsEl = el.namedItem("DocsAndViews").toElement();
  int nNrOfDocs = docsAndViewsEl.attribute("NumberOfDocuments", "0").toInt();
  // loop over all docs
  int nDoc = 0;
  QDomElement docEl;
  for (docEl = docsAndViewsEl.firstChild().toElement(), nDoc = 0; nDoc < nNrOfDocs; nDoc++, docEl = docEl.nextSibling().toElement()) {
    // read the document name and type
    QString docName = docEl.attribute( "URL", "");
    if (!docName.isEmpty() /* && URL::exists(docName)*/) {
      KURL url(docName);
      // create the views of this document, the first view creation will also create the document
      recreateViews(url, docEl);
    }
  }

////  QextMdiChildView* pLastView = 0L;
////  QextMdiChildFrm*  pLastFrm = 0L;
////  QextMdiIterator<QextMdiChildView*>* winListIter = pMainWidget->createIterator();
////  for(winListIter->first(); !winListIter->isDone(); winListIter->next()){
////    pLastView = winListIter->currentItem();
////    if (bMaxMode && pLastView->isAttached()) {
////      pLastFrm = pLastView->mdiParent();
////    }
////  }
////  // evil hack (of Falk): resize the childframe again 'cause of event timing probs with resizing
////  if (bMaxMode && pLastFrm && pLastFrm->parentWidget()) {
////    QApplication::sendPostedEvents();
////    pLastFrm->setGeometry(-QEXTMDI_MDI_CHILDFRM_BORDER,
////                          -QEXTMDI_MDI_CHILDFRM_BORDER - pLastFrm->captionHeight() - QEXTMDI_MDI_CHILDFRM_SEPARATOR,
////                          pLastFrm->parentWidget()->width() + QEXTMDI_MDI_CHILDFRM_DOUBLE_BORDER,
////                          pLastFrm->parentWidget()->height() + QEXTMDI_MDI_CHILDFRM_SEPARATOR + QEXTMDI_MDI_CHILDFRM_DOUBLE_BORDER + pLastFrm->captionHeight());
////  }
////  // show the last (the one above all)
////  if (pLastView) {
////    pLastView->show();
////    QApplication::sendPostedEvents();
////    // show the rest
////    for(winListIter->first(); !winListIter->isDone(); winListIter->next()){
////      winListIter->currentItem()->show();
////    }
////  }
////  if (bTaskBarWasOn) {
////     pMainWidget->showViewTaskBar();
////  }
////  delete winListIter; winListIter = NULL;
}

//---------------------------------------------------------------------------
void ProjectSession::recreateViews(KURL& url, QDomElement docEl)
{
  // read information about the views
  int nNrOfViews = docEl.attribute( "NumberOfViews", "0").toInt();
  // loop over all views of this document
  int nView = 0;
  QDomElement viewEl;
  const bool HIDE = false;
  QString viewType;
  QString context;
  if (docEl.hasAttribute("context")) {
    context = docEl.attribute("context");
  }
  
  for (viewEl = docEl.firstChild().toElement(), nView = 0; nView < nNrOfViews; nView++, viewEl = viewEl.nextSibling().toElement()) {

////    // is it the focused view? (XXX well, this only refers to the module instance)
////    if (viewEl.attribute( "Focus", "0").toInt()) {
////      // yes, memorize for later use
////      pFocusedView = pView;
////    }
    // read geometry of current view
    loadViewGeometry(viewEl);

////    // read the cursor position of current view
////    int line = docEl.attribute( "CursorPosLine", "-1").toInt();
////    int col  = docEl.attribute( "CursorPosCol", "0").toInt();

    if (context.isEmpty()) {
      PartController::getInstance()->editDocument(url);
    }
    else {
      PartController::getInstance()->showDocument(url, context);
    }

  }
////  // restore focus
////  if (pFocusedView != 0L) {
////    if (pFocusedView->parentWidget()->inherits("QextMdiChildView")) {
////        ((QextMdiChildView*)pFocusedView->parentWidget())->activate();
////    }
////    pFocusedView->setFocus();
////  }

}

//---------------------------------------------------------------------------
void ProjectSession::loadViewGeometry(QDomElement viewEl)
{
  // read the view position and size
  int nMinMaxMode = viewEl.attribute( "MinMaxMode", "0").toInt();
  int   nLeft     = viewEl.attribute( "Left", "-10000").toInt(); // XXX hack: value -10000 wouldn't be restored correctly
  int   nTop      = viewEl.attribute( "Top", "-10000").toInt();
  int   nWidth    = viewEl.attribute( "Width", "-1").toInt();
  int   nHeight   = viewEl.attribute( "Height", "-1").toInt();

  // MDI stuff
  bool bAttached = (bool) viewEl.attribute( "Attach", "1").toInt();

////  // restore appearence
////  QextMdi::MdiMode mdiMode = ((QextMdiMainFrm*)m_pDocViewMan->parent())->mdiMode();
////  if ((mdiMode != QextMdi::TabPageMode) && (mdiMode != QextMdi::ToplevelMode)) {
////    if ((!pMDICover->isAttached()) && (bAttached) ) {
////      pMDICover->attach();
////    }
////    if ( (pMDICover->isAttached()) && (!bAttached) ) {
////      pMDICover->detach();
////    }
////  }
////  if (nMinMaxMode == 0) {
////    pMDICover->setInternalGeometry(QRect(nLeft, nTop, nWidth, nHeight));
////  }
////  else {
////    if (nMinMaxMode == 1) {
////      pMDICover->minimize();
////    }
////    if (nMinMaxMode == 2) {
////      // maximize: nothing to do, this is already under control of the mainframe
////    }
////    pMDICover->setRestoreGeometry(QRect(nLeft, nTop, nWidth, nHeight));
////  }
}

//---------------------------------------------------------------------------
bool ProjectSession::saveToFile(const QString& sessionFileName)
{

  QString section, keyword;
  QDomElement session = domdoc.documentElement();


  int nDocs = 0;
  QString docIdStr;

////  // read the information about the mainframe widget
////  QDomElement mainframeEl = session.namedItem("Mainframe").toElement();
////  if(mainframeEl.isNull()){
////    mainframeEl=domdoc.createElement("Mainframe");
////    session.appendChild( mainframeEl);
////  }
////  bool bMaxMode = ((QextMdiMainFrm*)m_pDocViewMan->parent())->isInMaximizedChildFrmMode();
////  mainframeEl.setAttribute("MaximizeMode", bMaxMode);


  // read the information about the documents
  QDomElement docsAndViewsEl = session.namedItem("DocsAndViews").toElement();
  if (docsAndViewsEl.isNull()) {
    docsAndViewsEl = domdoc.createElement("DocsAndViews");
    session.appendChild( docsAndViewsEl);
  }
  else {
    // we need to remove the old ones before memorizing the current ones (to avoid merging)
    QDomNode n = docsAndViewsEl.firstChild();
    while ( !n.isNull() ) {
      QDomNode toBeRemoved = n;
      n = n.nextSibling();
      docsAndViewsEl.removeChild(toBeRemoved);
    }
  }

  QPtrListIterator<KParts::Part> it( *PartController::getInstance()->parts() );
  for ( ; it.current(); ++it ) {
    KParts::ReadOnlyPart* pReadOnlyPart = dynamic_cast<KParts::ReadOnlyPart*>(it.current());
    if (!pReadOnlyPart)
      continue;

    DocumentationPart* pDocuPart = dynamic_cast<DocumentationPart*>(pReadOnlyPart);

    // TODO: Save relative path for project sharing?
    QString url = pReadOnlyPart->url().url();

    docIdStr.setNum(nDocs);
    QDomElement docEl = domdoc.createElement("Doc" + docIdStr);
    docEl.setAttribute( "URL", url);
    docsAndViewsEl.appendChild( docEl);
    nDocs++;
////    docEl.setAttribute( "Type", "???");
////    // get the view list
////    QList<KWriteView> viewList = pDoc->viewList();
////    // write the number of views
////    docEl.setAttribute( "NumberOfViews", viewList.count());
    docEl.setAttribute( "NumberOfViews", 1);
    // loop over all views of this document
    int nView = 0;
////    KWriteView* pView = 0L;
    QWidget* pView = 0L;
    QString viewIdStr;
////    for (viewList.first(), nView = 0; viewList.current() != 0; viewList.next(), nView++) {
////      pView = viewList.current();
////      if (pView != 0L) {
        viewIdStr.setNum( nView);
        QDomElement viewEl = domdoc.createElement( "View"+viewIdStr);
        docEl.appendChild( viewEl);
        // focus?
////        viewEl.setAttribute("Focus", (((CEditWidget*)pView->parentWidget()) == m_pDocViewMan->currentEditView()));
        viewEl.setAttribute("Type", "???");
        // save geometry of current view
        saveViewGeometry( pView, viewEl);
////      }
////    }
////    // save cursor position of current view
////    if (pView) {
////      QPoint cursorPos(pView->cursorPosition());
////      docEl.setAttribute("CursorPosCol", cursorPos.x());
////      docEl.setAttribute("CursorPosLine", cursorPos.y());
////    }
    if (pDocuPart) {
      docEl.setAttribute( "context", pDocuPart->context() );
    }
  }

  docsAndViewsEl.setAttribute("NumberOfDocuments", nDocs);

  // Write it out to the session file on disc
  QFile f(sessionFileName);
  if ( f.open(IO_WriteOnly) ) {    // file opened successfully
    QTextStream t( &f );        // use a text stream
    t << domdoc.toCString();
    f.close();
  }
  initXMLTree();  // clear and initialize the tree again

  return true;
}

//---------------------------------------------------------------------------
void ProjectSession::saveViewGeometry( QWidget* pView, QDomElement viewEl)
{
  if (!pView) return;
  if (!pView->parentWidget()) return;
////  QextMdiChildView* pMDICover = dynamic_cast<QextMdiChildView*>(pView->parentWidget());
/*!*/  QWidget* pMDICover = dynamic_cast<QWidget*>(pView->parentWidget());
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
////    geom = pMDICover->internalGeometry();
  }
  else {
////    geom = pMDICover->restoreGeometry();
  }
  viewEl.setAttribute( "MinMaxMode", nMinMaxMode);
  viewEl.setAttribute( "Left", geom.left());
  viewEl.setAttribute( "Top", geom.top());
  viewEl.setAttribute( "Width", geom.width());
  viewEl.setAttribute( "Height", geom.height());

////  // MDI stuff
////  QextMdi::MdiMode mdiMode = ((QextMdiMainFrm*)m_pDocViewMan->parent())->mdiMode();
////  viewEl.setAttribute( "Attach", pMDICover->isAttached() || (mdiMode == QextMdi::TabPageMode));
}
