#include <qlayout.h>
#include <qtabwidget.h>


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
#include <kfiledialog.h>

#include <ktexteditor/viewcursorinterface.h>


#include "keditor/editor.h"
#include "keditor/cursor_iface.h"
#include "toplevel.h"
#include "core.h"


#include "partcontroller.h"


class PartListEntry
{
public:

  PartListEntry(KParts::Part *part, const KURL &url);

  const KURL &url() const { return m_url; };
  KParts::Part *part() const { return m_part; };

  bool isEqual(const KURL &url);


private:

  KURL         m_url;
  KParts::Part *m_part;

};


PartListEntry::PartListEntry(KParts::Part *part, const KURL &url)
  : m_url(url), m_part(part)
{
}


bool PartListEntry::isEqual(const KURL &url)
{
  // TODO: for local files, check inode identity!
  return m_url == url;
}


PartController *PartController::s_instance = 0;


PartController::PartController(QWidget *parent, QWidget *mainwindow, const char *name)
  : KDevPartController(parent, name)
{
  QVBoxLayout *vbox = new QVBoxLayout(this);

  m_tabWidget = new QTabWidget(this);
  vbox->addWidget(m_tabWidget);
  m_tabWidget->setMargin(2);

  connect(m_tabWidget, SIGNAL(currentChanged(QWidget *)), this, SLOT(slotCurrentChanged(QWidget *)));

  m_partManager = new KParts::PartManager(mainwindow, m_tabWidget);
  connect(m_partManager, SIGNAL(activePartChanged(KParts::Part*)), this, SLOT(slotActivePartChanged(KParts::Part*)));
  connect(m_partManager, SIGNAL(partRemoved(KParts::Part*)), this, SLOT(slotPartRemoved(KParts::Part*)));
  connect(m_partManager, SIGNAL(partAdded(KParts::Part*)), this, SLOT(slotPartAdded(KParts::Part*)));

  // TODO: get rid of this editor nonsense
  (void) editor();

  setupActions();
}


PartController::~PartController()
{
}


void PartController::createInstance(QWidget *parent, QWidget *mainwindow, const char *name)
{
  if (!s_instance)
    s_instance = new PartController(parent, mainwindow, name);
}


PartController *PartController::getInstance()
{
  return s_instance;
}


KParts::Part *PartController::activePart()
{
  return m_partManager->activePart();
}


void PartController::setupActions()
{
  (void) KStdAction::open(this, SLOT(slotOpenFile()), 
    TopLevel::getInstance()->actionCollection(), "file_open");

  m_saveAllFilesAction = new KAction(i18n("Save &all"), 0,
    this, SLOT(slotSaveAllFiles()),
    TopLevel::getInstance()->actionCollection(), "file_save_all");
  m_saveAllFilesAction->setEnabled(false);

  m_revertAllFilesAction = new KAction(i18n("&Revert all"), 0,
    this, SLOT(slotRevertAllFiles()),
    TopLevel::getInstance()->actionCollection(), "file_revert_all");
  m_revertAllFilesAction->setEnabled(false);

  m_closeWindowAction = new KAction(i18n("&Close window"), Key_F4,
    this, SLOT(slotCloseWindow()),
    TopLevel::getInstance()->actionCollection(), "file_closewindow");
  m_closeWindowAction->setEnabled(false);

  m_closeAllWindowsAction = new KAction(i18n("Close all &windows"), 0,
    this, SLOT(slotCloseAllWindows()),
    TopLevel::getInstance()->actionCollection(), "file_closeall");
  m_closeAllWindowsAction->setEnabled(false);

  m_closeOtherWindowsAction = new KAction(i18n("Close all &other windows"), 0,
    this, SLOT(slotCloseOtherWindows()),
    TopLevel::getInstance()->actionCollection(), "file_closeother");
  m_closeOtherWindowsAction->setEnabled(false);
}


void PartController::editDocument(const KURL &url, int lineNum)
{
  KParts::Part *existingPart = partForURL(url);
  if (existingPart)
  {
    activatePart(existingPart);
    setLineNumber(lineNum);
    return;
  }

  QString mimeType = KMimeType::findByURL(url, 0, true, true)->name();

  KParts::Factory *factory = 0;
  if (mimeType.startsWith("text/"))
  {
    // TODO: once there is another editor, make the one used
    // configurable
    factory = findPartFactory(mimeType, "KTextEditor/Document");
  }

  if (!factory)
    factory = findPartFactory(mimeType, "KParts/ReadWritePart");

  if (factory)
  {
    KParts::ReadWritePart *part = static_cast<KParts::ReadWritePart*>(factory->createPart(m_tabWidget, "KParts/ReadWritePart"));
    part->openURL(url);
    integratePart(part, url);
    setLineNumber(lineNum);
  }
  else
  {
    // at least try to show the document
    showDocument(url, lineNum);
  }
}


void PartController::showDocument(const KURL &url, int lineNum)
{
  KParts::Part *existingPart = partForURL(url);
  if (existingPart)
  {
    activatePart(existingPart);
    setLineNumber(lineNum);
    return;
  }

  QString mimeType = KMimeType::findByURL(url, 0, true, true)->name();

  KParts::Factory *factory = findPartFactory(mimeType, "KParts/ReadOnlyPart");

  if (factory)
  {
    KParts::ReadOnlyPart *part = static_cast<KParts::ReadOnlyPart*>(factory->createPart(m_tabWidget, "KParts/ReadOnlyPart"));
    part->openURL(url);
    integratePart(part, url);
    setLineNumber(lineNum);
  }
  else
  {
    // try to start a new handling process
    new KRun(url);
  }
}


void PartController::setLineNumber(int lineNum)
{
  KParts::Part *part = m_partManager ->activePart();

  if (!part->inherits("KTextEditor::Document") || !part->widget())
    return;

  KTextEditor::ViewCursorInterface *iface = dynamic_cast<KTextEditor::ViewCursorInterface*>(part->widget());
  if (iface)
    iface->setCursorPosition(lineNum, 0);
}


KParts::Factory *PartController::findPartFactory(const QString &mimeType, const QString &partType)
{
  KTrader::OfferList offers = KTrader::self()->query(mimeType, QString("'%1' in ServiceTypes").arg(partType));

  if (offers.count() > 0)
  {
    KService::Ptr ptr = offers.first();
    return static_cast<KParts::Factory*>(KLibLoader::self()->factory(ptr->library().latin1()));
  }

  return 0;
}


void PartController::integratePart(KParts::Part *part, const KURL &url)
{
  m_tabWidget->addTab(part->widget(), url.fileName());
  m_tabWidget->showPage(part->widget());

  m_partList.append(new PartListEntry(part, url));

  if (part->widget()->inherits("QFrame"))
    kdDebug() << "INHERITS QFRAME!!!!!!!!" << endl;

  m_partManager->addPart(part);
}


void PartController::removePart(KParts::Part *part)
{
  m_partManager->removePart(part);

  delete part;
}


KParts::Part *PartController::partForURL(const KURL &url)
{
  for (PartListEntry *entry = m_partList.first(); entry != 0; entry = m_partList.next())
    if (entry->isEqual(url))
      return entry->part();

  return 0;
}


void PartController::slotActivePartChanged(KParts::Part *part)
{
  kdDebug() << "ACTIVE PART: " << part << endl;

  TopLevel::getInstance()->createGUI(part);

  emit activePartChanged(part);
}


void PartController::activatePart(KParts::Part *part)
{
  m_partManager->setActivePart(part);
  m_tabWidget->showPage(part->widget());
  part->widget()->setFocus();
}


void PartController::updateBufferMenu()
{
  QPtrList<KAction> bufferActions;

  TopLevel::getInstance()->unplugActionList("buffer_list");

  QPtrListIterator<PartListEntry> it(m_partList);
  for ( ; it.current(); ++it)
  {
    QString name = it.current()->url().url();

    KAction *action = new KAction(name, 0, 0, name.latin1());
    connect(action, SIGNAL(activated()), this, SLOT(slotBufferSelected()));
    bufferActions.append(action);
  }

  TopLevel::getInstance()->plugActionList("buffer_list", bufferActions);
}


void PartController::slotBufferSelected()
{
  QPtrListIterator<PartListEntry> it(m_partList);
  for ( ; it.current(); ++it)
    if (it.current()->isEqual(KURL(sender()->name())))
    {
      activatePart(it.current()->part());
      return;
    }
}


void PartController::closeActivePart()
{
  KParts::Part *activePart = m_partManager->activePart();
  if (!activePart)
    return;

  closePart(activePart);
}


void PartController::closePart(KParts::Part *part)
{
  if (part->inherits("KParts::ReadWritePart"))
  {
    KParts::ReadWritePart *rw_part = static_cast<KParts::ReadWritePart*>(part);

    if (rw_part->isModified())
    {
      int res = KMessageBox::warningYesNo(TopLevel::getInstance(),
                  i18n("The document %1 is modified.\n"
                       "Close this window anyway?").arg(rw_part->url().url()));
      if (res == KMessageBox::No)
        return;
    }
  }

  delete part;
}


void PartController::slotPartRemoved(KParts::Part *part)
{
  kdDebug() << "PART REMOVED!!!" << endl;

  QPtrListIterator<PartListEntry> it(m_partList);
  for ( ; it.current(); ++it)
    if (it.current()->part() == part)
    {
      m_partList.remove(it.current());
      break;
    }

  updateBufferMenu();
  updateMenuItems();

  emit partRemoved(part);
}


void PartController::slotPartAdded(KParts::Part *part)
{
  kdDebug() << "PART ADDED!!!!" << endl;

  updateBufferMenu();
  updateMenuItems();

  emit partAdded(part);
}


void PartController::updateMenuItems()
{
  bool hasWriteParts = false;
  bool hasReadOnlyParts = false;

  for (PartListEntry *entry = m_partList.first(); entry != 0; entry = m_partList.next())
  {
    if (entry->part()->inherits("KParts::ReadWritePart"))
      hasWriteParts = true;
    if (entry->part()->inherits("KParts::ReadOnlyPart"))
      hasReadOnlyParts = true;
  }

  m_saveAllFilesAction->setEnabled(hasWriteParts);
  m_revertAllFilesAction->setEnabled(hasWriteParts);
  m_closeWindowAction->setEnabled(hasReadOnlyParts);
  m_closeAllWindowsAction->setEnabled(hasReadOnlyParts);
  m_closeOtherWindowsAction->setEnabled(hasReadOnlyParts);
}


KEditor::Editor *PartController::editor()
{
  // TODO: get rid of all this deprecated editor nonsense!

  static KEditor::Editor *_editor = 0;

  // only load the editor once
  if (_editor)
    return _editor;

  // find the preferred editor
  KConfig *config = kapp->config();
  config->setGroup("Editor");
  //  QString editor = config->readEntry("EmbeddedEditor", "KWriteEditorPart"); // dosn't work at the moment
  QString editor = config->readEntry("EmbeddedEditor","EditorTestPart");


  // ask the trader about the editors, using the preferred one if available
  KTrader::OfferList offers = KTrader::self()->query(QString::fromLatin1("KDevelop/Editor"), QString("Name == '%1'").arg(editor));

  // try to load the editor
  KTrader::OfferList::Iterator it;
  for (it = offers.begin(); it != offers.end(); ++it)
  {
    KLibFactory *factory = KLibLoader::self()->factory((*it)->library().latin1());
    if (!factory)
      continue;

    // Note: The create function might return 0, in that
    // case, we continue with our list
    _editor = static_cast<KEditor::Editor*>(factory->create(TopLevel::getInstance(), "editor"));
    if (_editor)
      break;
  }

  // Note: We should probably abort the application if no editor
  // is found at all!
  if (!_editor){
    KMessageBox::sorry(TopLevel::getInstance(), i18n("Can't find a Editor plugin :-(."));
    exit(0);
  }

  connect(m_partManager, SIGNAL(activePartChanged(KParts::Part*)), _editor, SLOT(activePartChanged(KParts::Part*)));

  return _editor;
}


void PartController::slotSaveAllFiles()
{
  saveAllFiles();
}


void PartController::saveAllFiles()
{
  QPtrListIterator<KParts::Part> it(*m_partManager->parts());
  for ( ; it.current(); ++it)
    if (it.current()->inherits("KParts::ReadWritePart"))
    {
      KParts::ReadWritePart *rw_part = static_cast<KParts::ReadWritePart*>(it.current());
      rw_part->save();

      Core::getInstance()->message(i18n("Saved %1").arg(rw_part->url().url()));
    }
}


void PartController::slotRevertAllFiles()
{
  revertAllFiles();
}


void PartController::revertAllFiles()
{
  QPtrListIterator<KParts::Part> it(*m_partManager->parts());
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
  while (!m_partList.isEmpty())
    closePart(m_partList.first()->part());
}


void PartController::slotCloseOtherWindows()
{
  KParts::Part *activePart = m_partManager->activePart();
  if (!activePart)
    return;

  while (m_partList.count() > 1)
  {
    QPtrListIterator<PartListEntry> it(m_partList);
    for ( ; it.current(); ++it)
      if (it.current()->part() != activePart)
        closePart(it.current()->part());
  }
}


void PartController::slotCurrentChanged(QWidget *w)
{
  QPtrListIterator<PartListEntry> it(m_partList);
  for ( ; it.current(); ++it)
    if (it.current()->part()->widget() == w)
    {
      m_partManager->setActivePart(it.current()->part(), w);
      break;
    }
}


void PartController::slotOpenFile()
{
  QString fileName = KFileDialog::getOpenFileName(QString::null, "*.*", TopLevel::getInstance(), i18n("Open file"));
  if (fileName.isNull())
    return;

  editDocument(KURL(fileName));
}


bool PartController::closeDocuments(const QStringList &documents)
{
  QStringList::ConstIterator it;
  for (it=documents.begin(); it != documents.end(); ++it)
  {
    KParts::Part *part = partForURL(KURL(*it));
    if (!part || !part->inherits("KParts::ReadWritePart"))
      continue;

    KParts::ReadWritePart *rw_part = static_cast<KParts::ReadWritePart*>(part);
    if (rw_part->isModified())
    {
      int res = KMessageBox::warningYesNoCancel(TopLevel::getInstance(),
        i18n("The document %1 is modified. Do you want to save it?").arg(rw_part->url().url()),
	i18n("Save file?"), i18n("Save"), i18n("Discard"), i18n("Cancel"));
      if (res == KMessageBox::Cancel)
        return false;
      if (res == KMessageBox::Ok)
        rw_part->save();
    }
    
    closePart(part);
  }

  return true;
}


#include "partcontroller.moc"
