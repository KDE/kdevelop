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
      pCurEditWidget->setFocus();
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
    pCurEditWidget->parentWidget()->setFocus();
  }
  else {
    debug(" document type found !\n");

    KWriteDoc* pDoc = findKWriteDoc(filename);
    pCurEditWidget = getFirstEditView(pDoc);

    debug(" focus view !\n");

    if (pCurEditWidget)
      pCurEditWidget->parentWidget()->setFocus();

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

  debug(" set focus on view !\n");
  pCurEditWidget->setFocus();
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
// find if there is another KWriteDoc in the doc list 
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
    delete pMDICover;
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
// find if there is another CDocBrowser in the doc list 
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
  debug("DocViewMan::closeCDocBrowser : %d !\n", pDoc);

  if(pDoc) {
    debug("getting pView !\n");
    KHTMLView* pView = pDoc->view();
    debug("pView : %d !\n", pView);
    if(pView) {
      debug("getting pMDICover !\n");
      QextMdiChildView* pMDICover = (QextMdiChildView*) pView->parentWidget();
      if(pMDICover) {
        debug("pMDICover : %d !\n", pMDICover);
        debug("hiding pMDICover !\n");
        pMDICover->hide();
        debug("reparent pView !\n");
        pView->reparent(0L, 0, QPoint(0,0));
        QApplication::sendPostedEvents();
        m_pParent->removeWindowFromMdi( pMDICover);
        delete pMDICover;
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
  delete pMDICover;

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
  delete pMDICover;

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

  debug("pView : %d !", pView);
  
  // emit the got focus signal 
  // (connected to CKDevelop but could also be caught by other ones)
  emit sig_viewGotFocus(pView);

  debug("emit the got focus signal !\n");
}

//-----------------------------------------------------------------------------
// returns the number of documents for the asked document type
//-----------------------------------------------------------------------------
int DocViewMan::docCount() const
{
  debug("DocViewMan::docCount !\n");

  return m_documentList.count();
}

//-----------------------------------------------------------------------------
// retrieves the document found by its filename
//-----------------------------------------------------------------------------
/*
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
*/

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

//------------------------------------------------------------------------------
// shows the desired bookmark (eventually, switches to file and activates it)
//------------------------------------------------------------------------------
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
/*
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
*/

/*
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
*/


/*
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
*/

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
/*
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
*/

bool DocViewMan::noInfoModified()
{
  debug("DocViewMan::noInfoModified !\n");

  QListIterator<QObject> itDoc(m_documentList);
  for (itDoc.toFirst(); itDoc.current() != 0; ++itDoc) {
    KWriteDoc* pDoc = dynamic_cast<KWriteDoc*> (itDoc.current());
    if(pDoc && pDoc->isModified()) {
      return false;
    }
  }
  return true;
}

/*
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
*/

/*
void DocViewMan::appendInfo(TEditInfo* info)
{
  edit_infos.append(info);
}
*/

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

// closes all KWrite documents and their views but not the document browser views
void DocViewMan::doFileCloseAll()
{
  debug("DocViewMan::doFileCloseAll !\n");

  // Get the project name
  QString prjName = m_pParent->getProjectName();
  if (prjName != QString(""))
    prjName = "\n" + prjName + "\n\n";

  QStrList handledNames;
  bool cont=true;

  // synchronizeDocAndInfo();

  QListIterator<QObject> itDoc(m_documentList);
  for (itDoc.toFirst(); cont && itDoc.current() != 0; ++itDoc) {
    KWriteDoc* pDoc = dynamic_cast<KWriteDoc*> (itDoc.current());
    if (pDoc
        && pDoc->isModified()
        && handledNames.contains(pDoc->fileName())<1) {

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
      int result = KMessageBox::warningYesNoCancel( m_pParent,
                                                    i18n("The project %1\n"
                                                    "contains changed files. Save modified file\n\n%2 ?\n\n")
                                                    .arg(prjName)
                                                    .arg(pDoc->fileName()),
                                                    i18n("Save changed files ?"));

      // create the save project messagebox

      // what to do
      if(result==KMessageBox::Yes) { // Yes- only save the current file
        // save file as if Untitled and close file
        if(m_pParent->isUntitled(pDoc->fileName())) {
          m_pParent->switchToFile(pDoc->fileName());
          handledNames.append(pDoc->fileName());
          cont = m_pParent->fileSaveAs();
          itDoc.toFirst(); // start again... 'cause we deleted an entry
        }
        else { // Save file and close it
          m_pParent->switchToFile(pDoc->fileName());
          handledNames.append(pDoc->fileName());
          m_pParent->slotFileSave();
          cont = !currentEditView()->isModified(); //something went wrong
        }
      }
      else if(result==KMessageBox::No) { // No - no save but close
        handledNames.append(pDoc->fileName());
        pDoc->setModified(false);
        itDoc.toFirst(); // start again... 'cause we deleted an entry
      }
         	
//      if(result==3) { // Save all
//        slotFileSaveAll();
//        break;
//      }
      else if(result==KMessageBox::Cancel) { // Cancel
        cont=false;
        break;
      }
    }  // end actual file close

    if (cont) { // close the document and its views
      closeKWriteDoc(pDoc);
    }
  } // end for-loop
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
    cont = noInfoModified();
  }

  return cont;
}

/** This closes all the edit documents */
void DocViewMan::doCloseAllDocs()
{
  debug("DocViewMan::doCloseAllDocs !\n");

  QListIterator<QObject> itDoc(m_documentList);
  for (itDoc.toFirst(); itDoc.current() != 0; ++itDoc) {
    KWriteDoc* pDoc = dynamic_cast<KWriteDoc*> (itDoc.current());
    if(pDoc)
      closeKWriteDoc(pDoc);
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

QObject* DocViewMan::getDocFromFilename(const QString& strFileName) const
{
  debug("DocViewMan::getDocFromFilename !\n");

  QListIterator<QObject> itDoc(m_documentList);
  QList<KWriteDoc> resultList;

  for (; itDoc.current() != 0; ++itDoc) {
    QObject* doc = itDoc.current();
    if (doc  && (docName(doc) == strFileName)) {
      return doc;
    }
  }

  return 0;
}

KWriteDoc* DocViewMan::getKWDocFromFilename(const QString& strFileName) const
{
  debug("DocViewMan::getKWDocFromFilename !\n");

  return (dynamic_cast<KWriteDoc*> (getDocFromFilename(strFileName)));
}


#include "docviewman.moc"
