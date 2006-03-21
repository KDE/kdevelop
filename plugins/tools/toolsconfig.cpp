#include "toolsconfig.h"

#include <qapplication.h>
#include <qlabel.h>
#include <qlayout.h>
#include <q3listbox.h>
#include <qpushbutton.h>
#include <q3header.h>
//Added by qt3to4:
#include <QShowEvent>
#include <QVBoxLayout>
#include <QHBoxLayout>

#include <kapplication.h>
#include <kdesktopfile.h>
#include <kdialog.h>
#include <kiconloader.h>
#include <klocale.h>

#include "tools_part.h"
#include "kapplicationtree.h"


ToolsConfig::ToolsConfig(QWidget *parent, const char *name)
    : QWidget(parent, name), _tree(0)
{
  _entries.setAutoDelete(true);
}


void ToolsConfig::showEvent(QShowEvent *e)
{
  QWidget::showEvent(e);

  if (!_tree)
    {
      QApplication::setOverrideCursor(Qt::WaitCursor);

      QHBoxLayout *hbox = new QHBoxLayout(this, KDialog::marginHint(), KDialog::spacingHint());

      QVBoxLayout *vbox = new QVBoxLayout(hbox);
      _tree = new KDevApplicationTree(this);
      _tree->header()->hide();
      QLabel *l = new QLabel(_tree, i18n("&Applications:"), this);
      l->show();
      _tree->show();

      vbox->addWidget(l);
      vbox->addWidget(_tree);

      vbox = new QVBoxLayout(hbox);

      _toList = new QPushButton(QApplication::reverseLayout() ? "<<" : ">>", this);
      _toList->show();
      vbox->addWidget(_toList);

      connect(_toList, SIGNAL(clicked()), this, SLOT(toList()));

      _toTree = new QPushButton(QApplication::reverseLayout() ? ">>" : "<<", this);
      _toTree->show();
      vbox->addWidget(_toTree);

      connect(_toTree, SIGNAL(clicked()), this, SLOT(toTree()));

      vbox = new QVBoxLayout(hbox);
      _list = new Q3ListBox(this);
      l = new QLabel(_list, i18n("&Tools menu:"), this);
      l->show();
      _list->show();
      vbox->addWidget(l);
      vbox->addWidget(_list);

      QApplication::restoreOverrideCursor();
    }

  fill();
  checkButtons();

  connect(_tree, SIGNAL(selectionChanged()), this, SLOT(checkButtons()));
  connect(_list, SIGNAL(selectionChanged()), this, SLOT(checkButtons()));
}


void ToolsConfig::checkButtons()
{
  _toList->setEnabled(_tree->selectedItem() && !_tree->selectedItem()->firstChild());
  _toTree->setEnabled(_list->currentItem() >= 0 && _list->currentItem() < (int)_list->count());
}


void ToolsConfig::fill()
{
  _entries.clear();

  KConfig *config = ToolsFactory::instance()->config();
  config->setGroup("Tools");

  QStringList list = config->readListEntry("Tools");

  for (QStringList::Iterator it = list.begin(); it != list.end(); ++it)
	add(*it);
}


void ToolsConfig::add(const QString &desktopFile)
{
  KDesktopFile df(desktopFile, true);
  if (df.readName().isEmpty())
    return;

  Entry *entry = new Entry;

  if (!df.readIcon().isEmpty())
    entry->icon = BarIcon(df.readIcon());
  entry->name = df.readName();
  entry->desktopFile = desktopFile;

  _entries.append(entry);

  updateList();

  checkButtons();
}


void ToolsConfig::toList()
{
  KDevAppTreeListItem *item = dynamic_cast<KDevAppTreeListItem*>(_tree->selectedItem());
  if (item && !item->desktopEntryPath().isEmpty())
    add(item->desktopEntryPath());
  checkButtons();
}


void ToolsConfig::toTree()
{
  _entries.remove(_list->currentItem());
  updateList();
  checkButtons();
}


void ToolsConfig::accept()
{
  KConfig *config = ToolsFactory::instance()->config();
  config->setGroup("Tools");

  QStringList l;
  Q3PtrListIterator<Entry> it(_entries);
    for ( ; it.current(); ++it)
	  l.append(it.current()->desktopFile);

  config->writeEntry("Tools", l);
  config->sync();
}


void ToolsConfig::updateList()
{
  _list->setUpdatesEnabled(false);

  _list->clear();

  Q3PtrListIterator<Entry> it(_entries);
  for ( ; it.current(); ++it)
	_list->insertItem(it.current()->icon, it.current()->name);

  _list->setUpdatesEnabled(true);
  _list->repaint();
}


#include "toolsconfig.moc"
