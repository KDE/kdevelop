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
#include <qwhatsthis.h>

#include <kmessagebox.h>
#include <klocale.h>
#include <kdebug.h>

#include "ckdevelop.h"
#include "kwdoc.h"
#include "cdocbrowser.h"
#include "doctreeview.h"
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
  ,m_curIsBrowser(true)
{
  debug("create docviewman !\n");

  m_MDICoverList.setAutoDelete(true);

  connect( this, SIGNAL(sig_viewGotFocus(QWidget*,int)), m_pParent, SLOT(slotViewSelected(QWidget*,int)) );
}

//------------------------------------------------------------------------------
DocViewMan::~DocViewMan()
{
  debug("delete docviewman !\n");
}

void DocViewMan::doSelectURL(const QString& url)
{
  debug("DocViewMan::doSelectURL !\n");

  // use latest focused browser document or create new browser document and view
  CDocBrowser* pCurBrowserDoc = currentBrowserDoc();
  KHTMLView* pBrowserView = currentBrowserView();
  if (pCurBrowserDoc) {
    if(url.contains("kdevelop/search_result.html") != 0){
      pCurBrowserDoc->showURL(url, true); // with reload if equal
    }
    else{
      pCurBrowserDoc->showURL(url); // without reload if equal
    }
  }
  else {
    CDocBrowser* pDoc = createCDocBrowser(url);
    if (pDoc == 0) return; // failed
    pBrowserView = createBrowserView(pDoc);
  }

  pBrowserView->parentWidget()->setFocus();
}

void DocViewMan::doSwitchToFile(QString filename, int line, int col, bool bForceReload, bool bShowModifiedBox)
{
  debug("DocViewMan::doSwitchToFile : '%s' !\n", filename.data());

  debug("show modified box : %d !\n", bShowModifiedBox);

  CEditWidget* pCurEditWidget = currentEditView();
  KWriteDoc* pCurEditDoc = currentEditDoc();

  QString editWidgetName;
  if (pCurEditWidget) {
    debug("getting edit widget name !");
    editWidgetName = pCurEditWidget->getName(); //FB
    debug("editWidgetName : '%s' !", editWidgetName.data());
  }
  else {
    debug("editWidgetName : '' !");
  }

  debug("looking if file already open !\n");

  // Make sure that we found the file in the editor_widget in our list
  if (pCurEditDoc) {
    // handle file if it was modified on disk by another editor/cvs
    QFileInfo file_info(editWidgetName);
    if ((file_info.lastModified() != pCurEditDoc->getLastFileModifDate()) && bShowModifiedBox) {
      debug(" KMessageBox !\n");
      if(KMessageBox::questionYesNo(m_pParent,
                                    i18n("The file %1 was modified outside this editor.\n"
                                         "Open the file from disk and delete the current Buffer?")
                                    .arg(editWidgetName),
                                    i18n("File modified"))==KMessageBox::Yes) {
        bForceReload = true;
        pCurEditDoc->setLastFileModifDate(file_info.lastModified());
      }
    }

    debug(" getting lastModified !\n");

    if (!bShowModifiedBox) {
      pCurEditDoc->setLastFileModifDate(file_info.lastModified());
    }

    debug(" before setCursorPosition !\n");

    if (!bForceReload && filename == editWidgetName) {
      if (pCurEditWidget && (line != -1))
        pCurEditWidget->setCursorPosition(line, col);

      //    cerr << endl <<endl << "Filename:" << filename
      // << "EDITNAME:" << pCurEditWidget->getName() <<"no action---:" << endl;
      QextMdiChildView* pMDICover = (QextMdiChildView*) pCurEditWidget->parentWidget();
      pMDICover->activate();
      return;
    }
  }

  // See if we already have the file wanted.
  KWriteDoc* pDoc = findKWriteDoc(filename);

  // bool found = (pDoc != 0);

  debug("getting document type !\n");

  // Not found or needing a reload causes the file to be read from disk
  if ((!pDoc) || bForceReload) {
    QFileInfo fileinfo(filename);
    if (!pDoc) {
      debug("Create a new doc !\n");
      pDoc = createKWriteDoc(filename);
      if (pDoc) {
        // Set the last modify date
        pDoc->setLastFileModifDate(fileinfo.lastModified());

        qDebug("createView for a new created doc");
        pCurEditWidget = createEditView(pDoc);
      }
    }
    else {
      // a view for this doc exists, already;
      // use the first view we found of this doc to show the text
      pCurEditWidget = getFirstEditView(pDoc);
      qDebug("found view in list of doc");
    }
    loadKWriteDoc(pDoc , filename, 1);

    qDebug("and loadDoc");
  }
  else {
    debug(" document type found !\n");

    KWriteDoc* pDoc = findKWriteDoc(filename);
    pCurEditWidget = getFirstEditView(pDoc);

    debug(" focus view !\n");

    // Don't use the saved text because it is useless
    // and removes the bookmarks
    // pCurEditWidget->setText(info->text);

    qDebug("doc (and at least 1 view) did exist, raise it");
  }

  if (!pCurEditWidget)
    return;

  // debug(" toggle modify cur edit widget !\n");
  // pCurEditWidget->toggleModified(info->modified);

  // If the caller wanted to be positioned at a particular place in the file
  // then they have supplied the line and col. Otherwise we use the
  // current info values (0 if new) for the placement.
  if (line != -1)
    pCurEditWidget->setCursorPosition(line, col);
  // else
  //  pCurEditWidget->setCursorPosition(info->cursor_line,info->cursor_col);

  pCurEditWidget->setName(filename);
  // info->text = pCurEditWidget->text();

  debug(" set focus on view (this will raise and activate the MDI view!\n");
  QextMdiChildView* pMDICover = (QextMdiChildView*) pCurEditWidget->parentWidget();
  pMDICover->activate();
}


void DocViewMan::doOptionsEditor()
{
  debug("DocViewMan::doOptionsEditor !\n");

  if(currentEditView())
  {
    currentEditView()->optDlg();
    doTakeOverOfEditorOptions();
  }
}

void DocViewMan::doOptionsEditorColors()
{
  debug("DocViewMan::doOptionsEditorColors !\n");

  if(currentEditView())
  {
    currentEditView()->colDlg();
    doTakeOverOfEditorOptions();
  }
}


void DocViewMan::doOptionsSyntaxHighlightingDefaults()
{
  debug("DocViewMan::doOptionsSyntaxHighlightingDefaults !\n");

  if(currentEditView())
  {
    currentEditView()->hlDef();
    doTakeOverOfEditorOptions();
  }
}

void DocViewMan::doOptionsSyntaxHighlighting()
{
  debug("DocViewMan::doOptionsSyntaxHighlighting !\n");

  if(currentEditView())
  {
    currentEditView()->hlDlg();
    doTakeOverOfEditorOptions();
  }
}

/** shared helper function for the 4 slots 
  * doOptionsEditor, doOptionsEditorColors,
  * doOptionsSyntaxHighlightingDefaults and doOptionsSyntaxHighlighting
  */
void DocViewMan::doTakeOverOfEditorOptions()
{
  debug("DocViewMan::doTakeOverOfEditorOptions !\n");

  KConfig* config = m_pParent->getConfig();
  if (config) {
    config->setGroup("KWrite Options");
    currentEditView()->writeConfig(config);
    currentEditView()->doc()->writeConfig(config);

    QListIterator<QObject> itDoc(m_documentList);
    for (itDoc.toFirst(); itDoc.current() != 0; ++itDoc) {
      KWriteDoc* pDoc = dynamic_cast<KWriteDoc*> (itDoc.current());
      if (pDoc) {
        CEditWidget* pCurEW = getFirstEditView(pDoc);
        pCurEW->copySettings(currentEditView());
        config->setGroup("KWrite Options");
        pCurEW->readConfig(config);
        pDoc->readConfig(config);
      }
    }
  }
}

/** */
void DocViewMan::doCopy()
{
  debug("DocViewMan::doCopy !\n");

  if (currentEditView()) {
    if (curDocIsBrowser())
      currentBrowserDoc()->slotCopyText();
    else
      currentEditView()->copyText();
  }
}

/** */
void DocViewMan::doSearch()
{
  debug("DocViewMan::doSearch !\n");

  if (curDocIsBrowser())
    currentBrowserDoc()->doSearchDialog();
  else
    currentEditView()->search();
}

/** */
void DocViewMan::doRepeatSearch(QString &search_text, int back)
{
  debug("DocViewMan::doRepeatSearch !\n");

  if (currentEditView()) {
    if (curDocIsBrowser())
      currentBrowserDoc()->findTextNext(QRegExp(search_text),true);
    else
      currentEditView()->searchAgain(back==1);
  }
}

/** */
void DocViewMan::doSearchText(QString &text)
{
  debug("DocViewMan::doSearchText !\n");

  if (currentEditView()) {
    if (curDocIsBrowser())
      text = currentBrowserDoc()->selectedText();
    else {
      text = currentEditView()->markedText();
      if(text == "") {
        text = currentEditView()->currentWord();
      }
    }
  }
}

/** */
void DocViewMan::doClearBookmarks()
{
  debug("DocViewMan::doClearBookmarks !\n");

  QListIterator<QObject> itDoc(m_documentList);
  for (itDoc.toFirst(); itDoc.current() != 0; ++itDoc) {
    KWriteDoc* pDoc = dynamic_cast<KWriteDoc*> (itDoc.current());
    if (pDoc) {
      pDoc->clearBookmarks();
    }
  }
}

void DocViewMan::doCreateNewView()
{
  debug("DocViewMan::doCreateNewView !\n");

  QWidget* pNewView = 0L;

  if(curDocIsBrowser()) {
    CDocBrowser* pBrowserDoc = createCDocBrowser(DocTreeKDevelopBook::locatehtml("about/intro.html"));
    pNewView = createBrowserView(pBrowserDoc);
  }
  else {
    pNewView = createEditView(currentEditDoc());
  }

  // raise and activate
  if (pNewView)
    pNewView->parentWidget()->setFocus();
}

/** */
bool DocViewMan::curDocIsHeaderFile()
{
  debug("DocViewMan::curDocIsHeaderFile !\n");

  return (!curDocIsBrowser()
    && m_pCurEditDoc
    && (getKWriteDocType(m_pCurEditDoc) == CPP_HEADER));
}

/** */
bool DocViewMan::curDocIsCppFile()
{
  debug("DocViewMan::curDocIsCppFile !\n");

  return (!curDocIsBrowser() 
	  && m_pCurEditDoc
	  && (getKWriteDocType(m_pCurEditDoc) == CPP_SOURCE));
}


ProjectFileType DocViewMan::getKWriteDocType(KWriteDoc* pDoc)
{
  debug("DocViewMan::getKWriteDocType !\n");

  return CProject::getType(pDoc->fileName());
}

KWriteDoc* DocViewMan::createKWriteDoc(const QString& strFileName)
{
  debug("creating KWriteDoc ");

  KWriteDoc* pDoc = new KWriteDoc(&m_highlightManager, strFileName);
  if (!pDoc)
    return 0L;

  QFileInfo file_info(strFileName);
    
  // Add the new doc to the list
  m_documentList.append(pDoc);

  KConfig* config = m_pParent->getConfig();
  if(config) {
    config->setGroup("KWrite Options");
    pDoc->readConfig(config);
  }

  pDoc->setFileName(strFileName);

  // Return the new document
  return pDoc; 
}

CDocBrowser* DocViewMan::createCDocBrowser(const QString& url)
{
  debug("DocViewMan::createCDocBrowser !\n");

  CDocBrowser* pDocBr = new CDocBrowser(0L, "browser");

  if(pDocBr) {
    // some signal-slot connections
    connect(pDocBr, SIGNAL(completed()),m_pParent, SLOT(slotDocumentDone()));
    connect(pDocBr, SIGNAL(signalURLBack()),m_pParent,SLOT(slotHelpBack()));
    connect(pDocBr, SIGNAL(signalURLForward()),m_pParent,SLOT(slotHelpForward()));
    connect(pDocBr, SIGNAL(signalBookmarkToggle()),m_pParent,SLOT(slotBookmarksToggle()));
    connect(pDocBr, SIGNAL(onURL(const QString&)),m_pParent,SLOT(slotURLonURL(const QString&)));
    connect(pDocBr, SIGNAL(signalSearchText()),m_pParent,SLOT(slotHelpSearchText()));
    //  connect(pDocBr, SIGNAL(goRight()), m_pParent, SLOT(slotHelpForward()));
    //  connect(pDocBr, SIGNAL(goLeft()), m_pParent, SLOT(slotHelpBack()));
    connect(pDocBr, SIGNAL(enableStop(int)), m_pParent, SLOT(enableCommand(int)));	
    connect(pDocBr->popup(), SIGNAL(highlighted(int)), m_pParent, SLOT(statusCallback(int)));
    connect(pDocBr, SIGNAL(signalGrepText(QString)), m_pParent, SLOT(slotEditSearchInFiles(QString)));
    //  connect(pDocBr, SIGNAL(textSelected(KHTMLPart *, bool)),m_pParent,SLOT(slotBROWSERMarkStatus(KHTMLView *, bool)));
    
    // init browser and assign URL
    pDocBr->setDocBrowserOptions();
    pDocBr->showURL(url, true); // with reload if equal

    // Add the new doc to the list
    m_documentList.append(pDocBr);
  }

  debug("End DocViewMan::createCDocBrowser !\n");

  // Return the new document
  return pDocBr; 
}

//-----------------------------------------------------------------------------
// load edit document from file
//-----------------------------------------------------------------------------
void DocViewMan::loadKWriteDoc(KWriteDoc* pDoc, 
                               const QString& strFileName, 
                               int /*mode*/)
{
  debug("DocViewMan::loadKWriteDoc !\n");

  if(QFile::exists(strFileName)) {
    QFile f(strFileName);
    if (f.open(IO_ReadOnly)) {
      pDoc->loadFile(f);
      f.close();
    }
  }
}

//-----------------------------------------------------------------------------
// save document to file
//-----------------------------------------------------------------------------
bool DocViewMan::saveKWriteDoc(KWriteDoc* pDoc, const QString& strFileName)
{
  debug("DocViewMan::saveKWriteDoc !\n");

  QFileInfo info(strFileName);
  if(info.exists() && !info.isWritable()) {
    KMessageBox::sorry(0L, i18n("You do not have write permission to this file"));
    return false;
  }
  
  QFile f(strFileName);
  if (f.open(IO_WriteOnly | IO_Truncate)) {
    pDoc->writeFile(f);
    pDoc->updateViews();
    f.close();
    return true;//kWriteDoc->setFileName(name);
  }
  KMessageBox::sorry(0L,  i18n("An Error occured while trying to open this Document"));
  return false;
}

//-----------------------------------------------------------------------------
// Find if there is another KWriteDoc in the doc list 
//-----------------------------------------------------------------------------
KWriteDoc* DocViewMan::findKWriteDoc()
{
  debug("DocViewMan::findKWriteDoc !\n");

  QListIterator<QObject> itDoc(m_documentList);
  for (itDoc.toFirst(); itDoc.current() != 0; ++itDoc) {
    KWriteDoc* pDoc = dynamic_cast<KWriteDoc*> (itDoc.current());
    if(pDoc) {
      return pDoc;
    }
  }
  return 0L;
}

//-----------------------------------------------------------------------------
// close an edit document, causes all views to be closed
//-----------------------------------------------------------------------------
void DocViewMan::closeKWriteDoc(KWriteDoc* pDoc)
{
  debug("DocViewMan::closeKWriteDoc !\n");
  if (!pDoc) return;

  QList<KWriteView> views = pDoc->viewList();
  QListIterator<KWriteView>  itViews(views);
  for (; itViews.current() != 0; ++itViews) {
    CEditWidget* pView = (CEditWidget*) itViews.current()->parentWidget();
    if (!pView) continue;
    disconnect(pView, SIGNAL(gotFocus(CEditWidget*)),
               this, SLOT(slot_gotFocus(CEditWidget*)));
    // remove the view from MDI and delete the view
    QextMdiChildView* pMDICover = (QextMdiChildView*) pView->parentWidget();
    m_pParent->removeWindowFromMdi( pMDICover);
    m_MDICoverList.remove( pMDICover);
  }

  if(pDoc) {
    // Remove the document from the list
    m_documentList.removeRef(pDoc);
    // now finally, delete the document
    delete pDoc;
  }

  // check if there's still a m_pCurBrowserDoc, m_pCurBrowserView, 
  // m_pCurEditDoc, m_pCurEditView
  KWriteDoc* pNewDoc = findKWriteDoc();

  if (pNewDoc == 0) {
    m_pCurEditDoc = 0L;
    m_pCurEditView = 0L;
  }

  //   emit an according signal if we closed the last doc
  if (m_documentList.count() == 0) {
    emit sig_lastViewClosed();
    emit sig_lastDocClosed();
  }
}

//-----------------------------------------------------------------------------
// Find if there is another CDocBrowser in the doc list 
//-----------------------------------------------------------------------------
CDocBrowser* DocViewMan::findCDocBrowser()
{
  debug("DocViewMan::findCDocBrowser !\n");

  QListIterator<QObject> itDoc(m_documentList);
  for (itDoc.toFirst(); itDoc.current() != 0; ++itDoc) {
    CDocBrowser* pDoc = dynamic_cast<CDocBrowser*> (itDoc.current());
    if(pDoc) {
      return pDoc;
    }
  }
  return 0L;
}

//-----------------------------------------------------------------------------
// close a browser document, causes all views to be closed
//-----------------------------------------------------------------------------
void DocViewMan::closeCDocBrowser(CDocBrowser* pDoc)
{
  debug("DocViewMan::closeCDocBrowser : %d !\n", (int)pDoc);

  if(pDoc) {
    debug("getting pView !\n");
    KHTMLView* pView = pDoc->view();
    debug("pView : %d !\n", (int)pView);
    if(pView) {
      debug("getting pMDICover !\n");
      QextMdiChildView* pMDICover = (QextMdiChildView*) pView->parentWidget();
      if(pMDICover) {
        debug("pMDICover : %d !\n", (int)pMDICover);
        debug("hiding pMDICover !\n");
        pMDICover->hide();
        debug("reparent pView !\n");
        pView->reparent(0L, 0, QPoint(0,0));
        QApplication::sendPostedEvents();
        m_pParent->removeWindowFromMdi( pMDICover);
        m_MDICoverList.remove( pMDICover);
      }
    }
    debug("deleting pDoc !\n");
    // Remove the document from the list
    m_documentList.removeRef(pDoc);
    // now finally, delete the document (which inclusively deletes the view)
    delete pDoc;
  }

  debug("finding new doc !\n");
  CDocBrowser* pNewDoc = findCDocBrowser();
  if (pNewDoc == 0) {
    m_pCurBrowserDoc = 0L;
    m_pCurBrowserView = 0L;
  }
  
  //   emit an according signal if we closed the last doc
  debug("counting documents !\n");
  if (m_documentList.count() == 0) {
    emit sig_lastViewClosed();
    emit sig_lastDocClosed();
  }
}

//-----------------------------------------------------------------------------
// retrieve the document pointer
//-----------------------------------------------------------------------------
/*
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
*/
//-----------------------------------------------------------------------------
// retrieve the document pointer and make sure it is a KWriteDoc
//-----------------------------------------------------------------------------
/*
KWriteDoc* DocViewMan::kwDocPointer(int docId) const
{
  return (dynamic_cast<KWriteDoc*> (docPointer(docId)));
}
*/

//-----------------------------------------------------------------------------
// cover a newly created view with a QextMDI childview 
//-----------------------------------------------------------------------------
void DocViewMan::addQExtMDIFrame(QWidget* pNewView)
{
  debug("DocViewMan::addQExtMDIFrame !\n");

  if (!pNewView) return;  // failed, could not create view

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

}

//-----------------------------------------------------------------------------
// create a new view for an edit document
//-----------------------------------------------------------------------------
CEditWidget* DocViewMan::createEditView(KWriteDoc* pDoc)
{
  debug("DocViewMan::createEditView !\n");

  // create the view and add to MDI
  CEditWidget* pEW = new CEditWidget(0L, "autocreatedview", pDoc);
  if(!pEW) return 0L;
  pEW->setCaption(pDoc->fileName());

  //connect the editor lookup function with slotHelpSText
  connect( pEW, SIGNAL(manpage(QString)),m_pParent, SLOT(slotHelpManpage(QString)));
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

  if (getKWriteDocType(pDoc) == CPP_SOURCE) {
    connect( pEW, SIGNAL(markStatus(KWriteView *, bool)), m_pParent, SLOT(slotCPPMarkStatus(KWriteView *, bool)));
  } else {
    connect( pEW, SIGNAL(markStatus(KWriteView *, bool)), m_pParent, SLOT(slotHEADERMarkStatus(KWriteView *, bool)));
  }
  
  // add view to view list of doc
  //   pDocViewNode->existingViews.append(pNewView);

  // Cover it by a QextMDI childview and add that MDI system
  addQExtMDIFrame(pEW);

  // some additional settings
  pEW->setFocusPolicy(QWidget::StrongFocus);
  pEW->setFont(KGlobalSettings::fixedFont());
  KConfig* config = m_pParent->getConfig();
  if(config) {
    config->setGroup("KWrite Options");
    pEW->readConfig(config);
  }

  return pEW;
}

//-----------------------------------------------------------------------------
// create a new view for a browser document
//-----------------------------------------------------------------------------
KHTMLView* DocViewMan::createBrowserView(CDocBrowser* pDoc)
{
  debug("DocViewMan::createBrowserView !\n");

  KHTMLView* pNewView = pDoc->view();
  pNewView->setCaption( pDoc->currentTitle());
  // add "what's this" entry
  m_pParent->getWhatsThis()->add(pNewView, i18n("Documentation Browser\n\n"
            "The documentation browser window shows the online-"
            "documentation provided with kdevelop as well as "
            "library class documentation created. Use the documentation "
            "tree to switch between various parts of the documentation."));
  pDoc->showURL(pDoc->currentURL(), true); // with reload if equal

  // Cover it by a QextMDI childview and add that MDI system
  addQExtMDIFrame(pNewView);
  
  return pNewView;
}

//-----------------------------------------------------------------------------
// close a view
//-----------------------------------------------------------------------------
void DocViewMan::closeView(QWidget* pView)
{
  debug("DocViewMan::closeView !\n");

  CEditWidget* pEditView = dynamic_cast<CEditWidget*> (pView);
  if(pEditView) closeEditView(pEditView);

  KHTMLView* pHTMLView = dynamic_cast<KHTMLView*> (pView);
  if(pHTMLView) closeBrowserView(pHTMLView);
}

//-----------------------------------------------------------------------------
// close an edit view
//-----------------------------------------------------------------------------
void DocViewMan::closeEditView(CEditWidget* pView)
{
  debug("DocViewMan::closeEditView !\n");
  if (!pView) return;

  // Get the document
  KWriteDoc* pDoc = pView->doc();

  QextMdiChildView* pMDICover = (QextMdiChildView*) pView->parentWidget();
  pMDICover->hide();
  
  // disconnect the focus signals
  disconnect(pMDICover, SIGNAL(gotFocus(QextMdiChildView*)), this, SLOT(slot_gotFocus(QextMdiChildView*)));

  // remove the view from MDI and delete the view
  m_pParent->removeWindowFromMdi( pMDICover);
  m_MDICoverList.remove( pMDICover);

  if (pDoc->viewCount() == 0) {
    closeKWriteDoc(pDoc);
  }
}

//-----------------------------------------------------------------------------
// close a browser view
//-----------------------------------------------------------------------------
void DocViewMan::closeBrowserView(KHTMLView* pView)
{
  debug("DocViewMan::closeBrowserView !\n");

  CDocBrowser* pDoc = (CDocBrowser*) pView->part();

  QextMdiChildView* pMDICover = (QextMdiChildView*) pView->parentWidget();
  pMDICover->hide();
  
  // disconnect the focus signals
  disconnect(pMDICover, SIGNAL(gotFocus(QextMdiChildView*)), this, SLOT(slot_gotFocus(QextMdiChildView*)));
  
  // get a KHTMLView out of the parent to avoid a delete, 
  // it will be deleted later in the CDocBrowser destructor
  pView->reparent(0L,0,QPoint(0,0));
  QApplication::sendPostedEvents();
  
  // remove the view from MDI and delete the view
  m_pParent->removeWindowFromMdi( pMDICover);
  m_MDICoverList.remove( pMDICover);

  closeCDocBrowser(pDoc);
}

//-----------------------------------------------------------------------------
// get the first edit view for a document
//-----------------------------------------------------------------------------
CEditWidget* DocViewMan::getFirstEditView(KWriteDoc* pDoc) const
{
  debug("DocViewMan::getFirstEditView !\n");

  return (dynamic_cast<CEditWidget*> (pDoc->getKWrite()));
}

//-----------------------------------------------------------------------------
// Connected to the focus in event occures signal of CEditWidget.
// Moves the focused view to the end of the list of focused views or
// adds it. Emits the sig_viewGotFocus signal
//-----------------------------------------------------------------------------
void DocViewMan::slot_gotFocus(QextMdiChildView* pMDICover)
{
  debug("DocViewMan::slot_gotFocus !\n");

  // set current view, distinguish between edit widget and browser widget
  QObjectList* pL = (QObjectList*) pMDICover->children();
  QWidget* pView = 0L;
  QObject* pChild;
  for ( pChild = pL->first(); pChild && !pView; pChild = pL->next()) {
    if (pChild->inherits("QWidget")) {
      pView = (QWidget*) pChild;
    }
  }

  if (pView->inherits("CEditWidget")) {
    m_pCurEditView = (CEditWidget*) pView;
    m_pCurEditDoc = m_pCurEditView->doc();
    m_curIsBrowser = false;
  } else {
    m_pCurBrowserView = (KHTMLView*) pView;
    m_pCurBrowserDoc = (CDocBrowser*) m_pCurBrowserView->part();
    m_curIsBrowser = true;
  }

  debug("pView : %d !", (int)pView);
  
  // emit the got focus signal 
  // (connected to CKDevelop but could also be caught by other ones)
  emit sig_viewGotFocus(pView);

  debug("emit the got focus signal !\n");
}

//-----------------------------------------------------------------------------
// Returns the number of documents
//-----------------------------------------------------------------------------
int DocViewMan::docCount() const
{
  debug("DocViewMan::docCount !\n");

  return m_documentList.count();
}

//-----------------------------------------------------------------------------
// Retrieves the KWriteDoc found by its filename
//-----------------------------------------------------------------------------
KWriteDoc* DocViewMan::findKWriteDoc(const QString& strFileName) const
{
  debug("DocViewMan::findKWriteDoc !\n");

  QListIterator<QObject> itDoc(m_documentList);
  for (itDoc.toFirst(); itDoc.current() != 0; ++itDoc) {
    KWriteDoc* pDoc = dynamic_cast<KWriteDoc*> (itDoc.current());
    if(pDoc && (pDoc->fileName() == strFileName)) {
      return pDoc;
    }
  }
  return 0L;
}

//-----------------------------------------------------------------------------
// Retrieves any Document found by its filename
//-----------------------------------------------------------------------------
QObject* DocViewMan::findDocFromFilename(const QString& strFileName) const
{
  debug("DocViewMan::findDocFromFilename !\n");

  QListIterator<QObject> itDoc(m_documentList);

  for (; itDoc.current() != 0; ++itDoc) {
    QObject* doc = itDoc.current();
    if (doc  && (docName(doc) == strFileName)) {
      return doc;
    }
  }

  return 0L;
}

//-----------------------------------------------------------------------------
// Connect to the signals of the bookmark popup menu
//-----------------------------------------------------------------------------
void DocViewMan::installBMPopup(QPopupMenu *p)
{
  debug("DocViewMan::installBMPopup");

  connect(p,SIGNAL(aboutToShow()),SLOT(updateBMPopup()));
  connect(p,SIGNAL(activated(int)),SLOT(gotoBookmark(int)));
}

//-----------------------------------------------------------------------------
// Updates the bookmarks for each document
//-----------------------------------------------------------------------------
void DocViewMan::updateBMPopup()
{
  debug("DocViewMan::installBMPopup");

  QPopupMenu* popup = (QPopupMenu *) sender();

  // Remove all menu items
  popup->clear();

  // Insert separator
  popup->insertSeparator();

  // Update bookmarks for each document
  QListIterator<QObject> itDoc(m_documentList);
  for (itDoc.toFirst(); itDoc.current() != 0; ++itDoc) {
    KWriteDoc* pDoc = dynamic_cast<KWriteDoc*> (itDoc.current());
    if(pDoc) {
      pDoc->updateBMPopup(popup);
    }
  }
}

//-----------------------------------------------------------------------------
// shows the desired bookmark (eventually, switches to file and activates it)
//-----------------------------------------------------------------------------
void DocViewMan::gotoBookmark(int n) {

  debug("DocViewMan::gotoBookmark : %d !\n", n);

  QPopupMenu* popup = (QPopupMenu *) sender();

  QString text = popup->text(n);

  // Find the KWriteDoc for this bookmark
  QListIterator<QObject> itDoc(m_documentList);
  for (itDoc.toFirst(); itDoc.current() != 0; ++itDoc) {
    KWriteDoc* pDoc = dynamic_cast<KWriteDoc*> (itDoc.current());
    if(pDoc) {
      if(text.startsWith(pDoc->fileName())) {
        m_pParent->switchToFile(pDoc->fileName());
        pDoc->gotoBookmark(text);
        return;
      }
    }
  }
}


//-----------------------------------------------------------------------------
// Find if no documents have been modified 
//-----------------------------------------------------------------------------
bool DocViewMan::noDocModified()
{
  debug("DocViewMan::noDocModified !\n");

  QListIterator<QObject> itDoc(m_documentList);
  for (itDoc.toFirst(); itDoc.current() != 0; ++itDoc) {
    KWriteDoc* pDoc = dynamic_cast<KWriteDoc*> (itDoc.current());
    if(pDoc && pDoc->isModified()) {
      return false;
    }
  }
  return true;
}


void DocViewMan::doFileSave(bool project)
{
  debug("DocViewMan::doFileSave !\n");

  QString filename = docName(m_pCurEditDoc);

  // save the current file
  saveFileFromTheCurrentEditWidget();
  // setInfoModified(filename, currentEditView()->isModified());
  QStrList lSavedFile;
  lSavedFile.append(filename);
#ifdef WITH_CPP_REPARSE
  if (project)
#else
    if (project && getKWriteDocType(m_pCurEditDoc) == CPP_HEADER)
#endif
      m_pParent->refreshClassViewByFileList(&lSavedFile);
}

// closes all KWrite documents and their views 
// but not the document browser views
void DocViewMan::doFileCloseAll()
{
  debug("DocViewMan::doFileCloseAll !\n");

  QStrList handledNames;
  bool cont=true;

  QListIterator<QObject> itDoc(m_documentList);
  itDoc.toFirst();
  QObject* itDocObject;
  while ( (itDocObject = itDoc.current()) )
  {
    KWriteDoc* pDoc = dynamic_cast<KWriteDoc*> (itDocObject);
    if (pDoc
        && pDoc->isModified()
        && handledNames.contains(pDoc->fileName())<1)
    {
      SaveAllDialog::SaveAllResult result = m_pParent->doProjectSaveAllDialog(pDoc->fileName());
      switch (result)
      {
        case SaveAllDialog::Yes:
        { // Yes- only save the current file
          // save file as if Untitled and close file
          if(m_pParent->isUntitled(pDoc->fileName()))
          {
            m_pParent->switchToFile(pDoc->fileName());
            handledNames.append(pDoc->fileName());
            cont = m_pParent->fileSaveAs();
          }
          else
          { // Save file and close it
            m_pParent->switchToFile(pDoc->fileName());
            handledNames.append(pDoc->fileName());
            m_pParent->slotFileSave();
            cont = !currentEditView()->isModified(); //something went wrong
          }
          break;
        }

        case SaveAllDialog::No:
        {
          // No - no save but close
          handledNames.append(pDoc->fileName());
          pDoc->setModified(false);
          break;
        }

        case SaveAllDialog::SaveAll:
        {
          // Save all
          m_pParent->slotFileSaveAll();
          break;
        }

        case SaveAllDialog::Cancel:
        {
          cont=false;     // We are not going to continue
          break;
        }
      }
    }  // end file close/save

    if (cont)
      // We want to close the document and views
      closeKWriteDoc(pDoc);
    else
      break;    // user decided not to continue because of unsaved files

    // If the doc has been deleted then the iterator will be moved to the next
    // doc already - so we only have to move on if a doc wasn't deleted.
    if (itDocObject == itDoc.current())
      ++itDoc;
  } // end while-loop
}

bool DocViewMan::doProjectClose()
{
  debug("DocViewMan::doProjectClose !\n");

  QStrList handledNames;
  bool cont = true;

  // synchronizeDocAndInfo();

  QListIterator<QObject> itDoc(m_documentList);
  for (itDoc.toFirst(); cont && itDoc.current() != 0; ++itDoc) {
    KWriteDoc* pDoc = dynamic_cast<KWriteDoc*> (itDoc.current());
    if (pDoc
        && pDoc->isModified()
        && handledNames.contains(pDoc->fileName())<1) {

      SaveAllDialog::SaveAllResult result = m_pParent->doProjectSaveAllDialog(pDoc->fileName());

        // what to do
      if(result==SaveAllDialog::Yes) {  // Yes- only save the actual file
        // save file as if Untitled and close file
        if(m_pParent->isUntitled(pDoc->fileName())) {
          m_pParent->switchToFile(pDoc->fileName());
          handledNames.append(pDoc->fileName());
          cont = m_pParent->fileSaveAs();
          // start again... 'cause we deleted an entry
          itDoc.toFirst();
        }
        // Save file and close it
        else {
          m_pParent->switchToFile(pDoc->fileName());
          handledNames.append(pDoc->fileName());
          m_pParent->slotFileSave();
          cont = ! pDoc->isModified(); //something went wrong
        }
      }

      if(result==SaveAllDialog::No) {   // No - no save but close
        handledNames.append(pDoc->fileName());
        pDoc->setModified(false);
        // start again... 'cause we deleted an entry
        itDoc.toFirst();
      }

      if(result==SaveAllDialog::SaveAll) {  // Save all
        m_pParent->slotFileSaveAll();
        break;
      }

      if (result==SaveAllDialog::Cancel) { // Cancel
        cont=false;
        break;
      }
    }  // end actual file close
  } // end for-loop

  // check if something went wrong with saving
  if (cont) {
    cont = noDocModified();
  }

  return cont;
}

/** This closes all the edit and browser documents */
void DocViewMan::doCloseAllDocs()
{
  debug("DocViewMan::doCloseAllDocs !\n");

  QListIterator<QObject> itDoc(m_documentList);
  while (itDoc.current() != 0L) {
    KWriteDoc* pEditDoc = dynamic_cast<KWriteDoc*> (itDoc.current());
    if (pEditDoc) {
      closeKWriteDoc(pEditDoc);
    }
    else {
      CDocBrowser* pBrDoc = dynamic_cast<CDocBrowser*> (itDoc.current());
      if (pBrDoc) {
        closeCDocBrowser( pBrDoc);
      }
    }

    // go to the begin of the list
    // we cannot go through the list because closing the document changes the list itself
    // and the iterator doesn't work any more
    itDoc.toFirst();
  }
}

bool DocViewMan::saveFileFromTheCurrentEditWidget()
{
  debug("DocViewMan::saveFileFromTheCurrentEditWidget !\n");

  // Get the current file name
  QString filename = currentEditView()->getName();
  KWriteDoc* pCurEditDoc = currentEditDoc();

  if (pCurEditDoc == 0)
    return false; //oops :-(

  // Ask if we should save it
  QFileInfo file_info(filename);
  if(file_info.lastModified() != pCurEditDoc->getLastFileModifDate()) {
    if (KMessageBox::No == KMessageBox::questionYesNo(m_pParent,
                    i18n("The file %1 was modified outside\n this editor.Save anyway?").arg(filename),
                    i18n("File modified")))
      return false;
  }

  // Really save it
  currentEditView()->doSave();
  QFileInfo file_info2(filename);
  pCurEditDoc->setLastFileModifDate(file_info2.lastModified());

  return true;
}

void DocViewMan::saveModifiedFiles()
{
  debug("DocViewMan::saveModifiedFiles ! \n");

  QProgressBar* pProgressBar = m_pParent->getProgressBar();
  ASSERT(pProgressBar);

  pProgressBar->setTotalSteps(m_documentList.count());
  pProgressBar->show();

  QStrList iFileList(false);
  bool mod = false;

  QStrList handledNames;

  // check all edit_infos if they are modified outside; if yes, ask for saving
  QListIterator<QObject> itDoc(m_documentList);
  for (itDoc.toFirst(); itDoc.current() != 0; ++itDoc) {
    KWriteDoc* pDoc = dynamic_cast<KWriteDoc*> (itDoc.current());
    if (pDoc) {
      int i = 0;
      pProgressBar->setProgress(++i);

      kdDebug() << "checking: " << pDoc->fileName() << "\n";
      kdDebug() << " " << ((pDoc->isModified()) ?
      "modified" : "not modified") << "\n";

      if (!m_pParent->isUntitled(pDoc->fileName())
          && pDoc->isModified()
          && handledNames.contains(pDoc->fileName()) < 1) {
        int qYesNo = KMessageBox::Yes;
        handledNames.append(pDoc->fileName());

        kdDebug() << " file info" << "\n";

        QFileInfo file_info(pDoc->fileName());
        if (file_info.lastModified() != pDoc->getLastFileModifDate()) {
          qYesNo = KMessageBox::questionYesNo(m_pParent,
                                              i18n("The file %1 was modified outside\n"
                                                   "this editor. Save anyway?")
                                              .arg(pDoc->fileName()),
                                              i18n("File modified"));
        }

        kdDebug() << " KMessageBox::Yes" << "\n";

        if (qYesNo == KMessageBox::Yes) {
          kdDebug() << " create file_info" << "\n";
          QFileInfo file_info(pDoc->fileName());
          bool isModified;

          kdDebug() << " use blind widget " << "\n";

          KWriteDoc* pNewDoc = new KWriteDoc(&m_highlightManager);
          CEditWidget* pBlindWidget = new CEditWidget(0L, 0, pDoc);

          pBlindWidget->setName(pDoc->fileName());
          pBlindWidget->setText(pDoc->text());
          pBlindWidget->toggleModified(true);

          kdDebug() << "doSave" << "\n";
          pBlindWidget->doSave();
          isModified = pBlindWidget->isModified();

          delete pBlindWidget;
          delete pNewDoc;

          kdDebug() << "doing save " << ((!isModified) ? "success" : "failed") << "\n";

          pDoc->setModified(isModified);

          if (!isModified) {
#ifdef WITH_CPP_REPARSE
            mod = true;
#else
            mod |= (pDoc->fileName().right(2)==".h" || pDoc->fileName().right(4)==".hxx");
#endif
            iFileList.append(pDoc->fileName());
            pDoc->setLastFileModifDate(file_info.lastModified());
          }
        }
      }
    }
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
  debug("DocViewMan::reloadModifiedFiles !\n");

  QListIterator<QObject> itDoc(m_documentList);
  for (itDoc.toFirst(); itDoc.current() != 0; ++itDoc) {
    KWriteDoc* pDoc = dynamic_cast<KWriteDoc*> (itDoc.current());
    if (pDoc) {
      QFileInfo file_info(pDoc->fileName());
    	
      // Reload only changed files
      if(pDoc->getLastFileModifDate() != file_info.lastModified()) {
        // Force reload, no modified on disc messagebox
        m_pParent->switchToFile(pDoc->fileName(),-1,-1,true,false);
      }
    }
  }
}


QList<KWriteDoc> DocViewMan::getKWriteDocList() const
{
  QListIterator<QObject> itDoc(m_documentList);
  QList<KWriteDoc> resultList;

  for (; itDoc.current() != 0; ++itDoc) {
    KWriteDoc* doc = dynamic_cast<KWriteDoc*> (itDoc.current());
    if (doc) {
      resultList.append(doc);
    }
  }
  return resultList;
}

QList<CDocBrowser> DocViewMan::getDocBrowserList() const
{
  QListIterator<QObject> itDoc(m_documentList);
  QList<CDocBrowser> resultList;

  for (; itDoc.current() != 0; ++itDoc) {
    CDocBrowser* doc = dynamic_cast<CDocBrowser*> (itDoc.current());
    if (doc) {
      resultList.append(doc);
    }
  }
  return resultList;
}

QString DocViewMan::docName(QObject* pDoc) const
{
  debug("DocViewMan::docName !\n");

  KWriteDoc* kwDoc = (dynamic_cast<KWriteDoc*> (pDoc));
  if (kwDoc) {
    return (kwDoc->fileName());
  }

  CDocBrowser* brDoc = (dynamic_cast<CDocBrowser*> (pDoc));
  if (brDoc) {
    return (QString(brDoc->name()));
  }

  return QString("");
}


#include "docviewman.moc"
