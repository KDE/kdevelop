/***************************************************************************
                          editormgr.cpp  -  description
                             -------------------
    copyright            : (C) 2000 by KDevelop team
    email                : kdevelop_team@kdevelop.org

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


// application specific includes
#include "editormgr.h"

#include "editorconfigwidget.h"
#include "editorfactory.h"
#include "editorview.h"

//#include "kdevevent.h"

#include <kdebug.h>
#include <kaction.h>
#include <kfiledialog.h>
#include <klibloader.h>
#include <klocale.h>
#include <kservice.h>
#include <kstdaction.h>
#include <ktrader.h>
#include <ktexteditor.h>
#include <kdialogbase.h>

#include <qvbox.h>
#include <qlist.h>

#include <assert.h>

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

//EditorDock::EditorDock( QWidget *parent, const char *name ) :
//  QWidget(parent, name)
//{
//}
//
///****************************************************************************/
//
//EditorDock::~EditorDock()
//{
//}

/****************************************************************************/

/** Constructor for the fileclass of the application */
EditorManager::EditorManager(QObject* parent, const char* name) :
  KDevComponent(parent, name)
{
  assert(parent && parent->isWidgetType());

  setInstance(EditorFactory::instance());
  setXMLFile( "kdeveditorpartui.rc" );

  // Get the factory that spits out the right ktexteditor parts
  // TODO: just take the first one at the moment.
  KTrader::OfferList offers = KTrader::self()->query( "KIDETextEditor/Document" );
  assert( offers.count() >= 1 );

  KTrader::OfferList::ConstIterator it;
  for (it = offers.begin(); it != offers.end(); ++it) {
    kdDebug(9005) << "KTextEditor service found : " << (*it)->name() << endl;
  }

  KService::Ptr service = *offers.begin();
  kdDebug(9005) << "KTextEditor service choosen : " << service->name() << endl;

  m_factory = KLibLoader::self()->factory( service->library() );
  assert( m_factory );
}

/****************************************************************************/

/** Destructor for the fileclass of the application */
EditorManager::~EditorManager()
{
}

/****************************************************************************/

void EditorManager::configWidgetRequested(KDialogBase *dlg)
{
  QVBox *vbox = dlg->addVBoxPage(i18n("Editor"));
  (void) new EditorConfigWidget(vbox, "editor config widget");
}

/****************************************************************************/

void EditorManager::setupGUI()
{
  // file menu actions
  KStdAction::openNew   (this, SLOT(slotFileNew()),       actionCollection());
  KStdAction::open      (this, SLOT(slotFileOpen()),      actionCollection());
//  KStdAction::quit      (this, SLOT(quit()),              actionCollection);

  // edit menu actions
  KStdAction::undo      (this, SLOT(slotEditUndo()),      actionCollection());
  KStdAction::redo      (this, SLOT(slotEditRedo()),      actionCollection());
  KStdAction::cut       (this, SLOT(slotEditCut()),       actionCollection());
  KStdAction::copy      (this, SLOT(slotEditCopy()),      actionCollection());
  KStdAction::paste     (this, SLOT(slotEditPaste()),     actionCollection());
  KStdAction::find      (this, SLOT(slotEditFind()),      actionCollection());
  KStdAction::findNext  (this, SLOT(slotEditFindNext()),  actionCollection());
  KStdAction::replace   (this, SLOT(slotEditReplace()),   actionCollection());

//  emit embedWidget(new EditorDock((QWidget*)parent(), ""), "EditorDock");
}

/****************************************************************************/

#ifdef removed
void EditorManager::slotEventHandler(KDevEvent* event)
{
  assert(event);
  switch (event->type())
  {
    case KDevEvent::GoToFile:
    {
      KDevGoToFileEvent* gotoFileEvent = static_cast<KDevGoToFileEvent*>(event);
      gotoFile(gotoFileEvent->url(), gotoFileEvent->lineNum());
      break;
    }

    default:
      break;
  }
}
#endif

/****************************************************************************/

void EditorManager::gotoFile(const KURL& url, int lineNum)
{
  KTextEditor::Document *document = findOpenDocument(url);
  if (!document)
    document = openDocument(url);

  if (document)
  {
    QListIterator<KTextEditor::View> it(document->views());
    for ( ; it.current(); ++it )
    {
      KTextEditor::View* view = it.current();
      kdDebug(9005) << "goto line: " << lineNum << endl;
      view->setCursorPosition (lineNum, 0);
    }
  }
}

/****************************************************************************/

/** initializes the document generally */
KTextEditor::Document* EditorManager::newDocument()
{
  KTextEditor::Document *document = static_cast<KTextEditor::Document *>(
                      m_factory->create( (QWidget*)parent(), 0, "KIDETextEditor::Document" ) );
  assert( document );
  document->setText("");
  m_documents.append(document);

  EditorView* view = new EditorView(document, (QWidget*)parent(), 0);
  emit embedWidget(view, DocumentView, "", i18n("Editor view"));
  return document;
}

/****************************************************************************/

/** loads the document by filename and format and emits the updateViews() signal */
KTextEditor::Document* EditorManager::openDocument(const KURL& url)
{
  if (!documentExists(url))
    return 0;

  KTextEditor::Document *document = static_cast<KTextEditor::Document *>(
                            m_factory->create( (QWidget*)parent(), 0, "KIDETextEditor::Document" ) );
  assert( document );
  m_documents.append(document);
  document->openURL(url);

  EditorView* view = new EditorView(document, (QWidget*)parent(), url.path());
  emit embedWidget(view, DocumentView, url.path(), i18n("Editor view"));
  return document;
}

/****************************************************************************/

bool EditorManager::documentExists(const KURL& url)
{
  return true;
}

/****************************************************************************/

bool EditorManager::documentIsWritable(const KURL& url)
{
  return true;
}

/****************************************************************************/

void EditorManager::slotFileOpen()
{
  QString fileToOpen=KFileDialog::getOpenFileName(QDir::homeDirPath(),
                        i18n("*|All files"), (QWidget*)parent(), i18n("Open File..."));

  if(!fileToOpen.isEmpty())
    openDocument(fileToOpen);
}

/****************************************************************************/

KTextEditor::Document* EditorManager::findOpenDocument(const KURL& url)
{
  QListIterator<KTextEditor::Document> it(m_documents);
  for ( ; it.current(); ++it )
  {
    KTextEditor::Document* document = it.current();
    if (document->url() == url)
      return document;
  }
  return 0;
}

/****************************************************************************/

/** closes the actual document */
void EditorManager::closeDocument(const KURL& url)
{
}

/****************************************************************************/

/** saves the document under filename and format.*/
bool EditorManager::saveDocument(const KURL& url)
{
  return true;
}

/****************************************************************************/

void EditorManager::slotFileNew()
{
  newDocument();
}

void EditorManager::slotFileSave()
{
//  slotStatusMsg(i18n("Saving file..."));
//  slotStatusMsg(i18n("Ready."));
}

void EditorManager::slotFileSaveAs()
{
//  slotStatusMsg(i18n("Saving file with a new filename..."));
//  slotStatusMsg(i18n("Ready."));
}

void EditorManager::slotFileClose()
{
//  slotStatusMsg(i18n("Closing file..."));
//  slotStatusMsg(i18n("Ready."));
}

void EditorManager::slotFilePrint()
{
//  slotStatusMsg(i18n("Printing..."));
//  slotStatusMsg(i18n("Ready."));
}

void EditorManager::slotFileQuit()
{
//  slotStatusMsg(i18n("Exiting..."));
//  saveOptions();
  // close the first window, the list makes the next one the first again.
  // This ensures that queryClose() is called on each window to ask for closing
//  KTMainWindow* w;
//  if(memberList)
//  {
//    for(w=memberList->first(); w!=0; w=memberList->first())
//    {
      // only close the window if the closeEvent is accepted. If the user presses Cancel on the saveModified() dialog,
      // the window and the application stay open.
//      if(!w->close())
//        break;
//    }
//  }
//  slotStatusMsg(i18n("Ready."));
}

void EditorManager::slotEditUndo()
{
//  slotStatusMsg(i18n("Reverting last action..."));
//  slotStatusMsg(i18n("Ready."));
}

void EditorManager::slotEditCut()
{
//  slotStatusMsg(i18n("Cutting selection..."));
//  slotStatusMsg(i18n("Ready."));
}

void EditorManager::slotEditCopy()
{
//  slotStatusMsg(i18n("Copying selection to clipboard..."));
//  slotStatusMsg(i18n("Ready."));
}

void EditorManager::slotEditPaste()
{
//  slotStatusMsg(i18n("Inserting clipboard contents..."));
//  slotStatusMsg(i18n("Ready."));
}

void EditorManager::slotEditRedo()
{
//  slotStatusMsg(i18n("Inserting clipboard contents..."));
//  slotStatusMsg(i18n("Ready."));
}

void EditorManager::slotEditFind()
{
//  slotStatusMsg(i18n("Inserting clipboard contents..."));
//  slotStatusMsg(i18n("Ready."));
}
void EditorManager::slotEditFindNext()
{
//  slotStatusMsg(i18n("Inserting clipboard contents..."));
//  slotStatusMsg(i18n("Ready."));
}
void EditorManager::slotEditReplace()
{
//  slotStatusMsg(i18n("Inserting clipboard contents..."));
//  slotStatusMsg(i18n("Ready."));
}

#include "editormgr.moc"
