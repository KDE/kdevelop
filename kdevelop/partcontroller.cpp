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

#include <ktexteditor/viewcursorinterface.h>
#include <ktexteditor/popupmenuinterface.h>
#include <ktexteditor/editinterface.h>


#include "ckdevelop.h"
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
}


PartController::~PartController()
{
}


void PartController::createInstance(QWidget *parent)
{
    if (!s_instance){
        s_instance = new PartController(parent);
    }
}


PartController *PartController::getInstance()
{
  return s_instance;
}

void PartController::editDocument(const KURL &url, int lineNum)
{
    CKDevelop::getInstance()->switchToFile( url.path(), lineNum );
}

void PartController::setServiceType(const QString &serviceType)
{
  m_presetEncoding = serviceType;
}

void PartController::showDocument(const KURL &url, int lineNum)
{
    kdDebug() << "PartController::showDocument() -- NOT IMPLEMENTED YET!!" << endl;
}

void PartController::showDocument(const KURL &url, const QString& /*context*/)
{
  // not implemented yet
  showDocument( url );
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
      KService::Ptr ptr = offers.first();
    }
    return static_cast<KParts::Factory*>(KLibLoader::self()->factory(ptr->library().latin1()));
  }

  return 0;
}


void PartController::integratePart(KParts::Part *part, const KURL &url)
{
  CKDevelop::getInstance()->embedPartView(part->widget(), url.fileName());

  m_partList.append(new PartListEntry(part, url));

  addPart(part);

  if (part->inherits("KTextEditor::Document") && part->widget())
  {
      // TODO: setup popup menu
  }
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
    CKDevelop::getInstance()->raiseView(part->widget());
    part->widget()->setFocus();
  }
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
      int res = KMessageBox::warningYesNo(CKDevelop::getInstance()->main(),
                  i18n("The document %1 is modified.\n"
                       "Close this window anyway?").arg(rw_part->url().url()));
      if (res == KMessageBox::No)
        return;
    }
  }

  if (part->widget())
    CKDevelop::getInstance()->removeView(part->widget());

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

}


void PartController::slotPartAdded(KParts::Part *)
{
}


void PartController::slotActivePartChanged(KParts::Part *)
{
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
            int res = KMessageBox::warningYesNoCancel(CKDevelop::getInstance()->main(),
                                                      i18n("The document %1 is modified. Do you want to save it?").arg(rw_part->url().url()),
                                                      i18n("Save File?"), i18n("Save"), i18n("Discard"), i18n("Cancel"));
            if (res == KMessageBox::Cancel)
                return false;
            if (res == KMessageBox::Ok)
                rw_part->save();
        }

        closePart(part);
    }

    return true;
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


void PartController::gotoExecutionPoint(const KURL &url, int lineNum)
{
  editDocument(url, lineNum);
}


void PartController::clearExecutionPoint()
{
}

void PartController::setEncoding(const QString &encoding)
{
}

#include "partcontroller.moc"
