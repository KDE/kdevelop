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
{
  m_docsAndViews.setAutoDelete(true);
  m_MDICoverList.setAutoDelete(true);
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
    if (pDoc == 0L) {
      return -1;  // creation failed
    }
    break;
  case DocViewMan::HTML:
    pDoc = new CDocBrowser(0L, "browser");
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
        //remove the view from MDI and delete it
        pView->hide();
        //XXXXX MDI-remove XXXXXX
        //...
        delete pView;
      }
      pCurDocViewNode->existingViews.clear();
      // emit an according signal if we closed the last view
      if (countViews() == 0) {
        emit sig_lastViewClosed();
      }

      KWriteDoc* pDoc = (KWriteDoc*) pCurDocViewNode->pDoc;
  //?      //   disconnect document signals
  //?       disconnect(pDoc, SIGNAL(sig_updated(QObject*, int)),
  //?                  this, SIGNAL(sig_updated(QObject*, int)));
      delete pDoc;
    }
  case DocViewMan::HTML:
    {
      CDocBrowser* pDoc = (CDocBrowser*) pCurDocViewNode->pDoc;
      delete pDoc;
    }
    break;
  }

  //   remove list entry
  m_docsAndViews.remove(pCurDocViewNode);
  //   emit an according signal if we closed the last doc
  if (m_docsAndViews.count() == 0) {
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
// get number of documents
//------------------------------------------------------------------------------
int DocViewMan::countDocs() const
{
  return m_docsAndViews.count();
}

//------------------------------------------------------------------------------
// get the ids of all documents
//------------------------------------------------------------------------------
QList<int> DocViewMan::docs() const
{
  QListIterator<DocViewNode>    itDoc(m_docsAndViews);
  QList<int> listDocIds;
  for (; itDoc.current() != 0; ++itDoc) {
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

  // cause a view to be created
  QWidget*   pNewView = 0;

  switch (docType( docId)) {
  case DocViewMan::Header:
  case DocViewMan::Source:
    {
      KWriteDoc* pDoc = (KWriteDoc*) pDocViewNode->pDoc;
      // create the view and add to MDI
      pNewView = new CEditWidget(0L, "autocreatedview", pDoc);
      pNewView->setCaption( pDoc->fileName());
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

  // cover it by a QextMDI childview and add that MDI system
  QextMdiChildView* pMDICover = new QextMdiChildView( pNewView->caption());
  m_MDICoverList.append( pMDICover);
  QBoxLayout* pLayout = new QHBoxLayout( pMDICover, 0, -1, "layout");
  pNewView->reparent( pMDICover, QPoint(0,0));
  QApplication::sendPostedEvents();
  pLayout->addWidget( pNewView);
  pMDICover->setName( pNewView->name());
  m_pParent->addWindow( pMDICover, QextMdi::StandardAdd);
  // captions
  QString shortName = pNewView->caption();
  int length = shortName.length();
  shortName = shortName.right(length - (shortName.findRev('/') +1));
  pMDICover->setTabCaption( shortName);
  // show
  pMDICover->show();

  // connect signals
//?       connect(pNewView, SIGNAL(sig_updated(QObject*, int)),
//?               this, SIGNAL(sig_updated(QObject*, int)));
  connect(pMDICover, SIGNAL(gotFocus(QextMdiChildView*)),
          this, SLOT(slot_gotFocus(QextMdiChildView*)));

  // connect document to view
  if (docId >= 0) {
    // add view to view list of doc
    pDocViewNode->existingViews.append(pNewView);
  }

  // the connect to this slot was made _after_ the first focusIn event, so we must fake this slot call
  slot_gotFocus( pMDICover);

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

  // store the current items since lists may change while deleting view and doc
  DocViewNode*   pDocViews = m_docsAndViews.current();
  // the view was found
  // disconnect the focus signals
  disconnect(pView->parentWidget(), SIGNAL(gotFocus(QextMdiChildView*)),
             this, SLOT(slot_gotFocus(QextMdiChildView*)));
  // remove view list entry
  pViewList->remove(pView);

  // remove the view from MDI and delete the view
  QextMdiChildView* pMDICover = (QextMdiChildView*) pView->parentWidget();
  m_pParent->removeWindowFromMdi( pMDICover);
  delete pMDICover;

  // emit an according signal if we closed the last view
  if (countViews() == 0) {
    emit sig_lastViewClosed();
  }
  // check whether there are remaining views
  if (pDocViews->existingViews.count() == 0) {
    // no -> delete document
    delete pDocViews->pDoc;
    //   remove list entry
    m_docsAndViews.remove(pDocViews);
    //   did we close the last doc?
    if (countDocs() == 0) {
      emit sig_lastDocClosed();
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
  QObject* pDoc = 0L;
  QListIterator<DocViewNode>    itDoc(m_docsAndViews);
  for (; itDoc.current() != 0 && !pDoc; ++itDoc) {
    QListIterator<QWidget>  itView(itDoc.current()->existingViews);
    for (; itView.current() != 0; ++itView) {
      if (itView.current() == pView) {
        // view found
        pDoc = itDoc.current()->pDoc;
      }
    }
  }
  if (pView->inherits("CEditWidget"))
    m_pCurEditDoc = (KWriteDoc*) pDoc;
  else
    m_pCurBrowserDoc = (CDocBrowser*) pDoc;

  // emit the got focus signal
  emit sig_viewGotFocus(pView);
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

#include "docviewman.moc"
