#include "historypart.h"

#include <kaction.h>
#include <kdebug.h>
#include <kgenericfactory.h>
#include <kmainwindow.h>
#include <kparts/part.h>
#include <kpopupmenu.h>
#include <kiconloader.h>

#include <ktexteditor/editor.h>

#include "kdevcore.h"
#include "kdevpartcontroller.h"
#include "kdevmainwindow.h"


class HistoryEntry
{
public:
  HistoryEntry(KParts::Part *part) : m_part(part) {};

  KParts::Part *m_part;
  KURL m_url;

};


typedef KGenericFactory<HistoryPart> HistoryPartFactory;

K_EXPORT_COMPONENT_FACTORY(libkdevhistory, HistoryPartFactory("kdevhistory"))

HistoryPart::HistoryPart(QObject *parent, const char *name, const QStringList &)
  : KDevPlugin("History", "history", parent, name ? name : "HistoryPart"), m_restoring(false)
{
  setInstance(HistoryPartFactory::instance());

  setXMLFile("kdevhistory.rc");

  connect(partController(), SIGNAL(partRemoved(KParts::Part*)), this, SLOT(partRemoved(KParts::Part*)));
  connect(partController(), SIGNAL(activePartChanged(KParts::Part*)), this, SLOT(activePartChanged(KParts::Part*)));
  setupActions();

  m_history.setAutoDelete(true);

  updateActions();
}


HistoryPart::~HistoryPart()
{
}


void HistoryPart::setupActions()
{
  m_backAction = new KToolBarPopupAction(i18n("&Back"), "back", 0,
    			this, SLOT(backActivated()),
  			actionCollection(), "history_back");
  m_backAction->setToolTip(i18n("Back"));
  m_backAction->setWhatsThis(i18n("<b>Back</b><p>Moves backwards one step in the browsing history."));

  m_forwardAction = new KToolBarPopupAction(i18n("&Forward"), "forward", 0,
		 	this, SLOT(forwardActivated()),
			actionCollection(), "history_forward");
  m_forwardAction->setToolTip(i18n("Forward"));
  m_forwardAction->setWhatsThis(i18n("<b>Forward</b><p>Moves forward one step in the browsing history."));

  connect(m_backAction->popupMenu(), SIGNAL(aboutToShow()),
	 this, SLOT(backAboutToShow()));
  connect(m_backAction->popupMenu(), SIGNAL(activated(int)),
	 this, SLOT(backPopupActivated(int)));
  connect(m_forwardAction->popupMenu(), SIGNAL(aboutToShow()),
	 this, SLOT(forwardAboutToShow()));
  connect(m_forwardAction->popupMenu(), SIGNAL(activated(int)),
	 this, SLOT(forwardPopupActivated(int)));
}


void HistoryPart::backAboutToShow()
{
  KPopupMenu *popup = m_backAction->popupMenu();
  popup->clear();

  int savePos = m_history.at();
  for (int i=0; i<10 && m_history.prev(); ++i)
    popup->insertItem(m_history.current()->m_url.fileName() );

  m_history.at(savePos);
}


void HistoryPart::forwardAboutToShow()
{
  KPopupMenu *popup = m_forwardAction->popupMenu();
  popup->clear();

  int savePos = m_history.at();
  for (int i=0; i<10 && m_history.next(); ++i)
    popup->insertItem(m_history.current()->m_url.fileName() );

  m_history.at(savePos);
}


void HistoryPart::backPopupActivated(int id)
{
  int by = m_backAction->popupMenu()->indexOf(id);

  saveState(partController()->activePart());
  for (int i=0; i < by; ++i)
    m_history.prev();
  if(m_history.prev()==0L) m_history.first();

  restoreState();

  updateActions();
}


void HistoryPart::forwardPopupActivated(int id)
{
  int by = m_forwardAction->popupMenu()->indexOf(id)+1;

  saveState(partController()->activePart());
  for (int i=0; i < by; ++i)
    m_history.next();
  if(m_history.current()==0L) m_history.last();

  restoreState();

  updateActions();
}


void HistoryPart::updateActions()
{
  m_backAction->setEnabled(m_history.current() != m_history.getFirst());
  m_forwardAction->setEnabled(m_history.current() != m_history.getLast());

  kdDebug(9031) << "history-length=" << m_history.count() << endl;
}


void HistoryPart::addHistoryEntry(HistoryEntry *entry)
{
  HistoryEntry *current = m_history.current();
  while (m_history.getLast() != current)
    m_history.removeLast();
  m_history.append(entry);
  m_history.last();

  saveState(entry->m_part);

  updateActions();
}

void HistoryPart::saveState(KParts::Part *part)
{
  if (!part || !part->inherits( "KTextEditor::Editor" ) )
    return;

  HistoryEntry *entry = m_history.current();
  if (!entry)
    return;

  entry->m_part = part;

  KParts::ReadOnlyPart *ro_part = dynamic_cast<KParts::ReadOnlyPart*>(part);
  if (ro_part)
    entry->m_url = ro_part->url();
  else
    entry->m_url = KURL();
}


void HistoryPart::restoreState()
{
  HistoryEntry *entry = m_history.current();
  if (!entry)
    return;

  m_restoring = true;

  partController()->setActivePart(entry->m_part);
  if (entry->m_part->widget())
  {
    mainWindow()->raiseView(entry->m_part->widget());
    entry->m_part->widget()->setFocus();
  }

  m_restoring = false;

  updateActions();
}


void HistoryPart::backActivated()
{
  saveState(partController()->activePart());

  if(m_history.prev()==0L) m_history.first();

  restoreState();
} 


void HistoryPart::forwardActivated()
{
  saveState(partController()->activePart());

  if(m_history.next()==0L)  m_history.last();

  restoreState();
}


void HistoryPart::activePartChanged(KParts::Part *part)
{
  kdDebug(9031) << "HistoryPart::activePartChanged()" << endl;
  
  if (!part || m_restoring || !part->inherits( "KTextEditor::Editor" ) )
    return;

  HistoryEntry *entry = new HistoryEntry(part);

  addHistoryEntry(entry);
}


void HistoryPart::partRemoved(KParts::Part *part)
{
  QPtrListIterator<HistoryEntry> it(m_history);

  for ( ; it.current(); ++it)
    if (it.current()->m_part == part)
      m_history.remove(it.current());

  updateActions();
}


#include "historypart.moc"
