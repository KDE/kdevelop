#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>


#include <qpopupmenu.h>
#include <qfile.h>


#include <kmimetype.h>
#include <kservice.h>
#include <ktrader.h>
#include <kapp.h>
#include <krun.h>
#include <kdebug.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kparts/part.h>
#include <kparts/factory.h>
#include <kparts/partmanager.h>
#include <kparts/browserextension.h>
#include <kfiledialog.h>
#include <kmainwindow.h>
#include <kaction.h>
#include <kstatusbar.h>
#include <khtml_part.h>
#include <kio/netaccess.h>

#include "toplevel.h"
#include "api.h"
#include "core.h"
#include "editorproxy.h"
#include "documentationpart.h"
#include "ksavealldialog.h"

#include "kdevproject.h"

#include "partcontroller.h"


PartController *PartController::s_instance = 0;


PartController::PartController(QWidget *parent)
  : KDevPartController(parent)
{
  connect(this, SIGNAL(partRemoved(KParts::Part*)), this, SLOT(updateMenuItems()));
  connect(this, SIGNAL(partAdded(KParts::Part*)), this, SLOT(updateMenuItems()));
  connect(this, SIGNAL(activePartChanged(KParts::Part*)), this, SLOT(updateMenuItems()));

  setupActions();
}


PartController::~PartController()
{
}


void PartController::createInstance(QWidget *parent)
{
  if (!s_instance)
    s_instance = new PartController(parent);
}


PartController *PartController::getInstance()
{
  return s_instance;
}


void PartController::setupActions()
{
  KActionCollection *ac = TopLevel::getInstance()->main()->actionCollection();

  (void) KStdAction::open(this, SLOT(slotOpenFile()),
    ac, "file_open");

  m_openRecentAction = KStdAction::openRecent( this, SLOT(slotOpenRecent(const KURL&) ),
    ac, "file_open_recent" );
  m_openRecentAction->loadEntries( kapp->config(), "RecentFiles" );

  m_saveAllFilesAction = new KAction(i18n("Save Al&l"), 0,
    this, SLOT(slotSaveAllFiles()),
    ac, "file_save_all");
  m_saveAllFilesAction->setEnabled(false);

  m_revertAllFilesAction = new KAction(i18n("Rever&t All"), 0,
    this, SLOT(slotRevertAllFiles()),
    ac, "file_revert_all");
  m_revertAllFilesAction->setEnabled(false);

  m_closeWindowAction = KStdAction::close(
    this, SLOT(slotCloseWindow()),
    ac, "file_close");
  m_closeWindowAction->setEnabled(false);

  m_closeAllWindowsAction = new KAction(i18n("Close All"), 0,
    this, SLOT(slotCloseAllWindows()),
    ac, "file_close_all");
  m_closeAllWindowsAction->setEnabled(false);

  m_closeOtherWindowsAction = new KAction(i18n("Close All Others"), 0,
    this, SLOT(slotCloseOtherWindows()),
    ac, "file_closeother");
  m_closeOtherWindowsAction->setEnabled(false);
}


void PartController::setEncoding(const QString &encoding)
{
  m_presetEncoding = encoding;
}


void PartController::editDocument(const KURL &inputUrl, int lineNum)
{
  KURL url = inputUrl;

  // Make sure the URL exists
  if (!url.isValid() || !KIO::NetAccess::exists(url)) {
    // Try to find this file in the current project's list instead
    KDevProject* project = API::getInstance()->project();

    if (project) {
      QStringList fileList = project->allFiles();
      
      for (QStringList::Iterator it = fileList.begin(); it != fileList.end(); ++it) {
        if ((*it).endsWith(url.url())) {
          // Match! The first one is as good as any one, I guess...
          url = project->projectDirectory() + "/" + *it;
          break;
        } 
      }
    }
    
    if (!url.isValid() || !KIO::NetAccess::exists(url)) {
      // See if this url is relative to the current project's directory
      url = project->projectDirectory() + "/" + url.url();
    }
    
    if (!url.isValid() || !KIO::NetAccess::exists(url)) {
      // Here perhaps we should prompt the user to find the file?
      return;
    }
  }
  
  url.cleanPath(true);
    
  KParts::Part *existingPart = partForURL(url);
  if (existingPart)
  {
    activatePart(existingPart);
    EditorProxy::getInstance()->setLineNumber(existingPart, lineNum);
    return;
  }

  QString preferred, className;

  QString mimeType, encoding;
  if (m_presetEncoding.isNull())
    mimeType = KMimeType::findByURL(url)->name();
  else {
    mimeType = "text/plain";
    encoding = m_presetEncoding;
    m_presetEncoding = QString::null;
  }

  if (mimeType.startsWith("text/")
      || mimeType == "application/x-desktop"
      || mimeType == "application/x-kdevelop" || mimeType == "application/x-kdevelop-project"
      || mimeType == "image/x-xpm"
      || mimeType == "application/x-perl")
  {
    mimeType = "text/plain";
    kapp->config()->setGroup("Editor");
    preferred = kapp->config()->readEntry("EmbeddedKTextEditor", "");
  }

  KParts::Factory *factory = 0;

  // load the appropriate part factory like chosen in the editor-chooser part
  // (Note: KTextEditor/Document is the editor in MDI mode. KTextEditor/Editor is the editor in SDI mode.
  //        But KTextEditor/Editor doesn't work for the Kate part on KDE-3.0.x, so better use KTextEditor/Document anyway.)
  QString services[] = {"KTextEditor/Document", "KParts/ReadWritePart", "KParts/ReadOnlyPart"};
  QString classnames[] = {"KTextEditor::Document", "KParts::ReadWritePart", "KParts::ReadOnlyPart"};
  for (uint i=0; i<3; ++i)
  {
    factory = findPartFactory(mimeType, services[i], preferred);
    if (factory)
    {
      className = classnames[i];
      break;
    }
  }

  if (factory)
  {
    // Currently, only a single view per document is supported.
    // So fall back (downgrade) from MDI-mode editor to SDI-mode editor
    // (Note: This always works since KTextEditor::Document inherits KTextEditor::Editor)
    if (className == "KTextEditor::Document") className = "KTextEditor::Editor";
    // create the object of the desired class
    KParts::ReadOnlyPart *part = static_cast<KParts::ReadOnlyPart*>(factory->createPart(TopLevel::getInstance()->main(), 0, 0, 0, className));
    KParts::BrowserExtension *extension = KParts::BrowserExtension::childObject(part);
    kdDebug(9000) << "Encoding: " << encoding << ", extension: " << extension << endl;
    if (extension && !encoding.isNull())
    {
      KParts::URLArgs args;
      args.serviceType = mimeType + ";" + encoding;
      extension->setURLArgs(args);
    }
    part->openURL(url);
    integratePart(part, url);
    EditorProxy::getInstance()->setLineNumber(part, lineNum);
  }
  else
    KRun::runURL(url, mimeType);
}


void PartController::showDocument(const KURL &url, const QString &context)
{
kdDebug(9000) << "SHOW: " << url.url() << " context=" << context << endl;

  DocumentationPart *part = 0;

  if (!context.isEmpty())
    part = findDocPart(context);

  if (!part)
  {
    part = new DocumentationPart;
    part->setContext(context);
    integratePart(part,url);
  }
  else
    activatePart(part);

  bool bSuccess = part->openURL(url);
  if (!bSuccess) {
    // part->showError(...);
  }
}


DocumentationPart *PartController::findDocPart(const QString &context)
{
  QPtrListIterator<KParts::Part> it(*parts());
  for ( ; it.current(); ++it)
  {
    DocumentationPart *part = dynamic_cast<DocumentationPart*>(it.current());
    if (part && (part->context() == context))
      return part;
  }

  return 0;
}


KParts::Factory *PartController::findPartFactory(const QString &mimeType, const QString &partType, const QString &preferredName)
{
  KTrader::OfferList offers = KTrader::self()->query(mimeType, QString("'%1' in ServiceTypes").arg(partType));

  if (offers.count() > 0)
  {
    KService::Ptr ptr = 0;
    // if there is a preferred plugin we'll take it
    if ( !preferredName.isEmpty() ) {
      KTrader::OfferList::Iterator it;
      for (it = offers.begin(); it != offers.end(); ++it) {
        if ((*it)->name() == preferredName) {
          ptr = (*it);
        }
      }
    }
    // else we just take the first in the list
    if ( !ptr ) {
      ptr = offers.first();
    }
    return static_cast<KParts::Factory*>(KLibLoader::self()->factory(QFile::encodeName(ptr->library())));
  }

  return 0;
}


void PartController::integratePart(KParts::Part *part, const KURL &url)
{
  if (!part->widget()) {
    // TODO error handling
    return; // to avoid later crash
  }

  TopLevel::getInstance()->embedPartView(part->widget(), url.fileName(), url.url());

  addPart(part);

  EditorProxy::getInstance()->installPopup(part, contextPopupMenu());

  // HACK: this is a workaround. The kate-part does not emit "completed" when
  // it save a file yet.
  // FIXME: remove this line once kate-part emits the right signal
  connect(part, SIGNAL(fileNameChanged()), this, SLOT(slotUploadFinished()));

  // tell the parts we loaded a document
  KParts::ReadOnlyPart *ro_part = dynamic_cast<KParts::ReadOnlyPart*>(part);
  if (ro_part && ro_part->url().isLocalFile())
    emit loadedFile(ro_part->url().path());

  // let's get notified when a document has been changed
  connect(part, SIGNAL(completed()), this, SLOT(slotUploadFinished()));
}


void PartController::slotUploadFinished()
{
  const KParts::ReadWritePart *rw_part = dynamic_cast<const KParts::ReadWritePart*>(sender());

  if (rw_part && rw_part->url().isLocalFile())
      emit savedFile(rw_part->url().path());
}


QPopupMenu *PartController::contextPopupMenu()
{
  static QPopupMenu *popup = 0;

  if (!popup)
    popup = (QPopupMenu*)(TopLevel::getInstance()->main())->factory()->container("rb_popup", TopLevel::getInstance()->main());

  return popup;
}


static bool urlIsEqual(const KURL &a, const KURL &b)
{
  if (a.isLocalFile() && b.isLocalFile())
  {
    struct stat aStat, bStat;

    if ((::stat(QFile::encodeName(a.fileName()), &aStat) == 0)
        && (::stat(QFile::encodeName(b.fileName()), &bStat) == 0))
    {
      return (aStat.st_dev == bStat.st_dev) && (aStat.st_ino == bStat.st_ino);
    }
  }

  return a == b;
}

KParts::Part *PartController::partForURL(const KURL &url)
{
  QPtrListIterator<KParts::Part> it(*parts());
  for ( ; it.current(); ++it)
  {
    KParts::ReadOnlyPart *ro_part = dynamic_cast<KParts::ReadOnlyPart*>(it.current());
    if (ro_part && urlIsEqual(url, ro_part->url()))
      return ro_part;
  }

  return 0;
}


void PartController::activatePart(KParts::Part *part)
{
  setActivePart(part);

  if (part->widget())
  {
    TopLevel::getInstance()->raiseView(part->widget());
    part->widget()->setFocus();
  }
}


void PartController::closeActivePart()
{
  if (!activePart())
    return;

  closePart(activePart());
}


bool PartController::closePart(KParts::Part *part)
{
  if (part->inherits("KParts::ReadOnlyPart"))
  {
    KParts::ReadOnlyPart *ro_part = static_cast<KParts::ReadOnlyPart*>(part);

    if (!ro_part->closeURL())
    {
      return false;
    }
  }

  // If we didn't call removePart(), KParts::PartManager::slotObjectDestroyed would
  // get called from the destroyed signal of the part being deleted below.
  // The call chain from that looks like this:
  // QObject::destroyed()
  //   KParts::PartManager::slotWidgetDestroyed() -> setActivePart() -> activePartChanged()
  //     TopLevelXXX::createGUI()
  //       KXMLGUIFactory::removeClient()
  // But then KXMLGUIFactory tries to remove the already-deleted part.
  // Normally this would work, because the factory uses a QGuardedPtr to the part.
  // But the QGuardedPtr is connected to the _same_ destroyed() slot that got us to
  // that point (slots are called in an undefined order)!

  // The following line can be removed with kdelibs HEAD! (2002-05-26)
  removePart( part );

  if (part->widget())
    TopLevel::getInstance()->removeView(part->widget());

  delete part;

  return true;
}


void PartController::updateMenuItems()
{
  bool hasWriteParts = false;
  bool hasReadOnlyParts = false;

  QPtrListIterator<KParts::Part> it(*parts());
  for ( ; it.current(); ++it)
  {
    if (it.current()->inherits("KParts::ReadWritePart"))
      hasWriteParts = true;
    if (it.current()->inherits("KParts::ReadOnlyPart"))
      hasReadOnlyParts = true;
  }

  m_saveAllFilesAction->setEnabled(hasWriteParts);
  m_revertAllFilesAction->setEnabled(hasWriteParts);
  m_closeWindowAction->setEnabled(hasReadOnlyParts);
  m_closeAllWindowsAction->setEnabled(hasReadOnlyParts);
  m_closeOtherWindowsAction->setEnabled(hasReadOnlyParts);
}


void PartController::slotSaveAllFiles()
{
  saveAllFiles();
}


void PartController::saveFile(KParts::Part *part)
{
  if ( part && part->inherits("KParts::ReadWritePart") ) {
    KParts::ReadWritePart *rw_part = static_cast<KParts::ReadWritePart*>(part);
    if( rw_part->isModified() ){
      rw_part->save();
      TopLevel::getInstance()->statusBar()->message(i18n("Saved %1").arg(rw_part->url().prettyURL()), 2000);
    }
  }
}

void PartController::saveAllFiles()
{
  QPtrListIterator<KParts::Part> it(*parts());
  for ( ; it.current(); ++it)
    saveFile( it.current() );
}


void PartController::slotRevertAllFiles()
{
  revertAllFiles();
}

void PartController::revertFile(KParts::Part *part)
{
  if ( !part )
    return;

  if (part->inherits("KParts::ReadWritePart")) {
      KParts::ReadWritePart *rw_part = static_cast<KParts::ReadWritePart*>(part);
      rw_part->openURL(rw_part->url());
    }
}

void PartController::revertAllFiles()
{
  QPtrListIterator<KParts::Part> it(*parts());
  for ( ; it.current(); ++it)
    revertFile( it.current() );
}


void PartController::slotCloseWindow()
{
  closeActivePart();
}


QStringList PartController::getModifiedDocuments( KParts::Part* excludeMe )
{
  QStringList modFiles;
  QPtrListIterator<KParts::Part> it(*parts());
  for ( ; it.current(); ++it) {
    if (it.current()->inherits("KParts::ReadWritePart")) {
      KParts::ReadWritePart *rw_part = static_cast<KParts::ReadWritePart*>(it.current());
      if ( rw_part->isModified() && rw_part != excludeMe )
        modFiles << rw_part->url().url();
    }
  }
  return modFiles;
}

void PartController::slotCloseAllWindows()
{
  QStringList modFiles = getModifiedDocuments();
  if ( modFiles.count() > 1 ) {
    KSaveAllDialog* sad = new KSaveAllDialog( modFiles, TopLevel::getInstance()->main() );
    sad->exec();
    if ( sad->result() == KSaveAllDialog::Cancel )
      return;
    else if ( sad->result() == KSaveAllDialog::Revert )
      revertAllFiles();
    else if ( sad->result() == KSaveAllDialog::SaveAll )
      saveAllFiles();
  }

  while (parts()->count() > 0)
    closePart(parts()->getFirst());
}


void PartController::slotCloseOtherWindows()
{
  if (!activePart())
    return;

  QStringList modFiles = getModifiedDocuments( activePart() );
  if ( modFiles.count() > 1 ) {
    KSaveAllDialog* sad = new KSaveAllDialog( modFiles, TopLevel::getInstance()->main() );
    sad->exec();
    if ( sad->result() == KSaveAllDialog::Cancel )
      return;
    else if ( sad->result() == KSaveAllDialog::Revert )
      revertAllFiles();
    else if ( sad->result() == KSaveAllDialog::SaveAll )
      saveAllFiles();
  }

  while (parts()->count() > 1)
  {
    QPtrListIterator<KParts::Part> it(*parts());
    for ( ; it.current(); ++it)
      if (it.current() != activePart())
        closePart(it.current());
  }
}


void PartController::slotCurrentChanged(QWidget *w)
{
  QPtrListIterator<KParts::Part> it(*parts());
  for ( ; it.current(); ++it)
    if (it.current()->widget() == w)
    {
      setActivePart(it.current(), w);
      break;
    }
}


void PartController::slotOpenFile()
{
  QStringList fileNames = KFileDialog::getOpenFileNames(QString::null, QString::null, TopLevel::getInstance()->main(), QString::null);

  for ( QStringList::Iterator it = fileNames.begin(); it != fileNames.end(); ++it ) {
    editDocument(KURL(*it));
    m_openRecentAction->addURL( KURL(*it) );
  }
  m_openRecentAction->saveEntries( kapp->config(), "RecentFiles" );
}

void PartController::slotOpenRecent( const KURL& url )
{
  editDocument( url );
}

bool PartController::closeDocuments(const QStringList &documents)
{
  QStringList::ConstIterator it;
  KSaveAllDialog::SaveAllResult result = KSaveAllDialog::Cancel;

  QStringList modFiles;
  for (it=documents.begin(); it != documents.end(); ++it) {
    KParts::Part *part = partForURL(KURL(*it));
    if ( part && part->inherits("KParts::ReadWritePart") && ((KParts::ReadWritePart*)part)->isModified() )
      modFiles << (*it);
  }

  if ( modFiles.count() > 1 ) {
    KSaveAllDialog* sad = new KSaveAllDialog( modFiles, TopLevel::getInstance()->main() );
    sad->exec();
    if ( sad->result() == KSaveAllDialog::Cancel )
      return false;
    else if ( sad->result() == KSaveAllDialog::Revert )
      result = KSaveAllDialog::Revert;
    else if ( sad->result() == KSaveAllDialog::SaveAll )
      result = KSaveAllDialog::SaveAll;
  }

  for (it=documents.begin(); it != documents.end(); ++it)
  {
    KParts::Part *part = partForURL(KURL(*it));

    if (!part)
      continue;

    if ( result == KSaveAllDialog::SaveAll ) {
      saveFile( part );
    } else if ( result == KSaveAllDialog::Revert ) {
      revertFile( part );
    }

    if(!closePart(part))
      return false;
  }

  return true;
}

void PartController::slotClosePartForWidget( const QWidget* w) 
{
  closePartForWidget(w);
}

void PartController::slotCloseAllButPartForWidget(QWidget* w)
{
  slotCurrentChanged(w);
  slotCloseOtherWindows();  
}

bool PartController::closePartForWidget( const QWidget* w )
{
  qDebug("in close part for widget");
  QPtrListIterator<KParts::Part> it(*parts());
  for ( ; it.current(); ++it)
    if (it.current()->widget() == w)
    {
      return closePart( *it );
    }
  return true;
}

bool PartController::readyToClose()
{
  QPtrListIterator<KParts::Part> it(*parts());
  for ( ; it.current(); ++it)
  {
    if(!closePart(*it))
      return false;
  }

  return true;
}


#include "partcontroller.moc"
