/***************************************************************************
  docviewman.cpp - MDI manager for
                   document classes of KDevelop (Kate::Documents, CDocBrowser)
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
#include <qstrlist.h>

#include <kmessagebox.h>
#include <klocale.h>
#include <kdebug.h>
#include <kstddirs.h>
#include <kpopupmenu.h>

#include <kate/document.h>
#include <kate/view.h>
#include <kparts/factory.h>

#include "ckdevelop.h"
#include "cdocbrowser.h"
#include "doctreeview.h"
#include "khtmlview.h"
#include "docviewman.h"
#include "./dbg/brkptmanager.h"
#include "./dbg/vartree.h"
#include "./ctags/ctagsdialog_impl.h"
#include "classstore.h"
#include "ckdevaccel.h"
#include "kdevcodecompletion.h"
#include "kdevcodetemplate.h"
#include "partcontroller.h"

DocViewMan::DocViewMan( CKDevelop* parent )
: QObject( parent)
  ,m_pParent(parent)
  ,m_pDocBookmarksMenu(0L)
  ,m_pCurEditDoc(0L)
  ,m_pCurEditView(0L)
  ,m_pCurBrowserDoc(0L)
  ,m_pCurBrowserView(0L)
  ,m_curIsBrowser(false)
  ,m_bCloseActionPending(false)
  ,m_pKateFactory(0L)
{
  m_MDICoverList.setAutoDelete(true);
  m_docBookmarksList.setAutoDelete(true);
  m_docBookmarksTitleList.setAutoDelete(true);

  m_pKateFactory = static_cast<KParts::Factory*>
                   (KLibLoader::self()->factory("libkatepart"));
  if (!m_pKateFactory) {
    KMessageBox::sorry(0L, i18n("This version of KDevelop uses Kate as its internal editor, "
                                "but Kate could not be found on your system! Please check "
                                "that kdebase and/or Kate are installed correctly." ) );
    ::exit(0);
  }

  connect( this, SIGNAL(sig_viewActivated(QWidget*)),
           m_pParent, SLOT(slotViewSelected(QWidget*)) );
  connect( this, SIGNAL(sig_newStatus(const QString&)),
           m_pParent, SLOT(slotStatusMsg(const QString&)) );
  connect( m_pParent, SIGNAL(lastChildViewClosed()),
           this, SLOT(slotResetMainFrmCaption()) );

  m_pCodeCompletion = new KDevCodeCompletion( this );
  m_pCodeTemplate = KDevCodeTemplate::self();
  m_pCodeTemplate->load();

  /* TEST
  m_pCodeTemplate->addTemplate( "ifb", "if with block", "if( | ){\n}" );
  m_pCodeTemplate->addTemplate( "cl", "class", "class " );
  m_pCodeTemplate->addTemplate( "pu", "public", "public " );
  m_pCodeTemplate->addTemplate( "pr", "private", "private " );
  m_pCodeTemplate->addTemplate( "pro", "protected", "protected " );
  */
}

DocViewMan::~DocViewMan()
{
    m_pCodeTemplate->save();
}

void DocViewMan::doSelectURL(const QString& url)
{
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
    pBrowserView = createBrowserView(pDoc, true);
  }

  if (pBrowserView && pBrowserView->parentWidget()) {
    pBrowserView->parentWidget()->setFocus();
  }
}

void DocViewMan::doSwitchToFile(QString filename, int line, int col, bool bForceReload, bool bShowModifiedBox)
{
  Kate::View* pEditWidget = 0L;
  Kate::Document* pEditDoc = findKWriteDoc(filename);
  Kate::View* pCurEditWidget = currentEditView();
  Kate::Document* pCurEditDoc = currentEditDoc();
  QString editWidgetName;

  if (pCurEditWidget) {
      if (pCurEditWidget->getDoc() == pEditDoc) {
          pEditWidget = pCurEditWidget;
      }
      else {
          if (pEditDoc) {
              pEditWidget = getFirstEditView(pEditDoc);
          }
      }
      editWidgetName = pCurEditWidget->getDoc()->docName();
  }
//  CEditWidget* pCurEditWidget = currentEditView();


  // Make sure that we found the file in the editor_widget in our list
  if (pEditDoc) {
    // handle file if it was modified on disk by another editor/cvs
    pCurEditDoc->isModOnHD(bShowModifiedBox);
// lets leave this to kate. (rokrau 6/25/01)
//    QFileInfo file_info(editWidgetName);
//    if ((file_info.lastModified() != pCurEditDoc->getLastFileModifDate()) && bShowModifiedBox) {
//      if(KMessageBox::questionYesNo(m_pParent,
//                                    i18n("The file %1 was modified outside this editor.\n"
//                                         "Open the file from disk and delete the current Buffer?")
//                                    .arg(editWidgetName),
//                                    i18n("File modified"))==KMessageBox::Yes) {
//        bForceReload = true;
//        pCurEditDoc->setLastFileModifDate(file_info.lastModified());
//      }
//    }

//    if (!bShowModifiedBox) {
//      pCurEditDoc->setLastFileModifDate(file_info.lastModified());
//    }

    if (!bForceReload && filename == editWidgetName) {
      if (pCurEditWidget && (line != -1))
        pCurEditWidget->setCursorPosition(line, col);

//    if (!bShowModifiedBox) {
//      pEditDoc->setLastFileModifDate(file_info.lastModified());
//    }
    }

//    if (!bShowModifiedBox) {
//      pEditDoc->setLastFileModifDate(file_info.lastModified());
//    }

    if (!bForceReload && pEditWidget) {
      if (line != -1)
        pEditWidget->setCursorPosition(line, col);

      //    cerr << endl <<endl << "Filename:" << filename
      // << "EDITNAME:" << pCurEditWidget->getName() <<"no action---:" << endl;
      QextMdiChildView* pMDICover = (QextMdiChildView*) pEditWidget->parentWidget();
      if (pMDICover) {
        pMDICover->activate();
      }
      return;
    }
  }

  // See if we already have the file wanted.
  Kate::Document* pDoc = findKWriteDoc(filename);

  // bool found = (pDoc != 0);

  // Not found or needing a reload causes the file to be read from disk
  if ((!pEditDoc) || bForceReload) {
    QFileInfo fileinfo(filename);
    if (!pDoc) {
      pDoc = createKWriteDoc(filename);
      if (pDoc) {
        //(kate takes care of this now, rokrau 06/25/01) Set the last modify date
        //pDoc->setLastFileModifDate(fileinfo.lastModified());
        pEditWidget = createEditView(pDoc, true);
      }
    }
    else {
      // a view for this doc exists, already;
      // use the first view we found of this doc to show the text
      pCurEditWidget = getFirstEditView(pDoc);
      // awkward but this reloads the file;
      pDoc->openURL(pDoc->url());
    }
    // not sure why this had to be reloaded
    // loadKWriteDoc(pDoc , filename, 1);
  }
  else {
    Kate::Document* pDoc = findKWriteDoc(filename);
    pCurEditWidget = getFirstEditView(pDoc);
  }

  if (!pEditWidget) {
    return;
  }

  // If the caller wanted to be positioned at a particular place in the file
  // then they have supplied the line and col. Otherwise we use the
  // current info values (0 if new) for the placement.
  if (line != -1)
    pEditWidget->setCursorPosition(line, col);

  pEditWidget->setName(filename);

  QextMdiChildView* pMDICover = (QextMdiChildView*) pEditWidget->parentWidget();
  pMDICover->activate();
  pEditWidget->setFocus();

  qDebug("DocViewMan::doSwitchToFile: cursor-pos(line: %d col: %d)", line, col);
}


void DocViewMan::doOptionsEditor()
{
  if(currentEditView())
  {
    //currentEditView()->optDlg();
    currentEditDoc()->configDialog();
    doTakeOverOfEditorOptions();
  }
  else {
// falk: NO!
//    KWriteDoc dummyDoc(&m_highlightManager, "/tmp/dummy");
//    KConfig* pConfig = m_pParent->getConfig();
//    if (pConfig) {
//      pConfig->setGroup("KWrite Options");
//      dummyDoc.readConfig(pConfig);
//    }
//    CEditWidget dummyView(0L, "dummyview", &dummyDoc);
//    if (pConfig) {
//      pConfig->setGroup("KWrite Options");
//      dummyView.readConfig(pConfig);
//    }
//    dummyView.colDlg();
//    doTakeOverOfEditorOptions(&dummyView);
  }
}

// obsolete since kate's config dialog takes care of this (rokrau 6/25/01)
//void DocViewMan::doOptionsEditorColors()
//{
//  if(currentEditView())
//  {
//    currentEditView()->colDlg();
//    doTakeOverOfEditorOptions();
//  }
//}


//void DocViewMan::doOptionsSyntaxHighlightingDefaults()
//{
//  if(currentEditView())
//  {
//    currentEditView()->hlDef();
//    doTakeOverOfEditorOptions();
//  }
//}

//void DocViewMan::doOptionsSyntaxHighlighting()
//{
//  if(currentEditView())
//  {
//    currentEditView()->hlDlg();
//    doTakeOverOfEditorOptions();
//  }
//}

/** shared helper function for the 4 slots
  * doOptionsEditor, doOptionsEditorColors,
  * doOptionsSyntaxHighlightingDefaults and doOptionsSyntaxHighlighting
  */
void DocViewMan::doTakeOverOfEditorOptions(Kate::View* pView)
{
  kdDebug() << "in DocViewMan::doTakeOverOfEditorOptions(), not yet implemented,"
            << "due to changes in the editor interface\n";
  return;
//  KConfig* config = m_pParent->getConfig();
//  if (config) {
//    config->setGroup("KWrite Options");
//    currentEditView()->writeConfig(config);
//    currentEditView()->doc()->writeConfig(config);
//
//    QListIterator<QObject> itDoc(m_documentList);
//    for (itDoc.toFirst(); itDoc.current() != 0; ++itDoc) {
//      Kate::Document* pDoc = dynamic_cast<KWriteDoc*> (itDoc.current());
//      if (pDoc) {
//        CEditWidget* pCurEW = getFirstEditView(pDoc);
//        pCurEW->copySettings(currentEditView());
//        config->setGroup("KWrite Options");
//        pCurEW->readConfig(config);
//        pDoc->readConfig(config);
//      }
//    }
//  }
}

/** */
void DocViewMan::doCopy()
{
  if (currentEditView()) {
    if (curDocIsBrowser())
      currentBrowserDoc()->slotCopyText();
    else
      currentEditView()->copy();
  }
}

/** */
void DocViewMan::doSearch()
{
  if (curDocIsBrowser()) {
    if (currentBrowserDoc()) {
      KActionCollection* pAC = currentBrowserDoc()->actionCollection();
      if (pAC) {
        KAction* pFindAction = pAC->action("find");
        if (pFindAction) {
          pFindAction->activate();
        }
      }
    }
  }
  else {
    if (currentEditView()) {
      currentEditView()->find();
    }
  }
}

/** */
void DocViewMan::doRepeatSearch(QString &/*search_text*/, int back)
{
  if (currentEditView()) {
      currentEditView()->findAgain(back==1);
  }
}

/** */
void DocViewMan::doSearchText(QString &text)
{
  if (currentEditView()) {
    if (curDocIsBrowser())
      text = currentBrowserDoc()->selectedText();
    else {
      text = currentEditDoc()->selection();
      if(text == "") {
        text = currentEditView()->currentWord();
      }
    }
  }
}

/** */
void DocViewMan::doClearBookmarks()
{
//  QListIterator<QObject> itDoc(m_documentList);
//  for (itDoc.toFirst(); itDoc.current() != 0; ++itDoc) {
//    Kate::Document* pDoc = dynamic_cast<Kate::Document*> (itDoc.current());
//    if (pDoc) {
//      pDoc->clearBookmarks();
//    }
//  }
// bookmarks are now defined on the Kate::View level
// we'll get to this later then
}

void DocViewMan::doCreateNewView()
{
  QWidget* pNewView = 0L;

  if (curDocIsBrowser()) {
    CDocBrowser* pDoc = createCDocBrowser(DocTreeKDevelopBook::locatehtml("about/intro.html"));
    pNewView = createBrowserView(pDoc, true);
  }
  else {
    pNewView = createEditView(currentEditDoc(), true);
  }

  // raise and activate
  if (pNewView)
    pNewView->parentWidget()->setFocus();
}

/** */
bool DocViewMan::curDocIsHeaderFile()
{
  return (!curDocIsBrowser()
    && m_pCurEditDoc
    && (getKWriteDocType(m_pCurEditDoc) == CPP_HEADER));
}

/** */
bool DocViewMan::curDocIsCppFile()
{
  return (!curDocIsBrowser()
      && m_pCurEditDoc
      && ((getKWriteDocType(m_pCurEditDoc)==CPP_SOURCE)||
          (getKWriteDocType(m_pCurEditDoc)==FTN_SOURCE)));
}


ProjectFileType DocViewMan::getKWriteDocType(Kate::Document* pDoc)
{
  return CProject::getType(pDoc->docName());
}

Kate::Document* DocViewMan::createKWriteDoc(const QString& strFileName)
{
  // ok here is the document, now how do i get a file into it?
  Kate::Document* pDoc = static_cast<Kate::Document *>
    (m_pKateFactory->createPart(0,
                                QString::null,
                                this,
                                QString::null,
                                "KTextEditor::Document"));
  //Kate::Document* pDoc = new Kate::Document(&m_highlightManager, strFileName);
  if (!pDoc)
    return 0L;
  // openURL should do the trick since it calls openFile... so well here we go
  pDoc->openURL(strFileName);
// No, we dont need to do this anymore (rokrau 06/25/01)
//  // Check if we must set the last modif date
//  QFileInfo file_info(strFileName);
//  if(file_info.exists()) {
//      pDoc->setLastFileModifDate(file_info.lastModified());
//  }

  // Add the new doc to the list
  m_documentList.append(pDoc);

/*
 * we will probably need to read the current configuration here,
 * but leave the internals to kate also (rokrau 6/25/01)
 */
//  KConfig* config = m_pParent->getConfig();
//  if(config) {
//    config->setGroup("KWrite Options");
//    pDoc->readConfig(config);
//  }

  pDoc->setDocName(strFileName);

  PartController::getInstance()->addPart( pDoc );

  // Return the new document
  return pDoc;
}

CDocBrowser* DocViewMan::createCDocBrowser(const QString& url)
{
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

  // Return the new document
  return pDocBr;
}

// no longer called from anywhere
//-----------------------------------------------------------------------------
// load edit document from file
//-----------------------------------------------------------------------------
//void DocViewMan::loadKWriteDoc(Kate::Document* pDoc,
//                               const QString& strFileName,
//                               int /*mode*/)
//{
//  if(QFile::exists(strFileName)) {
//    QFile f(strFileName);
//    if (f.open(IO_ReadOnly)) {
//      pDoc->loadFile(f);
//      f.close();
//    }
//  }
//}

// not called anymore (rokrau 6/25/01)
//-----------------------------------------------------------------------------
// save document to file
//-----------------------------------------------------------------------------
//bool DocViewMan::saveKWriteDoc(Kate::Document* pDoc, const QString& strFileName)
//{
//  QFileInfo info(strFileName);
//  if(info.exists() && !info.isWritable()) {
//    KMessageBox::sorry(0L, i18n("You do not have write permission to this file:\n" + strFileName));
//    return false;
//  }
//
//  QFile f(strFileName);
//  if (f.open(IO_WriteOnly | IO_Truncate)) {
//    pDoc->writeFile(f);
//    pDoc->updateViews();
//    f.close();
//    return true;//kWriteDoc->setFileName(name);
//  }
//  KMessageBox::sorry(0L,  i18n("An Error occured while trying to save this Document"));
//  return false;
//}

//-----------------------------------------------------------------------------
// Find if there is another Kate::Document in the doc list
//-----------------------------------------------------------------------------
Kate::Document* DocViewMan::findKWriteDoc()
{
  QListIterator<QObject> itDoc(m_documentList);
  for (itDoc.toFirst(); itDoc.current() != 0; ++itDoc) {
    Kate::Document* pDoc = dynamic_cast<Kate::Document*> (itDoc.current());
    if(pDoc) {
      return pDoc;
    }
  }
  return 0L;
}

//-----------------------------------------------------------------------------
void DocViewMan::slotRemoveFileFromEditlist(const QString &absFilename)
{
  Kate::Document* pDoc = findKWriteDoc( absFilename);
  closeKWriteDoc( pDoc);
  m_pParent->setMainCaption();
}


//-----------------------------------------------------------------------------
// close an edit document, causes all views to be closed
//-----------------------------------------------------------------------------
void DocViewMan::closeKWriteDoc(Kate::Document* pDoc)
{
  if (!pDoc) return;

//  QList<Kate::View> views = pDoc->viewList();
//  QListIterator<Kate::View>  itViews(views);
  QPtrList<KTextEditor::View> viewList = pDoc->views();
  for (Kate::View* itView = dynamic_cast<Kate::View*>(viewList.first());
                   itView != 0L ;
                   itView = dynamic_cast<Kate::View*>(viewList.next())) {
//    CEditWidget* pView = (CEditWidget*) itView->parentWidget();
    Kate::View* pView = (Kate::View*) itView->parentWidget();
    if (!pView) continue;
    // remove the view from MDI and delete the view
    QextMdiChildView* pMDICover = (QextMdiChildView*) pView->parentWidget();
    disconnect(pMDICover, SIGNAL(activated(QextMdiChildView*)),
               this, SLOT(slot_viewActivated(QextMdiChildView*)));
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
  Kate::Document* pNewDoc = findKWriteDoc();

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
  if(pDoc) {
    KHTMLView* pView = pDoc->view();
    if(pView) {
      QextMdiChildView* pMDICover = (QextMdiChildView*) pView->parentWidget();
      if(pMDICover) {
        pMDICover->hide();
        pView->reparent(0L, 0, QPoint(0,0));
        QApplication::sendPostedEvents();
        m_pParent->removeWindowFromMdi( pMDICover);
        m_MDICoverList.remove( pMDICover);
      }
    }
    // Remove the document from the list
    m_documentList.removeRef(pDoc);
    // now finally, delete the document (which inclusively deletes the view)
    delete pDoc;
  }

  CDocBrowser* pNewDoc = findCDocBrowser();
  if (pNewDoc == 0) {
    m_pCurBrowserDoc = 0L;
    m_pCurBrowserView = 0L;
  }

  // emit an according signal if we closed the last doc
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
// retrieve the document pointer and make sure it is a Kate::Document
//-----------------------------------------------------------------------------
/*
Kate::Document* DocViewMan::kwDocPointer(int docId) const
{
  return (dynamic_cast<Kate::Document*> (docPointer(docId)));
}
*/

//-----------------------------------------------------------------------------
// cover a newly created view with a QextMDI childview
//-----------------------------------------------------------------------------
void DocViewMan::addQExtMDIFrame(QWidget* pNewView, bool bShow, const QPixmap& icon)
{
  if (!pNewView) return;  // failed, could not create view

  // cover it by a QextMDI childview and add that MDI system
  QextMdiChildView* pMDICover = new QextMdiChildView( pNewView->caption());
  pMDICover->setIcon(icon);
//  pMDICover->setIcon(kapp->miniIcon());
  m_MDICoverList.append( pMDICover);
  QBoxLayout* pLayout = new QHBoxLayout( pMDICover, 0, -1, "layout");
  pNewView->reparent( pMDICover, QPoint(0,0));
//  QApplication::sendPostedEvents();
  pLayout->addWidget( pNewView);
  pMDICover->setName( pNewView->name());
  // captions
  QString shortName = pNewView->caption();
  int length = shortName.length();
  shortName = shortName.right(length - (shortName.findRev('/') +1));
  pMDICover->setTabCaption( shortName);
  pMDICover->setCaption(pNewView->caption());
  connect(pMDICover, SIGNAL(activated(QextMdiChildView*)),
          this, SLOT(slot_viewActivated(QextMdiChildView*)));

  // fake a viewActivated to update the currentEditView/currentBrowserView pointers _before_ adding to MDI control
  slot_viewActivated( pMDICover);

  // take it under MDI mainframe control (note: this triggers also a setFocus())
  int flags;
  if (bShow) {
      flags = QextMdi::StandardAdd;
  }
  else {
    flags = QextMdi::Hide;
  }
  // set the accelerators for Toplevel MDI mode (each toplevel window needs its own accels
  connect( m_pParent, SIGNAL(childViewIsDetachedNow(QWidget*)), this, SLOT(initKeyAccel(QWidget*)) );

  m_pParent->addWindow( pMDICover, flags);
  // correct the default settings of QextMDI ('cause we haven't a tab order for subwidget focuses)
  pMDICover->setFirstFocusableChildWidget(0L);
  pMDICover->setLastFocusableChildWidget(0L);
}

//-----------------------------------------------------------------------------
// create a new view for an edit document
//-----------------------------------------------------------------------------
//CEditWidget* DocViewMan::createEditView(Kate::Document* pDoc, bool bShow)
Kate::View* DocViewMan::createEditView(Kate::Document* pDoc, bool bShow)
{
  // create the view and add to MDI
  //CEditWidget* pEW = new CEditWidget(0L, "autocreatedview", pDoc);
  Kate::View *pEW = static_cast<Kate::View *>(pDoc->createView(m_pParent, QString::null));
  if(!pEW) return 0L;
  pEW->setCaption(pDoc->docName());

// this is going to be taken care of by the new context menu (rokrau 6/28/01)
//  // connect tag related functionality with searchTagsDialogImpl
//  searchTagsDialogImpl* ctagsDlg = m_pParent->getCTagsDialog();
//  connect( pEW, SIGNAL(tagSwitchTo()), m_pParent, SLOT(slotTagSwitchTo()));
//  connect( pEW, SIGNAL(tagOpenFile(QString)), ctagsDlg, SLOT(slotGotoFile(QString)));
//  connect( pEW, SIGNAL(tagDefinition(QString)), ctagsDlg, SLOT(slotGotoDefinition(QString)));
//  connect( pEW, SIGNAL(tagDeclaration(QString)), ctagsDlg, SLOT(slotGotoDeclaration(QString)));

// this is going to be taken care of by the new context menu (rokrau 6/28/01)
//  //connect the editor lookup function with slotHelpSText
//  connect( pEW, SIGNAL(manpage(QString)),m_pParent, SLOT(slotHelpManpage(QString)));
//  connect( pEW, SIGNAL(lookUp(QString)),m_pParent, SLOT(slotHelpSearchText(QString)));
//  connect( pEW, SIGNAL(newCurPos()), m_pParent, SLOT(slotNewLineColumn()));
//  connect( pEW, SIGNAL(newStatus()),m_pParent, SLOT(slotNewStatus()));
//  connect( pEW, SIGNAL(clipboardStatus(KWriteView *, bool)), m_pParent, SLOT(slotClipboardChanged(KWriteView *, bool)));
//  connect( pEW, SIGNAL(newUndo()),m_pParent, SLOT(slotNewUndo()));
  // slot doesn't exist anymore (rokrau 6/11/01)
  //connect( pEW, SIGNAL(bufferMenu(const QPoint&)),m_pParent, SLOT(slotBufferMenu(const QPoint&)));
// this is going to be taken care of by the new context menu (rokrau 6/28/01)
//  connect( pEW, SIGNAL(grepText(QString)), m_pParent, SLOT(slotEditSearchInFiles(QString)));
//  connect( pEW->popup(), SIGNAL(highlighted(int)), m_pParent, SLOT(statusCallback(int)));
//  // Connect the breakpoint manager to monitor the bp setting - even when the debugging isn't running
//  connect( pEW, SIGNAL(editBreakpoint(const QString&,int)), m_pParent->getBrkptManager(), SLOT(slotEditBreakpoint(const QString&,int)));
//  connect( pEW, SIGNAL(toggleBPEnabled(const QString&,int)), m_pParent->getBrkptManager(), SLOT(slotToggleBPEnabled(const QString&,int)));
//  connect( pEW, SIGNAL(toggleBreakpoint(const QString&,int)), m_pParent->getBrkptManager(), SLOT(slotToggleStdBreakpoint(const QString&,int)));
//  connect( pEW, SIGNAL(clearAllBreakpoints()), m_pParent->getBrkptManager(),   SLOT(slotClearAllBreakpoints()));
//  connect( pEW, SIGNAL(runToCursor(const QString&, int)), m_pParent, SLOT(slotDebugRunUntil(const QString&, int)));

// this is going to be taken care of by the new context menu (rokrau 6/28/01)
//  // connect adding watch variable from the rmb in the editors
//  connect( pEW, SIGNAL(addWatchVariable(const QString&)), m_pParent->getVarViewer()->varTree(), SLOT(slotAddWatchVariable(const QString&)));

  // connect adding watch variable from the rmb in the editors
  if (m_pParent->getVarViewer()) {
    connect( pEW, SIGNAL(addWatchVariable(const QString&)), m_pParent->getVarViewer()->varTree(), SLOT(slotAddWatchVariable(const QString&)));
  }

  if (getKWriteDocType(pDoc)==CPP_SOURCE||getKWriteDocType(pDoc)==FTN_SOURCE) {
    // gotta find this signal (rokrau 6/28/01)
    //connect( pEW, SIGNAL(markStatus(KWriteView *, bool)), m_pParent, SLOT(slotCPPMarkStatus(KWriteView *, bool)));
    QIconSet iconSet(SmallIcon("source_cpp"));
    // Cover it by a QextMDI childview and add that MDI system
    addQExtMDIFrame(pEW, bShow, iconSet.pixmap());
  } else if (getKWriteDocType(pDoc)==CPP_HEADER) {
    connect( pEW, SIGNAL(markStatus(Kate::View *, bool)), m_pParent, SLOT(slotHEADERMarkStatus(Kate::View *, bool)));
    QIconSet iconSet(SmallIcon("source_h"));
    // Cover it by a QextMDI childview and add that MDI system
    addQExtMDIFrame(pEW, bShow, iconSet.pixmap());
  } else {
    connect( pEW, SIGNAL(markStatus(Kate::View *, bool)), m_pParent, SLOT(slotMarkStatus(Kate::View *, bool)) );
    QIconSet iconSet(SmallIcon("txt"));
    addQExtMDIFrame(pEW, bShow, iconSet.pixmap());
  }

  // some additional settings
  pEW->setFocusPolicy(QWidget::StrongFocus);
  pEW->setFont(KGlobalSettings::fixedFont());

  kdDebug() << "in DocViewMan::createEditView(), not yet implemented,"
            << "due to changes in the editor interface\n";
//  KConfig* config = m_pParent->getConfig();
//  if(config) {
//    config->setGroup("KWrite Options");
//    pEW->readConfig(config);
//  }

  return pEW;
}

//-----------------------------------------------------------------------------
// create a new view for a browser document
//-----------------------------------------------------------------------------
KHTMLView* DocViewMan::createBrowserView(CDocBrowser* pDoc, bool bShow)
{
  KHTMLView* pNewView = pDoc->view();
  pNewView->setCaption( pDoc->currentTitle());
  // add "what's this" entry
  m_pParent->getWhatsThis()->add(pNewView, i18n("Documentation Browser\n\n"
            "The documentation browser window shows the online-"
            "documentation provided with kdevelop as well as "
            "library class documentation created. Use the documentation "
            "tree to switch between various parts of the documentation."));

  // Cover it by a QextMDI childview and add it to the MDI system
  addQExtMDIFrame(pNewView, bShow, SmallIcon("contents"));

  return pNewView;
}

//-----------------------------------------------------------------------------
// close a view
//-----------------------------------------------------------------------------
bool DocViewMan::closeView(QWidget* pWnd)
{
  // get the embedded view
  if (!pWnd) return true; // I ran into this case also, yuck!
  QObjectList* pL = (QObjectList*) (pWnd->children());
  if (!pL) return true; // I ran into this case once, no idea why this happened (rokrau 06/02/01)
  QWidget* pView = 0L;
  QObject* pChild;
  // the first object we'll find is the layout,
  // the second test will be successful for the one and only embedded widget
  for ( pChild = pL->first(); pChild && !pView; pChild = pL->next()) {
    if (pChild->isWidgetType()) {
      pView = (QWidget*) pChild;
      if (Kate::View* pEditView = dynamic_cast<Kate::View*> (pView)) {
        if (checkAndSaveFileOfCurrentEditView(true) != KMessageBox::Cancel) {
          closeEditView(pEditView);
          return true;
        }
        else {
          return false;
        }
      }
      else if (KHTMLView* pHTMLView = dynamic_cast<KHTMLView*> (pView)) {
        closeBrowserView(pHTMLView);
        return true;
      }
      else {
        kdDebug() << "expected EditView but found some unknown QWidget, keep looking\n";
      }
    }
  }
  kdDebug() << "nothing found but useless junk, life sucks\n";
  return false;
}

//-----------------------------------------------------------------------------
// close an edit view
//-----------------------------------------------------------------------------
void DocViewMan::closeEditView(Kate::View* pView)
{
  if (!pView) return;

  // Get the document
  Kate::Document* pDoc = pView->getDoc();

  QextMdiChildView* pMDICover = (QextMdiChildView*) pView->parentWidget();
  pMDICover->hide();

  // disconnect the focus signals
  disconnect(pMDICover, SIGNAL(activated(QextMdiChildView*)),
    this, SLOT(slot_viewActivated(QextMdiChildView*)));

  /* if there are no more views, the pointer have to be "reset" here,
   * because slot_viewActivated() can not be called any longer
   */
//  if (m_MDICoverList.count() == 0) {
    m_pCurEditView = 0L;
    m_pCurBrowserView = 0L;
//  }

  // remove the view from MDI and delete the view
  m_pParent->removeWindowFromMdi( pMDICover);
  m_MDICoverList.remove( pMDICover);

  if (pDoc->views().count() == 0) {
    closeKWriteDoc(pDoc);
  }
}

//-----------------------------------------------------------------------------
// close a browser view
//-----------------------------------------------------------------------------
void DocViewMan::closeBrowserView(KHTMLView* pView)
{
  CDocBrowser* pDoc = (CDocBrowser*) pView->part();

  QextMdiChildView* pMDICover = (QextMdiChildView*) pView->parentWidget();
  pMDICover->hide();

  // disconnect the focus signals
  disconnect(pMDICover, SIGNAL(activated(QextMdiChildView*)), this, SLOT(slot_viewActivated(QextMdiChildView*)));

  // get a KHTMLView out of the parent to avoid a delete,
  // it will be deleted later in the CDocBrowser destructor
  pView->reparent(0L,0,QPoint(0,0));
  QApplication::sendPostedEvents();

  // remove the view from MDI and delete the view
  m_pParent->removeWindowFromMdi( pMDICover);
  m_MDICoverList.remove( pMDICover);

  closeCDocBrowser(pDoc);
}

void DocViewMan::slotResetMainFrmCaption()
{
  if (m_pParent) {
    m_pParent->setMainCaption();
  }
}

//-----------------------------------------------------------------------------
// get the first edit view for a document
//-----------------------------------------------------------------------------
Kate::View* DocViewMan::getFirstEditView(Kate::Document* pDoc) const
{
  return (dynamic_cast<Kate::View*> (pDoc->views().getFirst()));
}

//-----------------------------------------------------------------------------
// Connected to the focus in event occures signal of CEditWidget.
// Moves the focused view to the end of the list of focused views or
// adds it. Emits the sig_viewActivated signal
//-----------------------------------------------------------------------------
void DocViewMan::slot_viewActivated(QextMdiChildView* pMDICover)
{
  // set current view, distinguish between edit widget and browser widget
  QObjectList* pL = (QObjectList*) pMDICover->children();
  QWidget* pView = 0L;
  QObject* pChild;
  for ( pChild = pL->first(); pChild && !pView; pChild = pL->next()) {
    if (pChild->isWidgetType()) {
      pView = (QWidget*) pChild;
    }
  }

  if (!pView) return;

  if (pView->inherits("Kate::View")) {
    m_pCurEditView = (Kate::View*) pView;
    m_pCurEditDoc = m_pCurEditView->getDoc();
    m_curIsBrowser = false;

// let kate take care of this (rokrau 6/25/01)
//    // check if the file has been modified outside
//    // --------
//    // Get the current file name
//    QString filename = m_pCurEditView->getName();
//    if (m_pCurEditDoc == 0)
//      return; //oops :-(

//    // check if it modified inside KDevelop
//    bool bModifiedInside = false;
//    if (m_pCurEditDoc->isModified()) {
//      bModifiedInside = true;
//    }
    // check if it is modified outside KDevelop
//    bool bModifiedOutside = false;
//    QFileInfo file_info(filename);
//    if ((file_info.lastModified() != m_pCurEditDoc->getLastFileModifDate())) {
//      bModifiedOutside = true;
//    }
//    if (bModifiedInside && bModifiedOutside) {
//      if (KMessageBox::warningYesNo(m_pParent
//               ,i18n("This file %1 was modified inside but also outside this editor.\n"
//                     "Do you want to keep your changes or reload it from disk?")
//               .arg(filename), i18n("File modified")
//               ,i18n("&Keep"), i18n("&Reload")) == KMessageBox::No) {
//        loadKWriteDoc(m_pCurEditDoc, filename, 1);
//      }
//      else {
//        m_pCurEditDoc->setLastFileModifDate(file_info.lastModified());
//      }
//    }
//    else if (bModifiedOutside) {
//      if (KMessageBox::questionYesNo(m_pParent
//               ,i18n("This file %1 was modified outside this editor.\n"
//                    "Do you want reload it from disk?")
//               .arg(filename), i18n("File modified")
//               ,i18n("&Yes"), i18n("&No")) == KMessageBox::Yes) {
//        loadKWriteDoc(m_pCurEditDoc, filename, 1);
//      }
//      else {
//        m_pCurEditDoc->setLastFileModifDate(file_info.lastModified());
//      }
//    }
  }
  else {
    m_pCurBrowserView = (KHTMLView*) pView;
    m_pCurBrowserDoc = (CDocBrowser*) m_pCurBrowserView->part();
    m_curIsBrowser = true;
  }

  // emit the got focus signal
  // (connected to CKDevelop but could also be caught by other ones)
  emit sig_viewActivated(pView);
}

//-----------------------------------------------------------------------------
// Returns the number of documents
//-----------------------------------------------------------------------------
int DocViewMan::docCount() const
{
  return m_documentList.count();
}

//-----------------------------------------------------------------------------
// Retrieves the Kate::Document found by its filename
//-----------------------------------------------------------------------------
Kate::Document* DocViewMan::findKWriteDoc(const QString& strFileName) const
{
  QListIterator<QObject> itDoc(m_documentList);
  for (itDoc.toFirst(); itDoc.current() != 0; ++itDoc) {
    Kate::Document* pDoc = dynamic_cast<Kate::Document*> (itDoc.current());
    if(pDoc && (pDoc->docName() == strFileName)) {
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
// Find if no documents have been modified
//-----------------------------------------------------------------------------
bool DocViewMan::noDocModified()
{
  QListIterator<QObject> itDoc(m_documentList);
  for (itDoc.toFirst(); itDoc.current() != 0; ++itDoc) {
    Kate::Document* pDoc = dynamic_cast<Kate::Document*> (itDoc.current());
    if(pDoc && pDoc->isModified()) {
      return false;
    }
  }
  return true;
}


bool DocViewMan::doFileClose()
{
  // we want to close a whole document,
  // this means to save and close all views!

  // get the current view (one of possibly many view of the document)
  Kate::View* pCurEditView = currentEditView();
  if (!pCurEditView) return true;
  QString filename = pCurEditView->getDoc()->docName();

  // close the current view of the document (this will ask the user in case of being modified)
  bool bClosed = closeView((QextMdiChildView*)pCurEditView->parentWidget());

  if (!bClosed) // action was cancelled
    return false;

  // if there was only one view of a document, the call of closeView() has closed the document as well.
  // That's why for closing of possible other views we must check if the document is still alive
  Kate::Document* pDoc = findKWriteDoc(filename);
  if (pDoc) {
     // now close all other open views of this document
     closeKWriteDoc( pDoc);
  }
  return true;
}

// closes all KWrite documents and their views
// but not the document browser views
void DocViewMan::doFileCloseAll()
{
  QStrList handledNames;
  bool cont=true;

  QListIterator<QObject> itDoc(m_documentList);
  itDoc.toFirst();
  QObject* itDocObject;
  while ( (itDocObject = itDoc.current()) )
  {
    Kate::Document* pDoc = dynamic_cast<Kate::Document*> (itDocObject);
    if (pDoc
        && pDoc->isModified()
        && handledNames.contains(pDoc->docName())<1)
    {
      SaveAllDialog::SaveAllResult result = m_pParent->doProjectSaveAllDialog(pDoc->docName());
      switch (result)
      {
        case SaveAllDialog::Yes:
        { // Yes- only save the current file
          // save file as if Untitled and close file
          m_bCloseActionPending = true;
          if(m_pParent->isUntitled(pDoc->docName()))
          {
            m_pParent->switchToFile(pDoc->docName());
            handledNames.append(pDoc->docName());
            cont = m_pParent->fileSaveAs();
          }
          else
          { // Save file and close it
            m_pParent->switchToFile(pDoc->docName());
            handledNames.append(pDoc->docName());
            m_pParent->slotFileSave();
            // this doesnt work anymore (rokrau 12/09/01)

            //cont = !currentEditView()->isModified(); //something went wrong
          }
          m_bCloseActionPending = false;
          break;
        }

        case SaveAllDialog::No:
        {
          // No - no save but close
          handledNames.append(pDoc->docName());
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
  QStrList handledNames;
  bool cont = true; // continue to close the project

  // synchronizeDocAndInfo();

  QListIterator<QObject> itDoc(m_documentList);
  for (itDoc.toFirst(); cont && itDoc.current() != 0; ++itDoc) {
    Kate::Document* pDoc = dynamic_cast<Kate::Document*> (itDoc.current());
    if (pDoc
        && pDoc->isModified()
        && handledNames.contains(pDoc->docName())<1) {

      SaveAllDialog::SaveAllResult result = m_pParent->doProjectSaveAllDialog(pDoc->docName());

        // what to do
      if(result==SaveAllDialog::Yes) {  // Yes- only save the actual file
        m_bCloseActionPending = true;
        // save file as if Untitled and close file
        if(m_pParent->isUntitled(pDoc->docName())) {
          m_pParent->switchToFile(pDoc->docName());
          handledNames.append(pDoc->docName());
          cont = m_pParent->fileSaveAs();
          // start again... 'cause we deleted an entry
          itDoc.toFirst();
        }
        // Save file and close it
        else {
          m_pParent->switchToFile(pDoc->docName());
          handledNames.append(pDoc->docName());
          m_pParent->slotFileSave();
          cont = ! pDoc->isModified(); //something went wrong
        }
        m_bCloseActionPending = false;
      }

      if(result==SaveAllDialog::No) {   // No - no save but close
        handledNames.append(pDoc->docName());
        pDoc->setModified(false);
        // start again... 'cause we deleted an entry
        itDoc.toFirst();
      }

      if(result==SaveAllDialog::SaveAll) {  // Save all
        m_pParent->slotFileSaveAll();
        break;
      }

      if (result==SaveAllDialog::Cancel) { // Cancel
        cont=false; // not continuing to close the project
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
  QListIterator<QObject> itDoc(m_documentList);
  while (itDoc.current() != 0L) {
    Kate::Document* pEditDoc = dynamic_cast<Kate::Document*> (itDoc.current());
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

int DocViewMan::checkAndSaveFileOfCurrentEditView(bool bDoModifiedInsideCheck, Kate::View* pCurEditView)
{
  // Get the current file name
  QString filename;
  Kate::Document* pCurEditDoc;
  if (!pCurEditView) {
    pCurEditView = currentEditView();
    pCurEditDoc = currentEditDoc();
  }
  else {
    pCurEditDoc = pCurEditView->getDoc();
  }

  if (pCurEditDoc == 0)
    return KMessageBox::Cancel; //oops :-(

  filename = pCurEditDoc->docName();

//  // check if it modified inside KDevelop
//  bool bModifiedInside = false;
//  if (pCurEditDoc->isModified()) {
//    bModifiedInside = true;
//  }
//  // check if it is modified outside KDevelop
//  bool bModifiedOutside = false;
//  QFileInfo file_info(filename);
//  if ((file_info.lastModified() > pCurEditDoc->getLastFileModifDate())) {
//    bModifiedOutside = true;
//  }

//  if (!bModifiedInside && !bModifiedOutside)
//     return KMessageBox::No;  // nothing to save

//  int button=KMessageBox::Yes;
//  if (bModifiedInside && bModifiedOutside) {
//    button = KMessageBox::warningYesNoCancel(m_pParent
//             ,i18n("This file %1 was modified inside but also outside this editor.\n"
//                   "Do you want to reject your changes or overwrite the changes that happened outside?")
//             .arg(filename), i18n("File modified")
//             ,i18n("&Overwrite"), i18n("&Reject"));
//  }
//  else if (bDoModifiedInsideCheck && bModifiedInside) {
//    button = KMessageBox::warningYesNoCancel(m_pParent
//             ,i18n("The file %1 was modified.\n"
//                  "Do you want to save your changes?")
//             .arg(filename), i18n("File modified")
//             ,i18n("&Yes"), i18n("&No"));
//  }

//  switch (button) {
//  case KMessageBox::No:
//  case KMessageBox::Cancel:
//    return button;
//  default:
//    break;
//  }

  // Really save it
  currentEditView()->save();
// let kate take care of this (rokrau 6/25/01)
//  QFileInfo file_info2(filename);
//  pCurEditDoc->setLastFileModifDate(file_info2.lastModified());

  // refresh class tree-view
  QStrList lSavedFile;
  lSavedFile.append(filename);
#ifdef WITH_CPP_REPARSE
  if (m_pParent->hasProject())
#else
    if (m_pParent->hasProject() && getKWriteDocType(m_pCurEditDoc) == CPP_HEADER)
#endif
  m_pParent->refreshClassViewByFileList(&lSavedFile);
  return KMessageBox::Yes;
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
    Kate::Document* pDoc = dynamic_cast<Kate::Document*> (itDoc.current());
    if (pDoc) {
      int i = 0;
      pProgressBar->setProgress(++i);

      kdDebug() << "checking: " << pDoc->docName() << "\n";
      kdDebug() << " " << ((pDoc->isModified()) ?
      "modified" : "not modified") << "\n";

      if (!m_pParent->isUntitled(pDoc->docName())
          && pDoc->isModified()
          && handledNames.contains(pDoc->docName()) < 1) {
        int qYesNo = KMessageBox::Yes;
        handledNames.append(pDoc->docName());

// let kate take care of this (rokrau 6/25/01)
//        kdDebug() << " file info" << "\n";
//        QFileInfo file_info(pDoc->docName());
//        if (file_info.lastModified() != pDoc->getLastFileModifDate()) {
//          qYesNo = KMessageBox::questionYesNo(m_pParent,
//                                              i18n("The file %1 was modified outside\n"
//                                                   "this editor. Save anyway?")
//                                              .arg(pDoc->docName()),
//                                              i18n("File modified"));
//        }


//      if (qYesNo == KMessageBox::Yes) {
//        kdDebug() << " KMessageBox::Yes" << "\n";
//          bool isModified=true;
          if (Kate::View* pEditView=getFirstEditView(pDoc)) {
            pEditView->save();
            // kind of awkward way to find out whether the save succeeded
//            isModified = pEditView->isModified();
//            kdDebug() << "save document: "
//                      << pEditView->getDoc()->docName() << ", "
//                      << ((!isModified) ? "succeeded" : "failed") << "\n";
          }

//          if (!isModified) {
#ifdef WITH_CPP_REPARSE
            mod = true;
#else
            mod |= (pDoc->docName().right(2)==".h" || pDoc->docName().right(4)==".hxx");
#endif
            iFileList.append(pDoc->docName());

            // file_info.lastModified has not recognized here the file modif time has changed
            // maybe a sync problem, maybe a Qt bug?
            // anyway, we have to create another file info, this works then.

            // let kate take care of this (rokrau 6/25/01)
            //QFileInfo fileInfoSavedFile(pDoc->docName());
            //pDoc->setLastFileModifDate(fileInfoSavedFile.lastModified());

//          }
//        }
      }
    }
  }

  pProgressBar->reset();

  if (m_pParent->hasProject() && !iFileList.isEmpty() && mod)
    m_pParent->refreshClassViewByFileList(&iFileList);

}

void DocViewMan::reloadModifiedFiles()
{
  QListIterator<QObject> itDoc(m_documentList);
  for (itDoc.toFirst(); itDoc.current() != 0; ++itDoc) {
    Kate::Document* pDoc = dynamic_cast<Kate::Document*> (itDoc.current());
    if (pDoc) {
      QFileInfo file_info(pDoc->docName());

      // Reload only changed files
//    if(pDoc->getLastFileModifDate() != file_info.lastModified()) {
      // hack, we need a way to check whether the document was modified outside
      // the current view
      // checking for the reload twice (rokrau 6/25/01)
        // Force reload, no modified on disc messagebox
        m_pParent->switchToFile(pDoc->docName(),-1,-1,true,false);
      //}
    }
  }
}


QList<Kate::Document> DocViewMan::getKWriteDocList() const
{
  QListIterator<QObject> itDoc(m_documentList);
  QList<Kate::Document> resultList;

  for (; itDoc.current() != 0; ++itDoc) {
    Kate::Document* doc = dynamic_cast<Kate::Document*> (itDoc.current());
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
  Kate::Document* kwDoc = (dynamic_cast<Kate::Document*> (pDoc));
  if (kwDoc) {
    return (kwDoc->docName());
  }

  CDocBrowser* brDoc = (dynamic_cast<CDocBrowser*> (pDoc));
  if (brDoc) {
    return (QString(brDoc->name()));
  }

  return QString("");
}

//-----------------------------------------------------------------------------
// Connect to the signals of the bookmark popup menus
//-----------------------------------------------------------------------------
void DocViewMan::setDocBMPopup(KPopupMenu* pDocBookmarksMenu)
{
        m_pDocBookmarksMenu = pDocBookmarksMenu;
//  debug("DocViewMan::installBMPopup");
//
//    // Install editor bookmark popup menu
//  QPopupMenu* code_bookmarks = new QPopupMenu();
//
//  connect(code_bookmarks,SIGNAL(aboutToShow()),
//          this,SLOT(updateCodeBMPopup()));
//  connect(code_bookmarks,SIGNAL(activated(int)),
//          this,SLOT(gotoCodeBookmark(int)));
//
//
//  bm_menu->insertItem(SmallIconSet("bookmark_folder"),
//                      i18n("Code &Window"),code_bookmarks,31000);
//
//    // Install browser bookmark popup menu
//  m_pDocBookmarksMenu = new QPopupMenu();
//
  connect(m_pDocBookmarksMenu,SIGNAL(activated(int)),
          this,SLOT(gotoDocBookmark(int)));
//
//  bm_menu->insertItem(SmallIconSet("bookmark_folder"),
//                      i18n("&Browser Window"), m_pDocBookmarksMenu,31010);
//
}

//-----------------------------------------------------------------------------
// Updates the bookmarks for each editor document
//-----------------------------------------------------------------------------
void DocViewMan::updateCodeBMPopup()
{
//  QPopupMenu* popup = (QPopupMenu *) sender();
//
//  // Remove all menu items
//  popup->clear();
//
//  // Insert separator
//  popup->insertSeparator();
//
//  // Update bookmarks for each document
//  QListIterator<QObject> itDoc(m_documentList);
//  for (itDoc.toFirst(); itDoc.current() != 0; ++itDoc) {
//    Kate::Document* pDoc = dynamic_cast<Kate::Document*> (itDoc.current());
//    if(pDoc) {
//      pDoc->updateBMPopup(popup);
//    }
//  }
}

//-----------------------------------------------------------------------------
// Shows the desired editor bookmark
// (eventually, switches to file and activates it)
//-----------------------------------------------------------------------------
void DocViewMan::gotoCodeBookmark(int n) {

//  QPopupMenu* popup = (QPopupMenu *) sender();
//
//  QString text = popup->text(n);
//
//  // Find the Kate::Document for this bookmark
//  QListIterator<QObject> itDoc(m_documentList);
//  for (itDoc.toFirst(); itDoc.current() != 0; ++itDoc) {
//    Kate::Document* pDoc = dynamic_cast<Kate::Document*> (itDoc.current());
//    if(pDoc) {
//      if(text.contains(pDoc->docName() + ";")) {
//        m_pParent->switchToFile(pDoc->docName());
//        pDoc->gotoBookmark(text);
//        return;
//      }
//    }
//  }
}

//-----------------------------------------------------------------------------
// Shows the desired browser bookmark
// (eventually, switches to file and activates it)
//-----------------------------------------------------------------------------
void DocViewMan::gotoDocBookmark(int id_)
{
    int id_index = m_pDocBookmarksMenu->indexOf(id_);

  m_pParent->openBrowserBookmark(m_docBookmarksList.at(id_index));
}

//-----------------------------------------------------------------------------
// Toggles a bookmark in the current document
//-----------------------------------------------------------------------------
void DocViewMan::doBookmarksToggle()
{
  if (curDocIsBrowser())
  {
    m_pDocBookmarksMenu->clear();

    // Check if the current URL is bookmarked
    int pos = m_docBookmarksList.find(currentBrowserDoc()->currentURL());
    if(pos > -1)
    {
      // The current URL is bookmarked, let's remove the bookmark
      m_docBookmarksList.remove(pos);
      m_docBookmarksTitleList.remove(pos);
    }
    else
    {
      CDocBrowser* pCurBrowserDoc = currentBrowserDoc();
      // The current URL is not bookmark, let's bookmark it
      m_docBookmarksList.append(pCurBrowserDoc->currentURL());
      m_docBookmarksTitleList.append(pCurBrowserDoc->currentTitle());
    }

    // Recreate thepopup menu
    for (uint i = 0 ; i < m_docBookmarksList.count(); i++){
      m_pDocBookmarksMenu->insertItem(SmallIconSet("html"),
                                      getBrowserMenuItem(i));
    }
  }
  else
  {
//    these are currently not available (rokrau 12/09/01)
//    if (currentEditView())
//      currentEditView()->toggleBookmark();
  }

}

//-----------------------------------------------------------------------------
// Clears bookmarks in the current document
//-----------------------------------------------------------------------------
void DocViewMan::doBookmarksClear()
{
  if (curDocIsBrowser())
    {
      m_docBookmarksList.clear();
      m_docBookmarksTitleList.clear();
      m_pDocBookmarksMenu->clear();
    }
  else
    {
      doClearBookmarks();
    }
}

//-----------------------------------------------------------------------------
// Goes to the next bookmark in the current document
//-----------------------------------------------------------------------------
void DocViewMan::doBookmarksNext()
{
  if (curDocIsBrowser())
  {
    if(m_docBookmarksList.count() > 0)
    {
      QString file = m_docBookmarksList.next();
      if (file.isEmpty())
        file = m_docBookmarksList.first();
      m_pParent->openBrowserBookmark(file);
    }
  }
  else
  {
    kdDebug() << "in DocViewMan::doBookmarksNext(), not yet implemented,"
              << "due to changes in the editor interface\n";
//    if (currentEditView())
//      currentEditView()->nextBookmark();
  }
}

//-----------------------------------------------------------------------------
// Goes to the previous bookmark in the current document
//-----------------------------------------------------------------------------
void DocViewMan::doBookmarksPrevious()
{
  if (curDocIsBrowser())
  {
    if(m_docBookmarksList.count() > 0)
    {
      QString file = m_docBookmarksList.prev();
      if(file.isEmpty())
        file = m_docBookmarksList.last();
      m_pParent->openBrowserBookmark(file);
    }
  }
  else
  {
    kdDebug() << "in DocViewMan::doBookmarksPrevious(), not yet implemented,"
              << "due to changes in the editor interface\n";
//    if (currentEditView())
//      currentEditView()->previousBookmark();
  }
}

//-----------------------------------------------------------------------------
// Get browser bookmark popup menu item
//-----------------------------------------------------------------------------
QString DocViewMan::getBrowserMenuItem(int index)
{
  QString bmTitle;
  if (index < 9) {
    bmTitle = QString ("&%1 %2").arg(index + 1)
      .arg(m_docBookmarksTitleList.at(index));
  } else {
    bmTitle = m_docBookmarksTitleList.at(index);
  }
  return bmTitle;
}

//-----------------------------------------------------------------------------
// Reads bookmarks from the config
//-----------------------------------------------------------------------------
void DocViewMan::readBookmarkConfig(KConfig* theConfig)
{
  theConfig->readListEntry("doc_bookmarks",m_docBookmarksList);
  theConfig->readListEntry("doc_bookmarks_title",m_docBookmarksTitleList);
  for ( uint i =0 ; i < m_docBookmarksTitleList.count(); i++)
    {
      m_pDocBookmarksMenu->insertItem(SmallIconSet("html"),
                                      getBrowserMenuItem(i));
    }
}

//-----------------------------------------------------------------------------
// Writes bookmarks from the config
//-----------------------------------------------------------------------------
void DocViewMan::writeBookmarkConfig(KConfig* theConfig)
{
  theConfig->writeEntry("doc_bookmarks", m_docBookmarksList);
  theConfig->writeEntry("doc_bookmarks_title", m_docBookmarksTitleList);
}

/**
* Helper method for the method below (slot entry point for QextMdi signals)
*/
void DocViewMan::initKeyAccel( QWidget* pTopLevelWidget)
{
  initKeyAccel(0L, pTopLevelWidget);
}

/**
* This method can be called from various places.
* CKDevelop calls it in its constructor, DocViewMan calls it from addQExtMDIFrame(..).
* Anyway, it sets the application accelerators for any _toplevel_ window of KDevelop.
*/
void DocViewMan::initKeyAccel( CKDevAccel* accel, QWidget* pTopLevelWidget)
{
  if (accel == 0L) {
    accel = new CKDevAccel( pTopLevelWidget );
  }

  accel->insertItem( i18n("Goto Previous Window"), "GotoPrevWin", IDK_GOTO_PREVWIN);
  accel->connectItem("GotoPrevWin", m_pParent, SLOT(activatePrevWin()), true, 0);
  accel->insertItem( i18n("Goto Next Window"), "GotoNextWin", IDK_GOTO_NEXTWIN);
  accel->connectItem("GotoNextWin", m_pParent, SLOT(activateNextWin()), true, 0);

  // view activation via alt+numberkey for tabpage mode
  // (Note: the program will call the index ( = viewnumber - 1))
  accel->insertItem( i18n("Activate View 1"), "ActivateView1", IDK_GOTO_EDITORVIEW1);
  accel->insertItem( i18n("Activate View 2"), "ActivateView2", IDK_GOTO_EDITORVIEW2);
  accel->insertItem( i18n("Activate View 3"), "ActivateView3", IDK_GOTO_EDITORVIEW3);
  accel->insertItem( i18n("Activate View 4"), "ActivateView4", IDK_GOTO_EDITORVIEW4);
  accel->insertItem( i18n("Activate View 5"), "ActivateView5", IDK_GOTO_EDITORVIEW5);
  accel->insertItem( i18n("Activate View 6"), "ActivateView6", IDK_GOTO_EDITORVIEW6);
  accel->insertItem( i18n("Activate View 7"), "ActivateView7", IDK_GOTO_EDITORVIEW7);
  accel->insertItem( i18n("Activate View 8"), "ActivateView8", IDK_GOTO_EDITORVIEW8);
  accel->insertItem( i18n("Activate View 9"), "ActivateView9", IDK_GOTO_EDITORVIEW9);
  accel->insertItem( i18n("Activate View 10"), "ActivateView10", IDK_GOTO_EDITORVIEW10);
  accel->connectItem("ActivateView1", this, SLOT(activateView1()), true, 0);
  accel->connectItem("ActivateView2", this, SLOT(activateView2()), true, 0);
  accel->connectItem("ActivateView3", this, SLOT(activateView3()), true, 0);
  accel->connectItem("ActivateView4", this, SLOT(activateView4()), true, 0);
  accel->connectItem("ActivateView5", this, SLOT(activateView5()), true, 0);
  accel->connectItem("ActivateView6", this, SLOT(activateView6()), true, 0);
  accel->connectItem("ActivateView7", this, SLOT(activateView7()), true, 0);
  accel->connectItem("ActivateView8", this, SLOT(activateView8()), true, 0);
  accel->connectItem("ActivateView9", this, SLOT(activateView9()), true, 0);
  accel->connectItem("ActivateView10", this, SLOT(activateView10()), true, 0);

  //file menu
  accel->connectItem( KStdAccel::New, m_pParent, SLOT(slotFileNew()), true, ID_FILE_NEW );
  accel->connectItem( KStdAccel::Open , m_pParent, SLOT(slotFileOpen()), true, ID_FILE_OPEN );
  accel->connectItem( KStdAccel::Close , m_pParent, SLOT(slotFileClose()), true, ID_FILE_CLOSE );

  accel->connectItem( KStdAccel::Save , m_pParent, SLOT(slotFileSave()), true, ID_FILE_SAVE );

  accel->insertItem(i18n("Save As"), "SaveAs", 0);
  accel->connectItem( "SaveAs", m_pParent, SLOT(slotFileSaveAs()), true, ID_FILE_SAVE_AS);

  accel->insertItem(i18n("Save All"), "SaveAll", 0);
  accel->connectItem( "SaveAll", m_pParent, SLOT(slotFileSaveAll()), true, ID_FILE_SAVE_ALL);

  accel->connectItem( KStdAccel::Print , m_pParent, SLOT(slotFilePrint()), true, ID_FILE_PRINT );
  accel->connectItem( KStdAccel::Quit, m_pParent, SLOT(slotFileQuit()), true, ID_FILE_QUIT );

  //edit menu
  accel->connectItem( KStdAccel::Undo , this, SLOT(slotEditUndo()), true, ID_EDIT_UNDO );

  accel->insertItem( i18n("Redo"), "Redo",IDK_EDIT_REDO );
  accel->connectItem( "Redo" , this, SLOT(slotEditRedo()), true, ID_EDIT_REDO  );

  accel->connectItem( KStdAccel::Cut , this, SLOT(slotEditCut()), true, ID_EDIT_CUT );
  accel->connectItem( KStdAccel::Copy , this, SLOT(slotEditCopy()), true, ID_EDIT_COPY );
  accel->connectItem( KStdAccel::Paste , this, SLOT(slotEditPaste()), true, ID_EDIT_PASTE );

  accel->insertItem( i18n("Indent"), "Indent",IDK_EDIT_INDENT );
  accel->connectItem( "Indent", m_pParent, SLOT(slotEditIndent() ), true, ID_EDIT_INDENT );

  accel->insertItem( i18n("Unindent"), "Unindent",IDK_EDIT_UNINDENT );
  accel->connectItem( "Unindent", m_pParent, SLOT(slotEditUnindent() ), true, ID_EDIT_UNINDENT );

  accel->insertItem( i18n("Comment"), "Comment",IDK_EDIT_COMMENT );
  accel->connectItem( "Comment", m_pParent, SLOT(slotEditComment() ), true, ID_EDIT_COMMENT );

  accel->insertItem( i18n("Uncomment"), "Uncomment",IDK_EDIT_UNCOMMENT );
  accel->connectItem( "Uncomment", m_pParent, SLOT(slotEditUncomment() ), true, ID_EDIT_UNCOMMENT );

  accel->insertItem( i18n("Insert File"), "InsertFile", 0);
  accel->connectItem( "InsertFile", this, SLOT(slotEditInsertFile()), true, ID_EDIT_INSERT_FILE );

  accel->connectItem( KStdAccel::Find, this, SLOT(slotEditSearch() ), true, IDK_EDIT_SEARCH );

  accel->insertItem( i18n("Repeat Search"), "RepeatSearch",IDK_EDIT_REPEAT_SEARCH );
  accel->connectItem( "RepeatSearch", this, SLOT(slotEditRepeatSearch(int) ), true, ID_EDIT_REPEAT_SEARCH );

  accel->insertItem( i18n("Repeat Search Back"), "RepeatSearchBack",IDK_EDIT_REPEAT_SEARCH_BACK );
  accel->connectItem( "RepeatSearchBack", this, SLOT(slotEditRepeatSearchBack() ), true, ID_EDIT_REPEAT_SEARCH_BACK );
  accel->connectItem( KStdAccel::Replace, m_pParent, SLOT(slotEditReplace() ), true, ID_EDIT_REPLACE );

  accel->insertItem( i18n("Search in Files"), "Grep", IDK_EDIT_GREP_IN_FILES );
  accel->connectItem( "Grep", m_pParent, SLOT(slotEditSearchInFiles() ), true, ID_EDIT_SEARCH_IN_FILES );

  accel->insertItem( i18n("Search Selection in Files"), "GrepSearch", IDK_EDIT_SEARCH_GREP_IN_FILES );
  accel->connectItem( "GrepSearch", m_pParent, SLOT(slotEditSearchText() ) );

  accel->insertItem( i18n("Search CTags Database"), "CTagsSearch", IDK_EDIT_TAGS_SEARCH );
  accel->connectItem( "CTagsSearch", m_pParent, SLOT(slotTagSearch() ), true, ID_EDIT_TAGS_SEARCH );

  accel->insertItem( i18n("Switch to Header/Source"), "CTagsSwitch", IDK_EDIT_TAGS_SWITCH );
  accel->connectItem( "CTagsSwitch", m_pParent, SLOT(slotTagSwitchTo() ), true, ID_EDIT_TAGS_SWITCH );

  accel->insertItem( i18n("Select All"), "SelectAll", IDK_EDIT_SELECT_ALL);
  accel->connectItem("SelectAll", this, SLOT(slotEditSelectAll() ), true, ID_EDIT_SELECT_ALL );

  accel->insertItem(i18n("Deselect All"), "DeselectAll", 0);
  accel->connectItem("DeselectAll", this, SLOT(slotEditDeselectAll()), true, ID_EDIT_DESELECT_ALL);

  accel->insertItem(i18n("Invert Selection"), "Invert Selection", 0);
  accel->connectItem("Invert Selection", this, SLOT(slotEditInvertSelection()), true, ID_EDIT_INVERT_SELECTION);

  accel->insertItem(i18n("Expand Text"), "Expand Text", CTRL + Key_J);
  accel->connectItem("Expand Text", this, SLOT(slotEditExpandText()), true, ID_EDIT_EXPAND_TEXT);

  accel->insertItem(i18n("Complete Text"), "Complete Text", CTRL + Key_Space);
  accel->connectItem("Complete Text", this, SLOT(slotEditCompleteText()), true, ID_EDIT_COMPLETE_TEXT);

  accel->insertItem(i18n("Expand Template"), "Expand Template", SHIFT + Key_Space);
  accel->connectItem("Expand Template", this, SLOT(slotEditExpandTemplate()), true, ID_EDIT_EXPAND_TEMPLATE);

  //view menu
  accel->insertItem( i18n("Goto Line"), "GotoLine",IDK_VIEW_GOTO_LINE);
  accel->connectItem( "GotoLine", m_pParent, SLOT( slotViewGotoLine()), true, ID_VIEW_GOTO_LINE );

  accel->insertItem( i18n("Next Error"), "NextError",IDK_VIEW_NEXT_ERROR);
  accel->connectItem( "NextError", m_pParent, SLOT( slotViewNextError()), true, ID_VIEW_NEXT_ERROR );

  accel->insertItem( i18n("Previous Error"), "PreviousError",IDK_VIEW_PREVIOUS_ERROR);
  accel->connectItem( "PreviousError", m_pParent, SLOT( slotViewPreviousError()), true, ID_VIEW_PREVIOUS_ERROR  );

  accel->insertItem( i18n("Dialog Editor"), "Dialog Editor", 0);
  accel->connectItem("Dialog Editor", m_pParent, SLOT(startDesigner()), true, ID_TOOLS_DESIGNER );

  accel->insertItem( i18n("Toogle Tree-View"), "Tree-View",IDK_VIEW_TREEVIEW);
  accel->connectItem( "Tree-View", m_pParent, SLOT(slotViewTTreeView()), true, ID_VIEW_TREEVIEW );

  accel->insertItem( i18n("Toogle Output-View"), "Output-View",IDK_VIEW_OUTPUTVIEW);
  accel->connectItem( "Output-View", m_pParent, SLOT(slotViewTOutputView()), true, ID_VIEW_OUTPUTVIEW );

  accel->insertItem( i18n("Toolbar"), "Toolbar", 0);
  accel->connectItem( "Toolbar", m_pParent, SLOT(slotViewTStdToolbar()), true, ID_VIEW_TOOLBAR );

  accel->insertItem( i18n("Browser-Toolbar"), "Browser-Toolbar", 0);
  accel->connectItem( "Browser-Toolbar", m_pParent, SLOT(slotViewTBrowserToolbar()), true, ID_VIEW_BROWSER_TOOLBAR );

  accel->insertItem( i18n("Statusbar"), "Statusbar", 0);
  accel->connectItem( "Statusbar", m_pParent, SLOT(slotViewTStatusbar()), true, ID_VIEW_STATUSBAR );

  accel->insertItem( i18n("MDI-View-Taskbar"), "MDI-View-Taskbar", 0);
  accel->connectItem( "MDI-View-Taskbar", m_pParent, SLOT(slotViewMdiViewTaskbar()), true, ID_VIEW_MDIVIEWTASKBAR );


  accel->insertItem( i18n("Preview dialog"), "Preview dialog",IDK_VIEW_PREVIEW);

  accel->insertItem( i18n("Refresh"), "Refresh", 0);
  accel->connectItem( "Refresh", m_pParent, SLOT(slotViewRefresh()), true, ID_VIEW_REFRESH);

  accel->insertItem( i18n("Goto Declaration"), "CVGotoDeclaration", 0);
  accel->connectItem( "CVGotoDeclaration", m_pParent,SLOT(slotClassbrowserViewDeclaration()),true, ID_CV_VIEW_DECLARATION); // project menu

  accel->insertItem( i18n("Goto Definition"), "CVGotoDefinition", 0);
  accel->connectItem( "CVGotoDefinition", m_pParent, SLOT(slotClassbrowserViewDefinition()), true,ID_CV_VIEW_DEFINITION );

  accel->insertItem( i18n("Class Declaration"), "CVGotoClass", 0);
  accel->connectItem( "CVGotoClass", m_pParent,SLOT(slotClassbrowserViewClass()),true, ID_CV_VIEW_CLASS_DECLARATION);

  accel->insertItem( i18n("Graphical Classview"), "CVViewTree", 0);
  accel->connectItem( "CVViewTree", m_pParent, SLOT(slotClassbrowserViewTree()), true, ID_CV_GRAPHICAL_VIEW );


  // projectmenu
  accel->insertItem( i18n("New Project"), "NewProject", 0);
  accel->connectItem( "NewProject", m_pParent, SLOT(slotProjectNewAppl()), true, ID_PROJECT_KAPPWIZARD );

  accel->insertItem( i18n("Open Project"), "OpenProject", 0);
  accel->connectItem( "OpenProject", m_pParent, SLOT(slotProjectOpen()), true, ID_PROJECT_OPEN );

  accel->insertItem( i18n("Close Project"), "CloseProject", 0);
  accel->connectItem("CloseProject", m_pParent, SLOT(slotProjectClose()), true, ID_PROJECT_CLOSE );

  accel->insertItem(i18n("New Class"), "NewClass", 0);
  accel->connectItem("NewClass", m_pParent, SLOT(slotProjectNewClass()), true, ID_PROJECT_NEW_CLASS );

  accel->insertItem(i18n("Add Existing File(s)"), "AddExistingFiles", 0);
  accel->connectItem("AddExistingFiles",m_pParent, SLOT(slotProjectAddExistingFiles()), true, ID_PROJECT_ADD_FILE_EXIST );

  accel->insertItem(i18n("Add new Translation File"),"Add new Translation File", 0);
  accel->connectItem("Add new Translation File", m_pParent, SLOT(slotProjectAddNewTranslationFile()), true, ID_PROJECT_ADD_NEW_TRANSLATION_FILE );

  accel->insertItem(i18n("File Properties"), "FileProperties", IDK_PROJECT_FILE_PROPERTIES);
  accel->connectItem("FileProperties", m_pParent, SLOT(slotProjectFileProperties() ), true, ID_PROJECT_FILE_PROPERTIES );

  accel->insertItem(i18n("Make Messages and Merge"), "MakeMessages", 0);
  accel->connectItem("MakeMessages", m_pParent, SLOT(slotProjectMessages()), true, ID_PROJECT_MESSAGES  );

  accel->insertItem(i18n("Make API-Doc"), "ProjectAPI", 0);
  accel->connectItem("ProjectAPI", m_pParent, SLOT(slotProjectAPI()), true, ID_PROJECT_MAKE_PROJECT_API );

  accel->insertItem(i18n("Make User-Manual..."), "ProjectManual", 0);
  accel->connectItem("ProjectManual", m_pParent, SLOT(slotProjectManual()), true, ID_PROJECT_MAKE_USER_MANUAL);

  accel->insertItem(i18n("Make Source-tgz"), "Source-tgz", 0);
  accel->connectItem("Source-tgz", m_pParent, SLOT(slotProjectMakeDistSourceTgz()), true, ID_PROJECT_MAKE_DISTRIBUTION_SOURCE_TGZ );

  accel->insertItem(i18n("Project options"), "ProjectOptions", IDK_PROJECT_OPTIONS);
  accel->connectItem("ProjectOptions", m_pParent, SLOT(slotProjectOptions() ), true, ID_PROJECT_OPTIONS );

  accel->insertItem(i18n("Make Tags File"), "MakeTagsfile", 0);
  accel->connectItem("MakeTagsfile", m_pParent, SLOT(slotProjectMakeTags() ), true, ID_PROJECT_MAKE_TAGS );

  accel->insertItem(i18n("Load Tags File"), "LoadTagsfile", ID_PROJECT_LOAD_TAGS);
  accel->connectItem("LoadTagsfile", m_pParent, SLOT(slotProjectLoadTags() ), true, ID_PROJECT_LOAD_TAGS );

  //build menu
  accel->insertItem( i18n("Compile File"), "CompileFile", IDK_BUILD_COMPILE_FILE );
  accel->connectItem( "CompileFile", m_pParent, SLOT( slotBuildCompileFile()), true, ID_BUILD_COMPILE_FILE );

  accel->insertItem( i18n("Make"), "Make", IDK_BUILD_MAKE );
  accel->connectItem( "Make", m_pParent, SLOT(slotBuildMake() ), true, ID_BUILD_MAKE );

  accel->insertItem( i18n("Rebuild All"), "RebuildAll", 0);
  accel->connectItem( "RebuildAll", m_pParent, SLOT(slotBuildRebuildAll()), true, ID_BUILD_REBUILD_ALL );

  accel->insertItem( i18n("Clean/Rebuild all"), "CleanRebuildAll", 0);
  accel->connectItem( "CleanRebuildAll", m_pParent, SLOT(slotBuildCleanRebuildAll()), true, ID_BUILD_CLEAN_REBUILD_ALL );

  accel->insertItem( i18n("Stop Process"), "Stop_proc", IDK_BUILD_STOP);
  accel->connectItem( "Stop_proc", m_pParent, SLOT(slotBuildStop() ), true, ID_BUILD_STOP );

  accel->insertItem( i18n("Execute"), "Run", IDK_BUILD_RUN);
  accel->connectItem( "Run", m_pParent, SLOT(slotBuildRun() ), true, ID_BUILD_RUN );

  accel->insertItem( i18n("Execute with Arguments"), "Run_with_args", IDK_BUILD_RUN_WITH_ARGS);
  accel->connectItem( "Run_with_args", m_pParent, SLOT(slotBuildRunWithArgs() ), true, ID_BUILD_RUN_WITH_ARGS );

  accel->insertItem( i18n("DistClean"), "BuildDistClean", 0);
  accel->connectItem("BuildDistClean",m_pParent, SLOT(slotBuildDistClean()), true, ID_BUILD_DISTCLEAN );

  accel->insertItem( i18n("Make Clean"), "BuildMakeClean", 0);
  accel->connectItem("BuildMakeClean",m_pParent, SLOT(slotBuildMakeClean()), true, ID_BUILD_MAKECLEAN );

  accel->insertItem( i18n("Autoconf and Automake"), "BuildAutoconf", 0);
  accel->connectItem("BuildAutoconf", m_pParent,SLOT(slotBuildAutoconf()), true, ID_BUILD_AUTOCONF );

  accel->insertItem( i18n("Configure..."), "BuildConfigure", 0);
  accel->connectItem( "BuildConfigure", m_pParent, SLOT(slotBuildConfigure()), true, ID_BUILD_CONFIGURE );

  // Bookmarks-menu
  accel->insertItem( i18n("Toggle Bookmark"), "Toggle_Bookmarks", IDK_BOOKMARKS_TOGGLE);
  accel->connectItem( "Toggle_Bookmarks", m_pParent, SLOT(slotBookmarksToggle() ), true, ID_BOOKMARKS_TOGGLE );

  accel->insertItem( i18n("Next Bookmark"), "Next_Bookmarks", IDK_BOOKMARKS_NEXT);
  accel->connectItem( "Next_Bookmarks", m_pParent, SLOT(slotBookmarksNext() ), true, ID_BOOKMARKS_NEXT );

  accel->insertItem( i18n("Previous Bookmark"), "Previous_Bookmarks", IDK_BOOKMARKS_PREVIOUS);
  accel->connectItem( "Previous_Bookmarks", m_pParent, SLOT(slotBookmarksPrevious() ), true, ID_BOOKMARKS_PREVIOUS );

  accel->insertItem( i18n("Clear Bookmarks"), "Clear_Bookmarks", IDK_BOOKMARKS_CLEAR);
  accel->connectItem( "Clear_Bookmarks", m_pParent, SLOT(slotBookmarksClear() ), true, ID_BOOKMARKS_CLEAR );

  //Help menu
  accel->connectItem( KStdAccel::Help , m_pParent, SLOT(slotHelpContents()), true, ID_HELP_CONTENTS );

  accel->insertItem( i18n("Search Marked Text"), "SearchMarkedText",IDK_HELP_SEARCH_TEXT);
  accel->connectItem( "SearchMarkedText", m_pParent, SLOT(slotHelpSearchText() ), true, ID_HELP_SEARCH_TEXT );

  accel->insertItem( i18n("Search for Help on"), "HelpSearch", 0);
  accel->connectItem( "HelpSearch", m_pParent, SLOT(slotHelpSearch()), true, ID_HELP_SEARCH );

  accel->insertItem( i18n("View Project API-Doc"), "HelpProjectAPI", 0);
  accel->connectItem("HelpProjectAPI", m_pParent, SLOT(slotHelpAPI()), true, ID_HELP_PROJECT_API);

  accel->insertItem( i18n("View Project User-Manual"), "HelpProjectManual", 0);
  accel->connectItem( "HelpProjectManual", m_pParent, SLOT(slotHelpManual()), true, ID_HELP_USER_MANUAL);   // Tab-Switch

  // Debugger startups
  accel->insertItem( i18n("Debug Start"), "DebugStart", 0);
  accel->connectItem( "DebugStart", m_pParent, SLOT(slotBuildDebugStart()), true, ID_DEBUG_START);

  accel->insertItem( i18n("Debug Start Other"), "DebugStartOther", 0);
  accel->connectItem( "DebugStartOther", m_pParent, SLOT(slotDebugNamedFile()), true, ID_DEBUG_START_OTHER);

  accel->insertItem( i18n("Debug Start with Args"), "DebugRunWithArgs", 0);
  accel->connectItem( "DebugRunWithArgs", m_pParent, SLOT(slotDebugRunWithArgs()), true, ID_DEBUG_SET_ARGS);

  accel->insertItem( i18n("Debug Examine Core"), "DebugExamineCore", 0);
  accel->connectItem( "DebugExamineCore", m_pParent, SLOT(slotDebugExamineCore()), true, ID_DEBUG_CORE);

  accel->insertItem( i18n("Debug Other Executable"), "DebugOtherExec", 0);
  accel->connectItem( "DebugOtherExec", m_pParent, SLOT(slotDebugNamedFile()), true, ID_DEBUG_NAMED_FILE);

  accel->insertItem( i18n("Debug Attach"), "DebugAttach", 0);
  accel->connectItem( "DebugAttach", m_pParent, SLOT(slotDebugAttach()), true, ID_DEBUG_ATTACH);

  // Debugger actions
  accel->insertItem( i18n("Debug Run"), "DebugRun", 0);
  accel->connectItem( "DebugRun", m_pParent, SLOT(slotDebugRun()), true, ID_DEBUG_RUN );

  accel->insertItem( i18n("Debug Run to Cursor"), "DebugRunCursor", 0);
  accel->connectItem( "DebugRunCursor", m_pParent, SLOT(slotDebugRunToCursor()), true, ID_DEBUG_RUN_CURSOR );

  accel->insertItem( i18n("Debug Stop"), "DebugStop", 0);
  accel->connectItem( "DebugStop", m_pParent, SLOT(slotDebugStop()        ), true, ID_DEBUG_STOP);

  accel->insertItem( i18n("Debug Step Into"), "DebugStepInto", 0);
  accel->connectItem( "DebugStepInto", m_pParent, SLOT(slotDebugStepInto()), true, ID_DEBUG_STEP);

  accel->insertItem( i18n("Debug Step Into Instr"), "DebugStepIntoInstr", 0);
  accel->connectItem( "DebugStepIntoInstr", m_pParent, SLOT(slotDebugStepIntoIns()), true, ID_DEBUG_STEP_INST);

  accel->insertItem( i18n("Debug Step Over"), "DebugStepOver", 0);
  accel->connectItem( "DebugStepOver", m_pParent, SLOT(slotDebugStepOver()), true, ID_DEBUG_NEXT);

  accel->insertItem( i18n("Debug Step Over Instr"), "DebugStepOverInstr", 0);
  accel->connectItem( "DebugStepOverInstr", m_pParent, SLOT(slotDebugStepOverIns()), true, ID_DEBUG_NEXT_INST);

  accel->insertItem( i18n("Debug Step Out"), "DebugStepOut", 0);
  accel->connectItem( "DebugStepOut", m_pParent, SLOT(slotDebugStepOutOff()), true, ID_DEBUG_FINISH);

  accel->insertItem( i18n("Debug Viewers"), "DebugViewer", 0);
  accel->connectItem( "DebugViewer", m_pParent, SLOT(slotDebugMemoryView()), true, ID_DEBUG_MEMVIEW);

  accel->insertItem( i18n("Debug Interrupt"), "DebugInterrupt", 0);
  accel->connectItem( "DebugInterrupt", m_pParent, SLOT(slotDebugInterrupt()), true, ID_DEBUG_BREAK_INTO);

  accel->insertItem( i18n("Debug Toggle Breakpoint"), "DebugToggleBreakpoint", 0);
  accel->connectItem( "DebugToggleBreakpoint", m_pParent, SLOT(slotDebugToggleBreakpoint()), true, ID_DEBUG_TOGGLE_BP);

  // activation of certain tool-views
  accel->insertItem( i18n("Class Tree"), "ActivateTreeView_Class", ID_ACTIVATETREEVIEW_CLASS);
  accel->connectItem( "ActivateTreeView_Class", m_pParent, SLOT(slotActivateTView_Class()), true, 0);
  accel->insertItem( i18n("Files of Project"), "ActivateTreeView_LFV", ID_ACTIVATETREEVIEW_LFV);
  accel->connectItem( "ActivateTreeView_LFV", m_pParent, SLOT(slotActivateTView_LFV()), true, 0);
  accel->insertItem( i18n("View on Project Directory"), "ActivateTreeView_RFV", ID_ACTIVATETREEVIEW_RFV);
  accel->connectItem( "ActivateTreeView_RFV", m_pParent, SLOT(slotActivateTView_RFV()), true, 0);
  accel->insertItem( i18n("Documentation"), "ActivateTreeView_Doc", ID_ACTIVATETREEVIEW_DOC);
  accel->connectItem( "ActivateTreeView_Doc", m_pParent, SLOT(slotActivateTView_Doc()), true, 0);
  accel->insertItem( i18n("Debugger Variable Watch View"), "ActivateTreeView_VAR", ID_ACTIVATETREEVIEW_VAR);
  accel->connectItem( "ActivateTreeView_VAR", m_pParent, SLOT(slotActivateTView_VAR()), true, 0);

  accel->insertItem( i18n("Output of KDevelop"), "ActivateOutputView_Messages", ID_ACTIVATEOUTPUTVIEW_MESSAGES);
  accel->connectItem( "ActivateOutputView_Messages", m_pParent, SLOT(slotActivateOView_Messages()), true, 0);
  accel->insertItem( i18n("Messages of Started Program"), "ActivateOutputView_StdInStdOut", ID_ACTIVATEOUTPUTVIEW_STDINSTDOUT);
  accel->connectItem( "ActivateOutputView_StdInStdOut", m_pParent, SLOT(slotActivateOView_StdInStdOut()), true, 0);
  accel->insertItem( i18n("Error Messages of Started Program"), "ActivateOutputView_StdErr", ID_ACTIVATEOUTPUTVIEW_STDERR);
  accel->connectItem( "ActivateOutputView_StdErr", m_pParent, SLOT(slotActivateOView_StdErr()), true, 0);
  accel->insertItem( i18n("Embedded Konsole Window"), "ActivateOutputView_Konsole", ID_ACTIVATEOUTPUTVIEW_KONSOLE);
  accel->connectItem( "ActivateOutputView_Konsole", m_pParent, SLOT(slotActivateOView_Konsole()), true, 0);
  accel->insertItem( i18n("Debugger Breakpoints"), "ActivateOutputView_BrkptManager", ID_ACTIVATEOUTPUTVIEW_BRKPTMANAGER);
  accel->connectItem( "ActivateOutputView_BrkptManager", m_pParent, SLOT(slotActivateOView_BrkptManager()), true, 0);
  accel->insertItem( i18n("Debugger Function Call Stack"), "ActivateOutputView_FrameStack", ID_ACTIVATEOUTPUTVIEW_FRAMESTACK);
  accel->connectItem( "ActivateOutputView_FrameStack", m_pParent, SLOT(slotActivateOView_FrameStack()), true, 0);
  accel->insertItem( i18n("Debugger Disassemble View"), "ActivateOutputView_Disassemble", ID_ACTIVATEOUTPUTVIEW_DISASSEMBLE);
  accel->connectItem( "ActivateOutputView_Disassemble", m_pParent, SLOT(slotActivateOView_Disassemble()), true, 0);
  accel->insertItem( i18n("Debugger Control View"), "ActivateOutputView_Dbg", ID_ACTIVATEOUTPUTVIEW_DBG);
  accel->connectItem( "ActivateOutputView_Dbg", m_pParent, SLOT(slotActivateOView_Dbg()), true, 0);

  accel->readSettings(0, false);
}

void DocViewMan::slotToolbarClicked(int item)
{
  switch (item) {
  case ID_EDIT_UNDO:
    slotEditUndo();
    break;
  case ID_EDIT_REDO:
    slotEditRedo();
    break;
  case ID_EDIT_COPY:
    slotEditCopy();
    break;
  case ID_EDIT_PASTE:
    slotEditPaste();
    break;
  case ID_EDIT_CUT:
    slotEditCut();
    break;
    }
}

void DocViewMan::slotEditUndo()
{
  if (currentEditDoc())
    currentEditDoc()->undo();
}

void DocViewMan::slotEditRedo()
{
  if (currentEditDoc())
    currentEditDoc()->redo();
}

void DocViewMan::slotEditCut()
{
  if (currentEditView()) {
        emit sig_newStatus(i18n("Cutting..."));
        currentEditView()->cut();
        emit sig_newStatus(i18n("Ready."));
    }
}

void DocViewMan::slotEditCopy()
{
    emit sig_newStatus(i18n("Copying..."));
  doCopy();
    emit sig_newStatus(i18n("Ready."));
}

void DocViewMan::slotEditPaste()
{
  if (currentEditView()) {
        emit sig_newStatus(i18n("Pasting selection..."));
        currentEditView()->paste();
        emit sig_newStatus(i18n("Ready."));
    }
}

void DocViewMan::slotEditInsertFile()
{
  kdDebug() << "in DocViewMan::slotEditInsertFile(), not yet implemented,"
            << "due to changes in the editor interface\n";
// sadly the new Kate interface doesnt allow for inserting of files, yet.
// it looks that we need to open the URL from here, and the call
// KateDocument::insertFile, not as simple as it used to be.
//  if (currentEditView()) {
//        emit sig_newStatus(i18n("Inserting file contents..."));
//        currentEditView()->insertFile();
//        emit sig_newStatus(i18n("Ready."));
//    }
}

void DocViewMan::slotEditSearch(){
  emit sig_newStatus(i18n("Searching..."));
  doSearch();
  emit sig_newStatus(i18n("Ready."));
}

void DocViewMan::slotEditRepeatSearch(int back)
{
  emit sig_newStatus(i18n("Repeating last search..."));
  doRepeatSearch(m_pParent->getDocSearchText(), back);
  emit sig_newStatus(i18n("Ready."));
}

void DocViewMan::slotEditRepeatSearchBack()
{
  slotEditRepeatSearch(1);        // flag backward search
}

void DocViewMan::slotEditSelectAll()
{
  if (currentEditDoc()) {
        emit sig_newStatus(i18n("Selecting all..."));
        currentEditDoc()->selectAll();
        emit sig_newStatus(i18n("Ready."));
    }
}

void DocViewMan::slotEditInvertSelection()
{
// this is currently not available through the interface (rokrau 12/09/01)
//  if (currentEditView())
//    currentEditView()->invertSelection();
}
void DocViewMan::slotEditDeselectAll()
{
  if (currentEditDoc())
    currentEditDoc()->clearSelection();
}

void DocViewMan::activateView1()
{
   m_pParent->activateView(0);
}

void DocViewMan::activateView2()
{
   m_pParent->activateView(1);
}

void DocViewMan::activateView3()
{
   m_pParent->activateView(2);
}

void DocViewMan::activateView4()
{
   m_pParent->activateView(3);
}

void DocViewMan::activateView5()
{
   m_pParent->activateView(4);
}

void DocViewMan::activateView6()
{
   m_pParent->activateView(5);
}

void DocViewMan::activateView7()
{
   m_pParent->activateView(6);
}

void DocViewMan::activateView8()
{
   m_pParent->activateView(7);
}

void DocViewMan::activateView9()
{
   m_pParent->activateView(8);
}

void DocViewMan::activateView10()
{
   m_pParent->activateView(9);
}

void DocViewMan::slotEditExpandText()
{
    if (currentEditView())
        m_pCodeCompletion->expandText();
}

void DocViewMan::slotEditCompleteText()
{
    if (currentEditView())
        m_pCodeCompletion->completeText();
}

void DocViewMan::slotEditExpandTemplate()
{
    //if( currentEditView() )
    m_pCodeTemplate->expandTemplate( currentEditView() );
}

#include "docviewman.moc"
