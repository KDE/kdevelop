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
#include <qdir.h>
#include <qfileinfo.h>
#include <qlayout.h>
#include <qobjectlist.h>
#include <qprogressbar.h>

#include <kmessagebox.h>
#include <klocale.h>
#include <kdebug.h>

#include "ckdevelop.h"
#include "kwdoc.h"
#include "cdocbrowser.h"
#include "khtmlview.h"
#include "ceditwidget.h"
#include "docviewman.h"
#include "./dbg/brkptmanager.h"
#include "./dbg/vartree.h"
#include <kmessagebox.h>

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
        removeFileFromEditlist( pDoc->fileName()); // this removes from edit_infos and calls closeDoc right after
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

/*---------------------------------------- getInfoFromFilename
 * TEditInfo* DocViewMan::getInfoFromFilename(const QString &filename)
 *
 * searches for the edit_info-element
 *
 * Returns:
 *       returns either 0l if the filename wasn't found
 *       or the pointer of the first occurence inside
 *       the TEditInfoList
 *-----------------------------------------------------------------*/
TEditInfo* DocViewMan::getInfoFromFilename(const QString &filename)
{
  TEditInfo *pRetVal=0l;
  bool bSearch=true;
  QDir dir=QFileInfo(filename).dir(true);
  QString fullname=dir.canonicalPath()+"/"+QFileInfo(filename).fileName();

  // search the current file which would be changed
  for(pRetVal=edit_infos.first(); bSearch && pRetVal != 0l;)
  {
    if (pRetVal->filename == fullname )
      bSearch=false;
    else
      pRetVal=edit_infos.next();
  }
  return pRetVal;
}

void DocViewMan::synchronizeDocAndInfo()
{
  debug("DocViewMan::synchronizeDocAndInfo ! \n");

  // synchronize the "modified"-information of the KWriteDocs with the TEditInfo list
  QList<int> allKWriteDocs = docs(DocViewMan::Header | DocViewMan::Source);
  QListIterator<int> docIter(allKWriteDocs);
  for ( ; docIter.current(); ++docIter) { // for all kwrite documents
    int curKWriteDocId = *(docIter.current());
    // Because of our filter from above the doc object is always a KWriteDoc, and we'll always get id's for existing docs
    KWriteDoc* pDoc = (KWriteDoc*) docPointer( curKWriteDocId);
    setInfoModified(pDoc->fileName(), pDoc->isModified());
  }
}



void DocViewMan::removeFileFromEditlist(const char *filename){
  TEditInfo* actual_info;

  QString corrAbsFilename = (m_pParent->isUntitled(filename)) 
    ? QString(filename) 
    : QFileInfo(filename).absFilePath();

  //search the actual edit_info and remove it
  for(actual_info=edit_infos.first();actual_info != 0;){
    TEditInfo* next_info=edit_infos.next();
    if (actual_info->filename == corrAbsFilename){ // found
//      KDEBUG(KDEBUG_INFO,CKDEVELOP,"remove edit_info begin\n");
//      m_pParent->menu_buffers->removeItem(actual_info->id);
      if(edit_infos.removeRef(actual_info)){
//	KDEBUG(KDEBUG_INFO,CKDEVELOP,"remove edit_info end\n");
      }
    }
    actual_info=next_info;
  }

  int docId = findDoc(corrAbsFilename);
  closeDoc( docId);
}

/*---------------------------------------- setInfoModified()
 * setInfoModified(const QString &sFilename, bool bModified)
 *
 *  search all edit_infos for the file named "sFilename", the first
 *  match will change 'modified'
 *
 * Parameters:
 *  sFilename   filename to search in the EditInfos
 *  bModified   sets editinfo->modified
 * Returns:
 *       returns true if a struct-element was changed
 *-----------------------------------------------------------------*/
bool DocViewMan::setInfoModified(const QString &sFilename, bool bModified)
{
  bool bChanged=false;

  for(TEditInfo* actual_info = edit_infos.first();
      !bChanged && actual_info != 0;
      actual_info = edit_infos.next())
  {
   if ( actual_info->filename == sFilename)
      { // found
        actual_info->modified=bModified;
        bChanged=true;
      }
  }

  return bChanged;
}

bool DocViewMan::noInfoModified()
{
    int no_modif = true;

    for(TEditInfo* actual_info = edit_infos.first();
	no_modif && actual_info != 0;
	actual_info = edit_infos.next())
    {
       if (actual_info->modified)
         no_modif = false;
    }

    return no_modif;
}

TEditInfo* DocViewMan::findInfo(const QString &sFilename)
{
    TEditInfo* actual_info = 0;

    for(TEditInfo* search_info = edit_infos.first();
	search_info != 0 && actual_info == 0;
	search_info = edit_infos.next())
      {
	if (search_info->filename == sFilename)
	  actual_info = search_info;
      }

    return actual_info;
}

void DocViewMan::appendInfo(TEditInfo* info)
{
  edit_infos.append(info);
}

void DocViewMan::appendInfoFilenames(QStrList &fileList)
{
  for(TEditInfo* actual_info = edit_infos.first();
      actual_info != 0;
      actual_info = edit_infos.next())
    {
      fileList.append(actual_info->filename);
      debug(actual_info->filename);
    }
}

// closes all KWrite documents and their views but not the document browser views
void DocViewMan::doFileCloseAll()
{
  // Get the project name
  QString prjName = m_pParent->getProjectName();
  if (prjName != QString(""))
    prjName = "\n" + prjName + "\n\n";

  QStrList handledNames;
  bool cont=true;

  // Added by Christian
  synchronizeDocAndInfo();

  for(TEditInfo* actual_info = edit_infos.first();
      cont && actual_info != 0;)
  {
    TEditInfo *next_info=edit_infos.next();
    if(actual_info->modified && handledNames.contains(actual_info->filename)<1)
    {
#warning FIXME MessageBox needed with an extra button.
//      KMessageBox *files_close=
//        new KMessageBox(this,
//                        i18n("The project\n")+prjName
//                          +i18n("contains changed files. Save modified file\n\n")
//                          +actual_info->filename+" ?\n\n",
//                          i18n("Save changed files ?"),
//                          i18n("Yes"), i18n("No"), /*i18n("Save all"), */i18n("Cancel"));
//
//      // show the messagea and store result in result:
//
//      files_close->show();
//
//      int result=files_close->result();
      int result = KMessageBox::warningYesNoCancel(m_pParent,
                          i18n("The project %1\n"
                                "contains changed files. Save modified file\n\n%2 ?\n\n")
                          .arg(prjName)
                          .arg(actual_info->filename),
                          i18n("Save changed files ?"));

      // create the save project messagebox

      // what to do
      if(result==KMessageBox::Yes) // Yes- only save the current file
      {
        // save file as if Untitled and close file
        if(m_pParent->isUntitled(actual_info->filename))
        {
          m_pParent->switchToFile(actual_info->filename);
          handledNames.append(actual_info->filename);
          cont=m_pParent->fileSaveAs();
          next_info=edit_infos.first(); // start again... 'cause we deleted an entry
        }        
        else // Save file and close it
        {
          m_pParent->switchToFile(actual_info->filename);
          handledNames.append(actual_info->filename);
          m_pParent->slotFileSave();
          actual_info->modified=currentEditView()->isModified();
          cont=!actual_info->modified; //something went wrong
        }
      }

      else if(result==KMessageBox::No) // No - no save but close
      {
        handledNames.append(actual_info->filename);
        actual_info->modified=false;
        removeFileFromEditlist(actual_info->filename); // immediate remove
        next_info=edit_infos.first(); // start again... 'cause we deleted an entry
      }

//      if(result==3) // Save all
//      {
//        slotFileSaveAll();
//        break;
//      }

      else if(result==KMessageBox::Cancel) // Cancel
      {
        cont=false;
        break;
      }
    }  // end actual file close
    if (cont) {
      // close the document
      int docId = findDoc( actual_info->filename);
      closeDoc( docId); // this closes all views, automatically
    }
    actual_info=next_info;
  } // end for-loop

  // check if something went wrong with saving
  if ( cont )
  {
    cont = noInfoModified();

    if(cont)
    {
      // menu_buffers->clear();

      //clear all edit_infos before starting a new project
      edit_infos.clear();

    }
  }

}

bool DocViewMan::doProjectClose()
{
  debug("DocViewMan::doProjectClose !\n");

  TEditInfo* actual_info = 0;
  QStrList handledNames;
  bool cont = true;

  synchronizeDocAndInfo();

  for(actual_info = edit_infos.first(); cont && actual_info != 0;)
    {
//    KDEBUG1(KDEBUG_INFO,CKDEVELOP,"check file: %s",actual_info->filename.data());
      TEditInfo *next_info=edit_infos.next();
      if(actual_info->modified && handledNames.contains(actual_info->filename)<1)
	{
	  SaveAllDialog::SaveAllResult result = m_pParent->doProjectSaveAllDialog(actual_info->filename);
	  
	  // what to do
	  if(result==SaveAllDialog::Yes)
	    {  // Yes- only save the actual file
				// save file as if Untitled and close file
	      if(m_pParent->isUntitled(actual_info->filename))
		{
		  m_pParent->switchToFile(actual_info->filename);
		  handledNames.append(actual_info->filename);
		  cont=m_pParent->fileSaveAs();
		  // start again... 'cause we deleted an entry
		  next_info=edit_infos.first(); 
		}
				// Save file and close it
	      else
		{
		  m_pParent->switchToFile(actual_info->filename);
		  handledNames.append(actual_info->filename);
		  m_pParent->slotFileSave();
		  actual_info->modified = currentEditView()->isModified();
		  cont = !actual_info->modified; //something went wrong
		}
	    }
	  
	  if(result==SaveAllDialog::No)
	    {   // No - no save but close
	      handledNames.append(actual_info->filename);
	      actual_info->modified=false;
	      // immediate remove
	      removeFileFromEditlist(actual_info->filename);
	      // start again... 'cause we deleted an entry
	      next_info=edit_infos.first(); 
	    }
	  
	  if(result==SaveAllDialog::SaveAll)
	    {  // Save all
	      m_pParent->slotFileSaveAll();
	      break;
	    }
	  
	  if(result==SaveAllDialog::Cancel)
	    { // Cancel
	      cont=false;
	      break;
	    }
	}  // end actual file close
      actual_info=next_info;
    } // end for-loop
  
  // check if something went wrong with saving
  if (cont)
  {
    cont = noInfoModified();

    if(cont) 
      {
	edit_infos.clear();
      }
  }

  return cont;
}

bool DocViewMan::saveFileFromTheCurrentEditWidget()
{
  // Get the current file name
  QString filename = currentEditView()->getName();

  // Get it's info
  TEditInfo* actual_info = findInfo(filename);
  if (actual_info == 0)
    return false; //oops :-(

  // Ask if we should save it
  QFileInfo file_info(filename);
  if(file_info.lastModified() != actual_info->last_modified)
  {
    if (KMessageBox::No == KMessageBox::questionYesNo(m_pParent,

                    i18n("The file %1 was modified outside\n this editor.Save anyway?").arg(filename),
                    i18n("File modified")))
      return false;
  }

  // Really save it
  currentEditView()->doSave();
  QFileInfo file_info2(filename);
  actual_info->last_modified = file_info2.lastModified();

  return true;
}

void DocViewMan::saveModifiedFiles() 
{
  debug("DocViewMan::saveModifiedFiles ! \n");

  QProgressBar* pProgressBar = m_pParent->getProgressBar();
  ASSERT(pProgressBar);

  pProgressBar->setTotalSteps(edit_infos.count());
  pProgressBar->show();

  QStrList iFileList(false);
  bool mod=false;

  QStrList handledNames;
  TEditInfo* actual_info; //FB?, *cpp_info, *header_info;

  // check all edit_infos if they are modified outside; if yes, ask for saving
  for (actual_info = edit_infos.first(); actual_info != 0;) {
    int i = 0;
    TEditInfo *next_info = edit_infos.next();
    pProgressBar->setProgress(++i);
    	
    kdDebug() << "checking: " << actual_info->filename << "\n";
    kdDebug() << " " << ((actual_info->modified) ? "modified" : "not modified") << "\n";
    	
    if (!m_pParent->isUntitled(actual_info->filename)
        && actual_info->modified
        && handledNames.contains(actual_info->filename) < 1)
    {
      int qYesNo = KMessageBox::Yes;
      handledNames.append(actual_info->filename);
      	
      kdDebug() << " file info" << "\n";
      	
      QFileInfo file_info(actual_info->filename);
      if (file_info.lastModified() != actual_info->last_modified)
      {
        qYesNo = KMessageBox::questionYesNo(m_pParent,
                    i18n("The file %1 was modified outside\nthis editor. Save anyway?").arg(actual_info->filename),
                    i18n("File modified"));
      }
      	
      kdDebug() << " KMessageBox::Yes" << "\n";
      	
      if (qYesNo==KMessageBox::Yes)
      {
        kdDebug() << " create file_info" << "\n";
        QFileInfo file_info(actual_info->filename);
        bool isModified;
        		
        kdDebug() << " use blind widget " << "\n";		

        KWriteDoc* pDoc = new KWriteDoc(&m_highlightManager);
        CEditWidget* pBlindWidget = new CEditWidget(0L, 0, pDoc, DocViewMan::Undefined);
        pBlindWidget->setName(actual_info->filename);
        pBlindWidget->setText(actual_info->text);
        pBlindWidget->toggleModified(true);

        kdDebug() << "doSave" << "\n";		
        pBlindWidget->doSave();
        isModified = pBlindWidget->isModified();
        delete pBlindWidget;
        delete pDoc;
        		
        kdDebug() << "doing save " << ((!isModified) ? "success" : "failed") << "\n";
        		
        		
//FB?        if (actual_info==cpp_info)
//FB?           cpp_widget->setModified(isModified);
//FB?        if (actual_info==header_info)
//FB?           header_widget->setModified(isModified);
        		
        actual_info->modified = isModified;
        if (!isModified)
        {
#ifdef WITH_CPP_REPARSE
          mod=true;
#else
          mod|=(actual_info->filename.right(2)==".h" || actual_info->filename.right(4)==".hxx");
#endif
          iFileList.append(actual_info->filename);
          actual_info->last_modified = file_info.lastModified();
        }
      }
    }
    actual_info=next_info;
  }
  debug("end handledNames !\n");
  debug("end edit widget !\n");
  debug("stat prog ! \n");

  pProgressBar->reset();

  debug("refreshClassViewByFileList ! \n");
  if (m_pParent->hasProject() && !iFileList.isEmpty() && mod)
    m_pParent->refreshClassViewByFileList(&iFileList);
}

void DocViewMan::reloadModifiedFiles()
{
  QListIterator<TEditInfo> it(edit_infos); // iterator for edit_infos list

  TEditInfo* actual_info;
    
  for ( ; it.current(); ++it ) {
    actual_info = it.current();
    QFileInfo file_info(actual_info->filename);

    // Reload only changed files
    if(actual_info->last_modified != file_info.lastModified()){
      // Force reload, no modified on disc messagebox
      m_pParent->switchToFile(actual_info->filename,-1,-1,true,false); 
    }
  }
}


#include "docviewman.moc"
