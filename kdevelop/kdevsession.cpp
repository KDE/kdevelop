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

#include <iostream>
using namespace std;


KDevSession::KDevSession(DocViewMan* pDocViewMan, const QString& /*fileName*/)
  : m_pDocViewMan(pDocViewMan)
{
	initXMLTree();
}
/** initializes the XML tree on startup of kdevelop and when a project has been closed to ensure
that the XML tree exists already including doctype when a project gets opened that doesn't have
a kdevses file or a new project gets generated (which doesn't have a kdevses file either as the project
has never been closed before opening it). */
void KDevSession::initXMLTree(){
  domdoc.clear();
  QDomDocument doc("KDevPrjSession");
  domdoc=doc;
//	QDomDocumentType type=domdoc.doctype();
//	type.setNodeValue("KDevPrjSession");
  domdoc.appendChild( domdoc.createProcessingInstruction( "xml", "version=\"1.0\" encoding=\"UTF-8\"" ) );
	// KDevPrjSession is the root element of the XML file
	QDomElement session = domdoc.documentElement();
  session = domdoc.createElement("KDevPrjSession");
  domdoc.appendChild( session);
}
//----------- helper functions ------------------//
/** find the QDomElement of the configuration configname and return a reference to it. Used by all get/set functions. */
const QDomElement& KDevSession::getConfigElement(const QString& configname){
  helper = domdoc.documentElement();
	helper = helper.namedItem("CompileConfigs").toElement();
	helper = helper.namedItem(configname).toElement();
	return helper;
}
/** creates a new QDomElement or fills an existing one with a textnode as its only child. the QDomElement name is equal to name, the
textnode value is value.  */
void KDevSession::createEntry(QDomElement parent, const QString& name, const QString& value){
	QDomElement ar = parent.namedItem(name).toElement();
	if( ar.isNull()){
  	QDomElement ar = domdoc.createElement(name);
		parent.appendChild(ar);
		ar.appendChild(domdoc.createTextNode(value));
	}	
	else if(ar.firstChild().isNull())
		ar.appendChild(domdoc.createTextNode(value));	
	else
		ar.firstChild().toText().setNodeValue(value);
}

//----------- helper functions ------------------//

//----------- GET functions ---------------------//
/** No descriptions */
QStringList KDevSession::getCompileConfigs(){
	//resulting stringlist to return
	QStringList configs;
	// root element
  QDomElement el = domdoc.documentElement();
	// point el to the CompileConfigs tag
	el=el.namedItem("CompileConfigs").toElement();
	// point n to first child and so forth to get the tag names of the configs
  QDomNode n = el.firstChild();

  while( !n.isNull() ) {
      QDomElement e = n.toElement(); // try to convert the node to an element.
      if( !e.isNull() ) { // the node was really an element.
          configs.append( e.tagName() );
      }
      n = n.nextSibling();
  }
  return configs;
}
/** returns the last used compile config for restore after loading */
QString KDevSession::getLastCompile(){
	QString comp;
  QDomElement e = domdoc.documentElement();
	e=e.namedItem("LastCompileConfig").toElement();
	return e.text();
}

/** returns the architecture set for the given configuration (which is the processortype) */
QString KDevSession::getArchitecture(const QString& configname){
	QDomElement e=getConfigElement(configname);
	e=e.namedItem("Architecture").toElement();
	return e.text();
}
/** returns the platform of the given configuration (Operating System) */
QString KDevSession::getPlatform(const QString& configname){
	QDomElement e=getConfigElement(configname);
	e=e.namedItem("Platform").toElement();
	return e.text();
}
/** returns the VPATH subdir configured for this configuration to start compilation in. */
QString KDevSession::getVPATHSubdir(const QString& configname){
	QDomElement e=getConfigElement(configname);
	e=e.namedItem("VPATHSubdir").toElement();
	return e.text();
}
/** return the CFLAGS for the given configuration to export before compiling. */
QString KDevSession::getCFLAGS(const QString& configname){
	QDomElement e=getConfigElement(configname);
	e=e.namedItem("CFLAGS").toElement();
	return e.text();
}
/** return the CPPFLAGS set for the given configuration. */
QString KDevSession::getCPPFLAGS(const QString& configname){
	QDomElement e=getConfigElement(configname);
	e=e.namedItem("CPPFLAGS").toElement();
	return e.text();
}
/** return the CXXFLAGS string for the configuration configname */
QString KDevSession::getCXXFLAGS(const QString& configname){
	QDomElement e=getConfigElement(configname);
	e=e.namedItem("CXXFLAGS").toElement();
	return e.text();
}
/** in analogy to the kdevprj project file all flags who are not occupied by the GUI are stored in the additional CXXFLAGS and put into the lineedit
of the project options dialog. */
QString KDevSession::getAdditCXXFLAGS(const QString& configname){
	QDomElement e=getConfigElement(configname);
	e=e.namedItem("AdditCXXFLAGS").toElement();
	return e.text();
}
/** retrieve the linker flags (LDFLAGS) for the configuration configname. */
QString KDevSession::getLDFLAGS(const QString& configname){
	QDomElement e=getConfigElement(configname);
	e=e.namedItem("LDFLAGS").toElement();
	return e.text();
}
/** return the configure arguments for this configuration. */
QString KDevSession::getConfigureArgs(const QString& configname){
	QDomElement e=getConfigElement(configname);
	e=e.namedItem("ConfigureArgs").toElement();
	return e.text();
}

//----------- SET / ADD functions ---------------------//

/** add a new compile configuration (configname)  to the config list (creates a new node in the tree) */
void KDevSession::addCompileConfig(const QString& configname){
	// root element
  QDomElement el = domdoc.documentElement();
  QDomElement configs = el.namedItem("CompileConfigs").toElement();
  if(configs.isNull()){
  	configs = domdoc.createElement("CompileConfigs"); //create on rootdocument
		el.appendChild(configs);	// append on the documentElement(inside root tree)
	}
	// now create the new configname as child of configs
	QDomElement cfg = domdoc.createElement(configname);
	configs.appendChild(cfg);
}
/** sets the last compile config  before the project was closed. */
void KDevSession::setLastCompile(const QString& comp){
  QDomElement el = domdoc.documentElement();
  QDomElement configs = el.namedItem("LastCompileConfig").toElement();
  if(configs.isNull()){
  	configs = domdoc.createElement("LastCompileConfig"); //create on rootdocument
		el.appendChild(configs);	// append on the documentElement(inside root tree)
		configs.appendChild(domdoc.createTextNode(comp));
	}
	else if(configs.firstChild().isNull())
		configs.appendChild(domdoc.createTextNode(comp));	
	else
		configs.firstChild().toText().setNodeValue(comp);
}
/** sets the architecture (processor type) for the configuration configname */
void KDevSession::setArchitecture(const QString& configname, const QString& arch){
	QDomElement config = getConfigElement(configname);
	createEntry(config, "Architecture", arch);
}
/** sets the platform (Operating System) for the given configuration configname */
void KDevSession::setPlatform(const QString& configname, const QString& platform){
	QDomElement config = getConfigElement(configname);
	createEntry(config, "Platform", platform);
}
/** sets the VPATH subdir for the configuration configname */
void KDevSession::setVPATHSubdir(const QString& configname, const QString& subdir){
	QDomElement config = getConfigElement(configname);
	createEntry(config, "VPATHSubdir", subdir);
}
/** sets the CFLAGS string for the given configuration configname */
void KDevSession::setCFLAGS(const QString& configname, const QString& cflags){
	QDomElement config = getConfigElement(configname);
	createEntry(config, "CFLAGS", cflags);
}
/** sets the CPPFLAGS string for the configuration configname */
void KDevSession::setCPPFLAGS(const QString& configname, const QString& cppflags){
	QDomElement config = getConfigElement(configname);
	createEntry(config, "CPPFLAGS", cppflags);
}
/** sets the CXXFLAGS string for the configuration configname */
void KDevSession::setCXXFLAGS(const QString& configname, const QString& cxxflags){
	QDomElement config = getConfigElement(configname);
	createEntry(config, "CXXFLAGS", cxxflags);
}
/** In analogy to the kdevprj project file the additional cxxflags not occupied by the configuration dialog
are put in here. */
void KDevSession::setAdditCXXFLAGS(const QString& configname, const QString& additflags){
	QDomElement config = getConfigElement(configname);
	createEntry(config, "AdditCXXFLAGS", additflags);
}
/** sets the linker flags for the configuration configname. */
void KDevSession::setLDFLAGS(const QString& configname, const QString& ldflags){
	QDomElement config = getConfigElement(configname);
	createEntry(config, "LDFLAGS", ldflags);
}
/** sets the configure arguments for the configuration configname */
void KDevSession::setConfigureArgs(const QString& configname,  const QString& confargs){
	QDomElement config = getConfigElement(configname);
	createEntry(config, "ConfigureArgs", confargs);
}

//-------------- Falk's Docs recreation ---------//
/** No descriptions */
void KDevSession::recreateDocs(QDomElement& el){
  QDomElement mainframeEl = el.namedItem("Mainframe").toElement();
  bool bMaxMode = (bool) mainframeEl.attribute("MaximizeMode", "0").toInt();
  QextMdiMainFrm* pMainWidget = (QextMdiMainFrm*) qApp->mainWidget();
  pMainWidget->setEnableMaximizedChildFrmMode(bMaxMode);
  bool bTaskBarWasOn = pMainWidget->isViewTaskBarOn();
  pMainWidget->hideViewTaskBar();

  // read the information about the documents
  QDomElement docsAndViewsEl = el.namedItem("DocsAndViews").toElement();
  int nNrOfDocs = docsAndViewsEl.attribute("NumberOfDocuments", "0").toInt();
  // loop over all docs
  int   nDoc;
  QDomElement docEl;
  for (docEl = docsAndViewsEl.firstChild().toElement(), nDoc = 0; nDoc < nNrOfDocs; nDoc++, docEl = docEl.nextSibling().toElement()) {
    // read the document name and type
    QString docName = docEl.attribute( "FileName", "");
    QString docType = docEl.attribute( "Type", "Unknown");
    if (!docName.isEmpty() && QFile::exists(docName)) {
      // create the document
      if (docType == QString("KWriteDoc")) {

        KWriteDoc* pDoc = m_pDocViewMan->createKWriteDoc(docName);
        if (pDoc) {
          // load contents from file
          QFile f(docName);
          if (f.open(IO_ReadOnly)) {
            pDoc->loadFile(f);
            f.close();
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

  QextMdiChildView* pLastView = 0L;
  QextMdiChildFrm*  pLastFrm = 0L;
  QextMdiIterator<QextMdiChildView*>* winListIter = pMainWidget->createIterator();
  for(winListIter->first(); !winListIter->isDone(); winListIter->next()){
    pLastView = winListIter->currentItem();
    if (bMaxMode && pLastView->isAttached()) {
      pLastFrm = pLastView->mdiParent();
    }
  }
  // evil hack (of Falk): resize the childframe again 'cause of event timing probs with resizing
  if (bMaxMode && pLastFrm && pLastFrm->parentWidget()) {
    QApplication::sendPostedEvents();
    pLastFrm->setGeometry(-QEXTMDI_MDI_CHILDFRM_BORDER,
                          -QEXTMDI_MDI_CHILDFRM_BORDER - pLastFrm->captionHeight() - QEXTMDI_MDI_CHILDFRM_SEPARATOR,
                          pLastFrm->parentWidget()->width() + QEXTMDI_MDI_CHILDFRM_DOUBLE_BORDER,
                          pLastFrm->parentWidget()->height() + QEXTMDI_MDI_CHILDFRM_SEPARATOR + QEXTMDI_MDI_CHILDFRM_DOUBLE_BORDER + pLastFrm->captionHeight());
  }
  // show the last (the one above all)
  if (pLastView) {
    pLastView->show();
    QApplication::sendPostedEvents();
    // show the rest
    for(winListIter->first(); !winListIter->isDone(); winListIter->next()){
      winListIter->currentItem()->show();
    }
  }
  if (bTaskBarWasOn) {
     pMainWidget->showViewTaskBar();
  }
}

//-------------- Falk's Views recreation ---------//
//------------------------------------------------------------------------------
void KDevSession::recreateViews( QObject* pDoc, QDomElement docEl)
{
  // read information about the views
  int nNrOfViews = docEl.attribute( "NumberOfViews", "0").toInt();
  // loop over all views of this document
  int nView;
  QDomElement viewEl;
  QWidget* pFocusedView = 0L;
  const bool HIDE = false;
  QString viewType;
  QWidget* pView = 0L;
  for (viewEl = docEl.firstChild().toElement(), nView = 0; nView < nNrOfViews; nView++, viewEl = viewEl.nextSibling().toElement()) {
    // get type
    viewType = viewEl.attribute( "Type", "Unknown");
    // create the view
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
  if (pView && (viewType == QString("KWriteView"))) {
    // read the cursor position of current view
    int line = docEl.attribute( "CursorPosLine", "-1").toInt();
    int col  = docEl.attribute( "CursorPosCol", "0").toInt();
    if (pDoc && (line != -1)) {
      KWrite* pKWrite = ((KWriteDoc*)pDoc)->getKWrite();
      if (pKWrite) {
         pKWrite->setCursorPosition(line, col);
      }
    }
  }

  // restore focus
  if (pFocusedView != 0L) {
    if (pFocusedView->parentWidget()->inherits("QextMdiChildView")) {
        ((QextMdiChildView*)pFocusedView->parentWidget())->activate();
    }
    pFocusedView->setFocus();
  }

}

//-------------- Falk's Views saving ---------//
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
  QextMdi::MdiMode mdiMode = ((QextMdiMainFrm*)m_pDocViewMan->parent())->mdiMode();
  viewEl.setAttribute( "Attach", pMDICover->isAttached() || (mdiMode == QextMdi::TabPageMode));
}

//-------------- Falk's Views restoring ---------//
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
  QextMdi::MdiMode mdiMode = ((QextMdiMainFrm*)m_pDocViewMan->parent())->mdiMode();
  if ((mdiMode != QextMdi::TabPageMode) && (mdiMode != QextMdi::ToplevelMode)) {
    if ((!pMDICover->isAttached()) && (bAttached) ) {
      pMDICover->attach();
    }
    if ( (pMDICover->isAttached()) && (!bAttached) ) {
      pMDICover->detach();
    }
  }
  if (nMinMaxMode == 0) {
    pMDICover->setInternalGeometry(QRect(nLeft, nTop, nWidth, nHeight));
  }
  else {
    if (nMinMaxMode == 1) {
      pMDICover->minimize();
    }
    if (nMinMaxMode == 2) {
      // maximize: nothing to do, this is already under control of the mainframe
    }
    pMDICover->setRestoreGeometry(QRect(nLeft, nTop, nWidth, nHeight));
  }
}
//-------------- restoring from the session file ---------//
//------------------------------------------------------------------------------
bool KDevSession::restoreFromFile(const QString& sessionFileName)
{
//  QDomDocument pInFile( "KDevPrjSession");
  bool bFileOpenOK = true;

  // Write it out to a tmp file
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
//------------------------------------------------------------------------------
//------ saving to session file, here saving of the complete tree required ---//
//------------------------------------------------------------------------------
bool KDevSession::saveToFile(const QString& sessionFileName)
{

  QString section, keyword;
  QDomElement session = domdoc.documentElement();


  int nDocs = 0;
  QString docIdStr;

  // read the information about the mainframe widget
  QDomElement mainframeEl = session.namedItem("Mainframe").toElement();
  if(mainframeEl.isNull()){
  	mainframeEl=domdoc.createElement("Mainframe");
  	session.appendChild( mainframeEl);
	}
  bool bMaxMode = ((QextMdiMainFrm*)m_pDocViewMan->parent())->isInMaximizedChildFrmMode();
  mainframeEl.setAttribute("MaximizeMode", bMaxMode);


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

  if (m_pDocViewMan->docCount() > 0) {
    QList<KWriteDoc> kWriteDocList = m_pDocViewMan->getKWriteDocList();
    for (kWriteDocList.first(); kWriteDocList.current() != 0; kWriteDocList.next()) {
      KWriteDoc* pDoc = kWriteDocList.current();
      if (!pDoc->fileName()) {
        // TODO: ask for filename
      }
      docIdStr.setNum(nDocs);
      QDomElement docEl = domdoc.createElement("Doc" + docIdStr);
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
      KWriteView* pView = 0L;
      QString viewIdStr;
      for (viewList.first(), nView = 0; viewList.current() != 0; viewList.next(), nView++) {
        pView = viewList.current();
        if (pView != 0L) {
          viewIdStr.setNum( nView);
          QDomElement viewEl = domdoc.createElement( "View"+viewIdStr);
          docEl.appendChild( viewEl);
          // focus?
          viewEl.setAttribute("Focus", (((CEditWidget*)pView->parentWidget()) == m_pDocViewMan->currentEditView()));
          viewEl.setAttribute("Type", "KWriteView");
          // save geometry of current view
          saveViewGeometry( pView->parentWidget(), viewEl);
        }
      }
      // save cursor position of current view
      if (pView) {
        QPoint cursorPos(pView->cursorPosition());
        docEl.setAttribute("CursorPosCol", cursorPos.x());
        docEl.setAttribute("CursorPosLine", cursorPos.y());
      }
    }
    QList<CDocBrowser> kDocBrowserList = m_pDocViewMan->getDocBrowserList();
    for (kDocBrowserList.first(); kDocBrowserList.current() != 0; kDocBrowserList.next()) {
      CDocBrowser* pDoc = kDocBrowserList.current();
      if (!pDoc->currentURL()) {
        // TODO: ask for filename
      }
      docIdStr.setNum(nDocs);
      QDomElement docEl = domdoc.createElement("Doc" + docIdStr);
      QString fname = pDoc->currentURL();
      if (fname.left(5) == QString("file:")) {
        fname = fname.right(fname.length()-5); // store without 'file:'
      }
      docEl.setAttribute( "FileName", fname);
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
      QDomElement viewEl = domdoc.createElement( "View");
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
    t << domdoc.toCString();
    f.close();
  }
	initXMLTree();  // clear and initialize the tree again

  return true;
}

//#include "kdevsession.moc"
