#include <qlayout.h>
#include <qtabwidget.h>
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
#include <kfiledialog.h>
#include <kmainwindow.h>
#include <kaction.h>

#include <ktexteditor/viewcursorinterface.h>
#include <ktexteditor/popupmenuinterface.h>
#include <ktexteditor/editinterface.h>


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


PartController::PartController(QWidget *parent)
  : KDevPartController(parent)
{
  connect(this, SIGNAL(partRemoved(KParts::Part*)), this, SLOT(slotPartRemoved(KParts::Part*)));
  connect(this, SIGNAL(partAdded(KParts::Part*)), this, SLOT(slotPartAdded(KParts::Part*)));
  connect(this, SIGNAL(activePartChanged(KParts::Part*)), this, SLOT(slotActivePartChanged(KParts::Part*)));

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

  m_saveAllFilesAction = new KAction(i18n("Save &all"), 0,
    this, SLOT(slotSaveAllFiles()),
    ac, "file_save_all");
  m_saveAllFilesAction->setEnabled(false);

  m_revertAllFilesAction = new KAction(i18n("&Revert all"), 0,
    this, SLOT(slotRevertAllFiles()),
    ac, "file_revert_all");
  m_revertAllFilesAction->setEnabled(false);

  m_closeWindowAction = new KAction(i18n("&Close window"), Key_F4,
    this, SLOT(slotCloseWindow()),
    ac, "file_closewindow");
  m_closeWindowAction->setEnabled(false);

  m_closeAllWindowsAction = new KAction(i18n("Close all &windows"), 0,
    this, SLOT(slotCloseAllWindows()),
    ac, "file_closeall");
  m_closeAllWindowsAction->setEnabled(false);

  m_closeOtherWindowsAction = new KAction(i18n("Close all &other windows"), 0,
    this, SLOT(slotCloseOtherWindows()),
    ac, "file_closeother");
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
    KConfig *config = kapp->config();
    config->setGroup("Editor");
    QString editor = config->readEntry("EmbeddedKTextEditor", "");

    factory = findPartFactory(mimeType, "KTextEditor/Document", editor);
  }

  if (!factory)
    factory = findPartFactory(mimeType, "KParts/ReadWritePart");

  if (factory)
  {
    KParts::ReadWritePart *part = static_cast<KParts::ReadWritePart*>(factory->createPart(TopLevel::getInstance()->main(), "KParts/ReadWritePart"));
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
    KParts::ReadOnlyPart *part = static_cast<KParts::ReadOnlyPart*>(factory->createPart(TopLevel::getInstance()->main(), "KParts/ReadOnlyPart"));
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
  KParts::Part *part = activePart();

  if (!part->inherits("KTextEditor::Document") || !part->widget())
    return;

  KTextEditor::ViewCursorInterface *iface = dynamic_cast<KTextEditor::ViewCursorInterface*>(part->widget());
  if (iface)
    iface->setCursorPosition(lineNum, 0);
}


KParts::Factory *PartController::findPartFactory(const QString &mimeType, const QString &partType, const QString &preferredName)
{
  kdDebug() << "Preferred Name: " << preferredName << endl;
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

  m_partList.append(new PartListEntry(part, url));

  addPart(part);

  if (part->inherits("KTextEditor::Document") && part->widget())
  {
    KTextEditor::PopupMenuInterface *iface = dynamic_cast<KTextEditor::PopupMenuInterface*>(part->widget());
    if (iface)
      iface->installPopup(contextPopupMenu());

    // HACK: this is a workaround. The kate-part does not emit "completed" when
    // it save a file yet.
    // FIXME: remove this line once kate-part emits the right signal
    connect(part, SIGNAL(fileNameChanged()), this, SLOT(slotUploadFinished()));
  }

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
  {
    popup = (QPopupMenu*)(TopLevel::getInstance()->main())->factory()->container("rb_popup", TopLevel::getInstance()->main());
    connect(popup, SIGNAL(aboutToShow()), this, SLOT(slotPopupAboutToShow()));
    connect(popup, SIGNAL(aboutToHide()), this, SLOT(slotPopupAboutToHide()));
  }

  return popup;
}


KParts::Part *PartController::partForURL(const KURL &url)
{
  for (PartListEntry *entry = m_partList.first(); entry != 0; entry = m_partList.next())
    if (entry->isEqual(url))
      return entry->part();

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


void PartController::updateBufferMenu()
{
  QPtrList<KAction> bufferActions;

  TopLevel::getInstance()->main()->unplugActionList("buffer_list");

  QPtrListIterator<PartListEntry> it(m_partList);
  for ( ; it.current(); ++it)
  {
    QString name = it.current()->url().url();

    KToggleAction *action = new KToggleAction(name, 0, 0, name.latin1());
    action->setChecked(it.current()->part() == activePart());
    connect(action, SIGNAL(activated()), this, SLOT(slotBufferSelected()));
    bufferActions.append(action);
  }

  TopLevel::getInstance()->main()->plugActionList("buffer_list", bufferActions);
}


void PartController::slotBufferSelected()
{
  QPtrListIterator<PartListEntry> it(m_partList);
  for ( ; it.current(); ++it)
    if (it.current()->isEqual(KURL(sender()->name())))
    {
      activatePart(it.current()->part());
      break;
    }

  updateBufferMenu();
}


void PartController::closeActivePart()
{
  if (!activePart())
    return;

  closePart(activePart());
}


void PartController::closePart(KParts::Part *part)
{
  if (part->inherits("KParts::ReadWritePart"))
  {
    KParts::ReadWritePart *rw_part = static_cast<KParts::ReadWritePart*>(part);

    if (rw_part->isModified())
    {
      int res = KMessageBox::warningYesNoCancel(TopLevel::getInstance()->main(),
	          i18n("The document %1 is modified. Do you want to save it?").arg(rw_part->url().url()),
	          i18n("Save file?"), i18n("Save"), i18n("Discard"), i18n("Cancel"));
      if (res == KMessageBox::Cancel)
        return;
      if (res == KMessageBox::Ok)
        rw_part->save();
    }
  }

  if (part->widget())
    TopLevel::getInstance()->removeView(part->widget());
  
  delete part;
}


void PartController::slotPartRemoved(KParts::Part *part)
{
  QPtrListIterator<PartListEntry> it(m_partList);
  for ( ; it.current(); ++it)
    if (it.current()->part() == part)
    {
      m_partList.remove(it.current());
      break;
    }

  updateBufferMenu();
  updateMenuItems();
}


void PartController::slotPartAdded(KParts::Part *)
{
  updateBufferMenu();
  updateMenuItems();
}


void PartController::slotActivePartChanged(KParts::Part *)
{
  updateBufferMenu();
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

      Core::getInstance()->message(i18n("Saved %1").arg(rw_part->url().url()));
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
  while (!m_partList.isEmpty())
    closePart(m_partList.first()->part());
}


void PartController::slotCloseOtherWindows()
{
  if (!activePart())
    return;

  while (m_partList.count() > 1)
  {
    QPtrListIterator<PartListEntry> it(m_partList);
    for ( ; it.current(); ++it)
      if (it.current()->part() != activePart())
        closePart(it.current()->part());
  }
}


void PartController::slotCurrentChanged(QWidget *w)
{
  QPtrListIterator<PartListEntry> it(m_partList);
  for ( ; it.current(); ++it)
    if (it.current()->part()->widget() == w)
    {
      setActivePart(it.current()->part(), w);
      break;
    }
}


void PartController::slotOpenFile()
{
  QString fileName = KFileDialog::getOpenFileName(QString::null, "*.*", TopLevel::getInstance()->main(), i18n("Open file"));
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
      int res = KMessageBox::warningYesNoCancel(TopLevel::getInstance()->main(),
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


void PartController::slotPopupAboutToShow()
{
  QPopupMenu *popup = (QPopupMenu*)sender();
  if (!popup)
    return;

  // ugly hack: mark the "original" items 
  m_popupIds.resize(popup->count());
  for (uint index=0; index < popup->count(); ++index)
    m_popupIds[index] = popup->idAt(index);
  
  // first fill the menu in the file context

  if (!activePart())
    return;

  KParts::ReadOnlyPart *ro_part = dynamic_cast<KParts::ReadOnlyPart*>(activePart());
  if (ro_part)
  {
    FileContext context(ro_part->url().path());
    Core::getInstance()->fillContextMenu(popup, &context);
  }
  
  // second, fill the menu in the editor context

  if (!activePart()->widget())
    return;

  KTextEditor::ViewCursorInterface *cursorIface = dynamic_cast<KTextEditor::ViewCursorInterface*>(activePart()->widget());
  KTextEditor::EditInterface *editIface = dynamic_cast<KTextEditor::EditInterface*>(activePart());

  if (!cursorIface || !editIface)
  {
    Core::getInstance()->fillContextMenu(popup, 0);
  }
  else
  {
    uint line, col;
    cursorIface->cursorPosition(&line, &col);
    EditorContext context(editIface->textLine(line), col);
    Core::getInstance()->fillContextMenu(popup, &context);
  }
}


void PartController::slotPopupAboutToHide()
{
  QPopupMenu *popup = (QPopupMenu*)sender();
  if (!popup)
    return;

  // ugly hack: remove all but the "original" items
  for (int index=popup->count()-1; index >= 0; --index)
    if (m_popupIds.contains(popup->idAt(index)) == 0)
      popup->removeItemAt(index);
}


bool PartController::readyToClose()
{
  QPtrListIterator<PartListEntry> it(m_partList);
  for ( ; it.current(); ++it)
  {
    if (!it.current()->part()->inherits("KParts::ReadWritePart"))
      continue;

    KParts::ReadWritePart *rw_part = static_cast<KParts::ReadWritePart*>(it.current()->part());
    if (rw_part->isModified())
    {
      int res = KMessageBox::warningYesNoCancel(TopLevel::getInstance()->main(), 
		  i18n("The document %1 is modified. Do you want to save it?").arg(rw_part->url().url()), 
		  i18n("Save file?"), i18n("Save"), i18n("Discard"), i18n("Cancel"));

      if (res == KMessageBox::Cancel)
        return false;

      if (res == KMessageBox::Ok)
        rw_part->save();
    }

  }

  return true;
}


#include "partcontroller.moc"
