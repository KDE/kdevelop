/***************************************************************************
  docviewman.cpp - MDI manager for
                   document classes of KDevelop (KWriteDocs, CDocBrowser)
                   and view classes of KDevelop (CEditWidget, KHTMLView)
                             -------------------

    begin                : 03 Mar 2001
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

#include <qfile.h>
#include <qfileinfo.h>
#include <qlayout.h>
#include <qobjectlist.h>

#include <kmessagebox.h>
#include <klocale.h>

#include "ckdevelop.h"
#include "kwdoc.h"
#include "cdocbrowser.h"
#include "khtmlview.h"
#include "ceditwidget.h"
#include "docviewman.h"
#include "./dbg/brkptmanager.h"
#include "./dbg/vartree.h"

//==============================================================================
// class implementation
//------------------------------------------------------------------------------
DocViewMan::DocViewMan( CKDevelop* parent)
: QObject( parent)
  ,m_pParent(parent)
  ,m_pCurEditDoc(0L)
  ,m_pCurEditView(0L)
  ,m_pCurBrowserDoc(0L)
  ,m_pCurBrowserView(0L)
  ,m_currentDocType(DocViewMan::Undefined)
{
  m_docsAndViews.setAutoDelete(true);
  m_MDICoverList.setAutoDelete(true);

  connect( this, SIGNAL(sig_viewGotFocus(QWidget*,int)), m_pParent, SLOT(slotViewSelected(QWidget*,int)) );
}

//------------------------------------------------------------------------------
DocViewMan::~DocViewMan()
{
}

//------------------------------------------------------------------------------
// create a new empty document
// optionally fill the data by loading a file,
// return the document id (-1 if failed)
//------------------------------------------------------------------------------
int DocViewMan::createDoc( int contentsType, const QString& strFileName)
{
  QObject*   pDoc = 0L;

  switch (contentsType) {
  case DocViewMan::Header:
  case DocViewMan::Source:
    pDoc = new KWriteDoc( &m_highlightManager, strFileName);
    debug("creating KWriteDoc ");
    if (pDoc == 0L) {
      return -1;  // creation failed
    }
    break;
  case DocViewMan::HTML:
    pDoc = new CDocBrowser(0L, "browser");
    debug("creating CDocBrowser ");
    break;
  }

//?       // connect signals
//?       connect(pDoc, SIGNAL(sig_updated(QObject*, int)),
//?               this, SIGNAL(sig_updated(QObject*, int)));
//?
  // create new list item
  DocViewNode*   pNewInfo = new DocViewNode; // to be deleted by list
  static int idCounter = 0;
  if (pNewInfo != 0) {
    pNewInfo->pDoc = pDoc;
    pNewInfo->docId = idCounter++; //pDoc->id();
    pNewInfo->docType = contentsType;
    m_docsAndViews.append(pNewInfo);
  }

  debug(" with id : %d !\n", pNewInfo->docId);

  // return id of new document
  return pNewInfo->docId; //pDoc->id();
}

//------------------------------------------------------------------------------
// load document from file
//------------------------------------------------------------------------------
void DocViewMan::loadDoc(int docId, const QString& strFileName, int /*mode*/)
{
  // find document info
  QListIterator<DocViewNode> itDoc(m_docsAndViews);
  for (; (itDoc.current() != 0) && (itDoc.current()->docId != docId); ++itDoc) {}
  DocViewNode* pDocViewNode = itDoc.current();
  if (!pDocViewNode)
    return; // failed

  // read
  switch (pDocViewNode->docType) {
  case DocViewMan::Header:
  case DocViewMan::Source:
    if(QFile::exists(strFileName)) {
      QFile f(strFileName);
      if (f.open(IO_ReadOnly)) {
        KWriteDoc* pDoc = (KWriteDoc*) pDocViewNode->pDoc;
        pDoc->loadFile(f);
        f.close();
      }
    }
    break;
  case DocViewMan::HTML:
    // TODO
    break;
  }
}

//------------------------------------------------------------------------------
// save document to file
//------------------------------------------------------------------------------
bool DocViewMan::saveDoc(int docId, const QString& strFileName)
{
  // find document info
  QListIterator<DocViewNode> itDoc(m_docsAndViews);
  for (; (itDoc.current() != 0) && (itDoc.current()->docId != docId); ++itDoc) {}
  DocViewNode* pDocViewNode = itDoc.current();

  if (!pDocViewNode || strFileName.isEmpty())
    return false; // failed

  // write
  switch (pDocViewNode->docType) {
  case DocViewMan::Header:
  case DocViewMan::Source:
    {
      QFileInfo info(strFileName);
      if(info.exists() && !info.isWritable()) {
        KMessageBox::sorry(0L, i18n("You do not have write permission to this file"));
        return false;
      }

      QFile f(strFileName);
      if (f.open(IO_WriteOnly | IO_Truncate)) {
        KWriteDoc* pDoc = (KWriteDoc*) pDocViewNode->pDoc;
        pDoc->writeFile(f);
        pDoc->updateViews();
        f.close();
        return true;//kWriteDoc->setFileName(name);
      }
      KMessageBox::sorry(0L,  i18n("An Error occured while trying to open this Document"));
      return false;
    }
    break;
  default:
    // nothing to do
    break;
  }
  return true;
}

//------------------------------------------------------------------------------
// close a document, causes all views to be closed
//------------------------------------------------------------------------------
void DocViewMan::closeDoc(int docId)
{
  // find document info
  QListIterator<DocViewNode> itDoc(m_docsAndViews);
  for (; (itDoc.current() != 0) && (itDoc.current()->docId != docId); ++itDoc) {}
  DocViewNode*   pCurDocViewNode = itDoc.current();
  if (!pCurDocViewNode)
    return; //failed

  switch (pCurDocViewNode->docType) {
  case DocViewMan::Header:
  case DocViewMan::Source:
    {
      // first, disconnect all views, then delete them
      // --> this avoids that deleting one view focuses the next dying view which would be senseless
      QListIterator<QWidget>  itViews(pCurDocViewNode->existingViews);
      for (; itViews.current() != 0; ++itViews) {
        CEditWidget* pView = (CEditWidget*) itViews.current();
        // disconnect the focus signals
        disconnect(pView, SIGNAL(gotFocus(CEditWidget*)),
                   this, SLOT(slot_gotFocus(CEditWidget*)));
        // remove the view from MDI and delete the view
        QextMdiChildView* pMDICover = (QextMdiChildView*) pView->parentWidget();
        m_pParent->removeWindowFromMdi( pMDICover);
        delete pMDICover;
      }
      pCurDocViewNode->existingViews.clear();
      // emit an according signal if we closed the last view
      if (countViews() == 0) {
        emit sig_lastViewClosed();
      }
      // now finally, delete the document
      KWriteDoc* pDoc = (KWriteDoc*) pCurDocViewNode->pDoc;
  //?      //   disconnect document signals
  //?       disconnect(pDoc, SIGNAL(sig_updated(QObject*, int)),
  //?                  this, SIGNAL(sig_updated(QObject*, int)));
      delete pDoc;
    }
    break;
  case DocViewMan::HTML:
    {
      QListIterator<QWidget>  itViews(pCurDocViewNode->existingViews);
      for (; itViews.current() != 0; ++itViews) {
        // remove the view from MDI
        QWidget* pView = itViews.current();
        QextMdiChildView* pMDICover = (QextMdiChildView*) pView->parentWidget();
        pMDICover->hide();
        pView->reparent(0L, 0, QPoint(0,0));
        QApplication::sendPostedEvents();
        m_pParent->removeWindowFromMdi( pMDICover);
        delete pMDICover;
      }
      pCurDocViewNode->existingViews.clear();
      // emit an according signal if we closed the last view
      if (countViews() == 0) {
        emit sig_lastViewClosed();
      }
      // now finally, delete the document (which inclusively deletes the view)
      CDocBrowser* pDoc = (CDocBrowser*) pCurDocViewNode->pDoc;
      delete pDoc;
    }
    break;
  }

  //   remove list entry
  int removedDocType = pCurDocViewNode->docType;
  m_docsAndViews.remove(pCurDocViewNode);

  // check if there's still a m_pCurBrowserDoc, m_pCurBrowserView, m_pCurEditDoc, m_pCurEditView
  bool bBrowserDocFound = false;
  bool bEditDocFound = false;
  for (itDoc.toFirst(); itDoc.current() != 0; ++itDoc) {
    int docType = itDoc.current()->docType;
    if (docType == DocViewMan::HTML)
      bBrowserDocFound = true;
    else if ((docType == DocViewMan::Source) || (docType == DocViewMan::Header))
      bEditDocFound = true;
  }
  switch (removedDocType) {
  case DocViewMan::Header:
  case DocViewMan::Source:
    if (!bEditDocFound) {
      m_pCurEditDoc = 0L;
      m_pCurEditView = 0L;
    }
    break;
  case DocViewMan::HTML:
    if (!bBrowserDocFound) {
      m_pCurBrowserDoc = 0L;
      m_pCurBrowserView = 0L;
    }
    break;
  }

  //   emit an according signal if we closed the last doc
  if (m_docsAndViews.count() == 0) {
    m_currentDocType = DocViewMan::Undefined;
    emit sig_lastDocClosed();
  }
}

//------------------------------------------------------------------------------
// retrieve the document pointer
//------------------------------------------------------------------------------
QObject* DocViewMan::docPointer(int docId) const
{
  // find document info
  QListIterator<DocViewNode>  itDoc(m_docsAndViews);
  for (; (itDoc.current() != 0) && (itDoc.current()->docId != docId); ++itDoc) {}

  if (itDoc.current() != 0) {
    return itDoc.current()->pDoc;
  }
  else {
    return 0;
  }
}

//------------------------------------------------------------------------------
// get the ids of all documents of this type or a combination of types
//------------------------------------------------------------------------------
QList<int> DocViewMan::docs( int type) const
{
  QListIterator<DocViewNode>    itDoc(m_docsAndViews);
  QList<int> listDocIds;
  if (type == DocViewMan::Undefined)
    for (; itDoc.current() != 0; ++itDoc) {
      listDocIds.append(&(itDoc.current()->docId));
    }
  else
    for (; itDoc.current() != 0; ++itDoc) {
      if (itDoc.current()->docType & type)
        listDocIds.append(&(itDoc.current()->docId));
    }
  return listDocIds;
}

//------------------------------------------------------------------------------
// get the id of a document displayed by a given view
//------------------------------------------------------------------------------
int DocViewMan::docOfView(QWidget* pView) const
{
  QListIterator<DocViewNode>    itDoc(m_docsAndViews);
  for (; itDoc.current() != 0; ++itDoc) {
    QListIterator<QWidget>  itView(itDoc.current()->existingViews);
    for (; itView.current() != 0; ++itView) {
      if (itView.current() == pView) {
        // view found
        return itDoc.current()->docId;
      }
    }
  }
  // view not found
  return -1;
}

//------------------------------------------------------------------------------
// create a new view for a document
//------------------------------------------------------------------------------
QWidget* DocViewMan::createView(int docId)
{
  // find document info
  QListIterator<DocViewNode> itDoc(m_docsAndViews);
  for (; (itDoc.current() != 0) && (itDoc.current()->docId != docId); ++itDoc) {}
  DocViewNode* pDocViewNode = itDoc.current();
  if (!pDocViewNode)
    return 0L; // failed, no such doc found

  // get the type of document
  int doctype = pDocViewNode->docType;

  // cause a view to be created
  QWidget*   pNewView = 0;

  switch (doctype) {
  case DocViewMan::Header:
  case DocViewMan::Source:
    {
      KWriteDoc* pDoc = (KWriteDoc*) pDocViewNode->pDoc;
      // create the view and add to MDI
      CEditWidget* pEW = new CEditWidget(0L, "autocreatedview", pDoc, pDocViewNode->docType);
      pNewView = pEW;
      pNewView->setCaption( pDoc->fileName());

      //connect the editor lookup function with slotHelpSText
      connect( pEW, SIGNAL(lookUp(QString)),m_pParent, SLOT(slotHelpSearchText(QString)));
      connect( pEW, SIGNAL(newCurPos()), m_pParent, SLOT(slotNewLineColumn()));
      connect( pEW, SIGNAL(newStatus()),m_pParent, SLOT(slotNewStatus()));
      connect( pEW, SIGNAL(clipboardStatus(KWriteView *, bool)), m_pParent, SLOT(slotClipboardChanged(KWriteView *, bool)));
      connect( pEW, SIGNAL(newUndo()),m_pParent, SLOT(slotNewUndo()));
      connect( pEW, SIGNAL(bufferMenu(const QPoint&)),m_pParent, SLOT(slotBufferMenu(const QPoint&)));
      connect( pEW, SIGNAL(grepText(QString)), m_pParent, SLOT(slotEditSearchInFiles(QString)));
      connect( pEW->popup(), SIGNAL(highlighted(int)), m_pParent, SLOT(statusCallback(int)));
    	// Connect the breakpoint manager to monitor the bp setting - even when the debugging isn't running
      connect( pEW, SIGNAL(editBreakpoint(const QString&,int)), m_pParent->getBrkptManager(), SLOT(slotEditBreakpoint(const QString&,int)));
      connect( pEW, SIGNAL(toggleBPEnabled(const QString&,int)), m_pParent->getBrkptManager(), SLOT(slotToggleBPEnabled(const QString&,int)));
      connect( pEW, SIGNAL(toggleBreakpoint(const QString&,int)), m_pParent->getBrkptManager(), SLOT(slotToggleStdBreakpoint(const QString&,int)));
      connect( pEW, SIGNAL(clearAllBreakpoints()), m_pParent->getBrkptManager(),   SLOT(slotClearAllBreakpoints()));
      // connect adding watch variable from the rmb in the editors
      connect( pEW, SIGNAL(addWatchVariable(const QString&)), m_pParent->getVarViewer()->varTree(), SLOT(slotAddWatchVariable(const QString&)));
      if (doctype == DocViewMan::Source) {
        connect( pEW, SIGNAL(markStatus(KWriteView *, bool)), m_pParent, SLOT(slotCPPMarkStatus(KWriteView *, bool)));
      }
      else {
        connect( pEW, SIGNAL(markStatus(KWriteView *, bool)), m_pParent, SLOT(slotHEADERMarkStatus(KWriteView *, bool)));
      }
    }
    break;
  case DocViewMan::HTML:
    {
      CDocBrowser* pDoc = (CDocBrowser*) pDocViewNode->pDoc;
      pNewView = pDoc->view();
      pNewView->setCaption( pDoc->currentURL());
    }
    break;
  }

  if (!pNewView)
    return 0L;  // failed, could not create view

  // connect signals
//?       connect(pNewView, SIGNAL(sig_updated(QObject*, int)),
//?               this, SIGNAL(sig_updated(QObject*, int)));

  // connect document with view
  if (docId >= 0) {
    // add view to view list of doc
    pDocViewNode->existingViews.append(pNewView);
  }

  // cover it by a QextMDI childview and add that MDI system
  QextMdiChildView* pMDICover = new QextMdiChildView( pNewView->caption());
  m_MDICoverList.append( pMDICover);
  QBoxLayout* pLayout = new QHBoxLayout( pMDICover, 0, -1, "layout");
  pNewView->reparent( pMDICover, QPoint(0,0));
  QApplication::sendPostedEvents();
  pLayout->addWidget( pNewView);
  pMDICover->setName( pNewView->name());
  // captions
  QString shortName = pNewView->caption();
  int length = shortName.length();
  shortName = shortName.right(length - (shortName.findRev('/') +1));
  pMDICover->setTabCaption( shortName);
  connect(pMDICover, SIGNAL(gotFocus(QextMdiChildView*)),
          this, SLOT(slot_gotFocus(QextMdiChildView*)));

  // fake a gotFocus to update the currentEditView/currentBrowserView pointers _before_ adding to MDI control
  slot_gotFocus( pMDICover);

  // take it under MDI mainframe control (note: this triggers also a setFocus())
  m_pParent->addWindow( pMDICover, QextMdi::StandardAdd);
  // correct the default settings of QextMDI ('cause we haven't a tab order for subwidget focuses)
  pMDICover->setFirstFocusableChildWidget(0L);
  pMDICover->setLastFocusableChildWidget(0L);
  // show
  pMDICover->show();

  return pNewView;
}

//------------------------------------------------------------------------------
// close a view
//------------------------------------------------------------------------------
void DocViewMan::closeView(QWidget* pView)
{
  // find document info
  bool bFound = false;
  QList<QWidget>*   pViewList;
  for (m_docsAndViews.first(); (m_docsAndViews.current() != 0) && (!bFound);) {
    pViewList = &(m_docsAndViews.current()->existingViews);
    for (pViewList->first(); (pViewList->current() != 0) && (!bFound);) {
      if (pViewList->current() == pView) {
        bFound = true;
      }
      else {
        pViewList->next();
      }
    }
    if (!bFound) {
      m_docsAndViews.next();
    }
  }

  if (!bFound)
    return;

  // the view was found
  // store the current items since lists may change while deleting view and doc
  DocViewNode*   pDocViews = m_docsAndViews.current();
  QextMdiChildView* pMDICover = (QextMdiChildView*) pView->parentWidget();
  pMDICover->hide();

  // disconnect the focus signals
  disconnect(pMDICover, SIGNAL(gotFocus(QextMdiChildView*)), this, SLOT(slot_gotFocus(QextMdiChildView*)));
  // remove view list entry
  pViewList->remove(pView);
  int docType = pDocViews->docType;
  if (docType == DocViewMan::HTML) {
    // get a KHTMLView out of the parent to avoid a delete, it will be deleted later in the CDocBrowser destructor
    pView->reparent(0L,0,QPoint(0,0));
    QApplication::sendPostedEvents();
  }
  // remove the view from MDI and delete the view
  m_pParent->removeWindowFromMdi( pMDICover);
  delete pMDICover;

  // emit an according signal if we closed the last view
  if (countViews() == 0) {
    emit sig_lastViewClosed();
  }
  // check whether there are remaining views
  if (pDocViews->existingViews.count() == 0) {
    switch (docType) {
    case DocViewMan::Header:
    case DocViewMan::Source:
      {
        KWriteDoc* pDoc = (KWriteDoc*) pDocViews->pDoc;
        m_pParent->removeFileFromEditlist( pDoc->fileName()); // this removes from edit_infos and calls closeDoc right after
      }
      break;
    case DocViewMan::HTML:
      closeDoc( pDocViews->docId);
      break;
    }
  }
}

//------------------------------------------------------------------------------
// get number of views handled by this doc view manager
//------------------------------------------------------------------------------
int DocViewMan::countViews() const
{
  int nViews = 0;
  // scan the document info list
  QListIterator<DocViewNode> it(m_docsAndViews);
  for ( ; it.current() != 0L; ++it) {
    nViews += it.current()->existingViews.count();
  }
  return nViews;
}

//------------------------------------------------------------------------------
// get number of views for a document
//------------------------------------------------------------------------------
int DocViewMan::countViewsOfDoc(int docId) const
{
  // find document info
  QListIterator<DocViewNode> it(m_docsAndViews);
  for ( ; it.current() && (it.current()->docId != docId); ++it) {}
  if( it.current() != 0)
    return it.current()->existingViews.count();
  else
    return 0;
}

//------------------------------------------------------------------------------
// get all view pointer for a document
//------------------------------------------------------------------------------
const QList<QWidget> DocViewMan::viewsOfDoc(int docId) const
{
  // find document info
  QListIterator<DocViewNode> it(m_docsAndViews);
  for ( ; it.current() && (it.current()->docId != docId); ++it) {}
  if( it.current() != 0)
    return it.current()->existingViews;
  else
    // return an empty list
    return QList<QWidget>();
}

//------------------------------------------------------------------------------
// get the type of a document
//------------------------------------------------------------------------------
int DocViewMan::docType(int docId) const
{
  // find document info
  QListIterator<DocViewNode> it(m_docsAndViews);
  for ( ; it.current() && (it.current()->docId != docId); ++it) {}
  if( it.current() != 0)
    return it.current()->docType;
  else
    // not found
    return -1;
}

//------------------------------------------------------------------------------
// Connected to the focus in event occures signal of CEditWidget.
// Moves the focused view to the end of the list of focused views or
// adds it. Emits the sig_viewGotFocus signal
//------------------------------------------------------------------------------
void DocViewMan::slot_gotFocus(QextMdiChildView* pMDICover)
{
  // set current view, distinguish between edit widget and browser widget
  QObjectList* pL = (QObjectList*) pMDICover->children();
  QWidget* pView = 0L;
  QObject* pChild;
  for ( pChild = pL->first(); pChild && !pView; pChild = pL->next()) {
    if (pChild->inherits("QWidget")) {
      pView = (QWidget*) pChild;
    }
  }

  if (pView->inherits("CEditWidget"))
    m_pCurEditView = (CEditWidget*) pView;
  else
    m_pCurBrowserView = (KHTMLView*) pView;

  // set current document
  m_currentDocType = DocViewMan::Undefined; // assume worst case
  QObject* pDoc = 0L;
  QListIterator<DocViewNode>    itDoc(m_docsAndViews);
  for (; itDoc.current() != 0 && !pDoc; ++itDoc) {
    QListIterator<QWidget>  itView(itDoc.current()->existingViews);
    for (; itView.current() != 0; ++itView) {
      if (itView.current() == pView) {
        // view found
        pDoc = itDoc.current()->pDoc;
        m_currentDocType = itDoc.current()->docType;
      }
    }
  }
  if (pView->inherits("CEditWidget"))
    m_pCurEditDoc = (KWriteDoc*) pDoc;
  else
    m_pCurBrowserDoc = (CDocBrowser*) pDoc;

  // emit the got focus signal (connected to CKDevelop but could also be caught by other ones)
  emit sig_viewGotFocus( pView, m_currentDocType);
}

//------------------------------------------------------------------------------
// returns the number of documents for the asked document type
//------------------------------------------------------------------------------
int DocViewMan::docCount() const
{
  return m_docsAndViews.count();
}

//------------------------------------------------------------------------------
// retrieves the document found by its filename
//------------------------------------------------------------------------------
int DocViewMan::findDoc( const QString& strFileName) const
{
  QListIterator<DocViewNode> it(m_docsAndViews);
  bool bFound = false;
  DocViewNode* pDocViewNode = 0L;
  for ( ; it.current() && !bFound; ++it) {
    pDocViewNode = it.current();
    switch (pDocViewNode->docType) {
      case DocViewMan::Header:
      case DocViewMan::Source:
        {
          KWriteDoc* pDoc = (KWriteDoc*) pDocViewNode->pDoc;
          if (pDoc->fileName() == strFileName)
            bFound = true;
        }
        break;
      case DocViewMan::HTML:
        {
          CDocBrowser* pDoc = (CDocBrowser*) pDocViewNode->pDoc;
          if (pDoc->currentURL() == strFileName)
            bFound = true;
        }
        break;
    }
  }
  if( !pDocViewNode || !bFound)
    return -1;
  else
    return pDocViewNode->docId;
}

void DocViewMan::installBMPopup(QPopupMenu *p)
{
  debug("DocViewMan::installBMPopup");

  connect(p,SIGNAL(aboutToShow()),SLOT(updateBMPopup()));
  connect(p,SIGNAL(activated(int)),SLOT(gotoBookmark(int)));
}

//------------------------------------------------------------------------------
// Updates the bookmarks for each document
//------------------------------------------------------------------------------
void DocViewMan::updateBMPopup()
{
  debug("DocViewMan::installBMPopup");

  QPopupMenu* popup = (QPopupMenu *) sender();

  // Remove all menu items
  popup->clear();

  // Insert separator
  popup->insertSeparator();

  // Update bookmarks for each document
  QList<int> allDocs = docs(DocViewMan::Header | DocViewMan::Source);
  QListIterator<int> docIter(allDocs);
  for ( ; docIter.current(); ++docIter) { // for all kwrite documents
    int curDocId = *(docIter.current());

    KWriteDoc* pDoc = (KWriteDoc*) docPointer(curDocId);
    pDoc->updateBMPopup(popup);
  }
}

//------------------------------------------------------------------------------
// shows the desired bookmark (eventually, switches to file and activates it)
//------------------------------------------------------------------------------
void DocViewMan::gotoBookmark(int n) {

  debug("DocViewMan::gotoBookmark : %d !\n", n);

  QPopupMenu* popup = (QPopupMenu *) sender();

  QString text = popup->text(n);

  // Find the KWriteDoc for this bookmark
  QList<int> allDocs = docs(DocViewMan::Header | DocViewMan::Source);
  QListIterator<int> docIter(allDocs);
  for ( ; docIter.current(); ++docIter) { // for all kwrite documents
    int curDocId = *(docIter.current());

    KWriteDoc* pDoc = (KWriteDoc*) docPointer(curDocId);
    if(text.startsWith(pDoc->fileName())) {
      m_pParent->switchToFile(pDoc->fileName());
      pDoc->gotoBookmark(text);
      return;
    }
  }
}


#include "docviewman.moc"
