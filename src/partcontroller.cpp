#include <qpopupmenu.h>


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


#include "toplevel.h"
#include "core.h"
#include "editorproxy.h"
#include "documentationpart.h"

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

  m_saveAllFilesAction = new KAction(i18n("Save &All"), 0,
    this, SLOT(slotSaveAllFiles()),
    ac, "file_save_all");
  m_saveAllFilesAction->setEnabled(false);

  m_revertAllFilesAction = new KAction(i18n("&Revert All"), 0,
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

  m_closeOtherWindowsAction = new KAction(i18n("Close All &Others"), 0,
    this, SLOT(slotCloseOtherWindows()),
    ac, "file_closeother");
  m_closeOtherWindowsAction->setEnabled(false);
}


void PartController::setEncoding(const QString &encoding)
{
  m_presetEncoding = encoding;
}


void PartController::editDocument(const KURL &url, int lineNum)
{
  KParts::Part *existingPart = partForURL(url);
  if (existingPart)
  {
    activatePart(existingPart);
    EditorProxy::getInstance()->setLineNumber(existingPart, lineNum);
    return;
  }

  QString preferred, serviceType;

  QString mimeType, encoding;
  if (m_presetEncoding.isNull())
    mimeType = KMimeType::findByURL(url)->name();
  else {
    mimeType = "text/plain";
    encoding = m_presetEncoding;
    m_presetEncoding = QString::null;
  }
  
  if (mimeType.startsWith("text/")
      || mimeType == "application/x-desktop")
  {
    mimeType = "text/plain";
    kapp->config()->setGroup("Editor");
    preferred = kapp->config()->readEntry("EmbeddedKTextEditor", "");
  }

  KParts::Factory *factory = 0;

  QString services[] = {"KTextEditor/Document", "KParts/ReadWritePart", "KParts/ReadOnlyPart"};
  for (uint i=0; i<3; ++i)
  {
    factory = findPartFactory(mimeType, services[i], preferred);
    if (factory)
    {
      serviceType = services[i];
      break;
    }
  }

  if (factory)
  {
    KParts::ReadOnlyPart *part = static_cast<KParts::ReadOnlyPart*>(factory->createPart(TopLevel::getInstance()->main(), serviceType));
    KParts::BrowserExtension *extension = KParts::BrowserExtension::childObject(part);
    kdDebug() << "Encoding: " << encoding << ", extension: " << extension << endl;
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

  part->openURL(url);
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
  kdDebug(9000) << "Preferred Name: " << preferredName << endl;
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
    return static_cast<KParts::Factory*>(KLibLoader::self()->factory(ptr->library().latin1()));
  }

  return 0;
}


void PartController::integratePart(KParts::Part *part, const KURL &url)
{
  TopLevel::getInstance()->embedPartView(part->widget(), url.fileName());

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
  // TODO: for local files, check inode identity!
  return a == b;
}

KParts::Part *PartController::partForURL(const KURL &url)
{
  QPtrListIterator<KParts::Part> it(*parts());
  for ( ; it.current(); ++it)
  {
    KParts::ReadOnlyPart *ro_part = dynamic_cast<KParts::ReadOnlyPart*>(it.current());
    if (urlIsEqual(url, ro_part->url()))
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


void PartController::saveAllFiles()
{
  QPtrListIterator<KParts::Part> it(*parts());
  for ( ; it.current(); ++it)
    if (it.current()->inherits("KParts::ReadWritePart"))
    {
      KParts::ReadWritePart *rw_part = static_cast<KParts::ReadWritePart*>(it.current());
      rw_part->save();

      TopLevel::getInstance()->statusBar()->message(i18n("Saved %1").arg(rw_part->url().prettyURL()), 2000);
    }
}


void PartController::slotRevertAllFiles()
{
  revertAllFiles();
}


void PartController::revertAllFiles()
{
  QPtrListIterator<KParts::Part> it(*parts());
  for ( ; it.current(); ++it)
    if (it.current()->inherits("KParts::ReadWritePart"))
    {
      KParts::ReadWritePart *rw_part = static_cast<KParts::ReadWritePart*>(it.current());
      rw_part->openURL(rw_part->url());
    }
}


void PartController::slotCloseWindow()
{
  closeActivePart();
}


void PartController::slotCloseAllWindows()
{
  while (parts()->count() > 0)
    closePart(parts()->getFirst());
}


void PartController::slotCloseOtherWindows()
{
  if (!activePart())
    return;

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
  QStringList fileNames = KFileDialog::getOpenFileNames(QString::null, "*.*", TopLevel::getInstance()->main(), i18n("Open File"));

  for ( QStringList::Iterator it = fileNames.begin(); it != fileNames.end(); ++it ) {
    editDocument(KURL(*it));
  }
}


bool PartController::closeDocuments(const QStringList &documents)
{
  QStringList::ConstIterator it;
  for (it=documents.begin(); it != documents.end(); ++it)
  {
    KParts::Part *part = partForURL(KURL(*it));

    if (!part)
      continue;
    
    if(!closePart(part))
      return false;
  }

  return true;
}

bool PartController::closePartForWidget( const QWidget* w )
{
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
