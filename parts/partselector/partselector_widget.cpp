#include <qlistview.h>
#include <qheader.h>


#include <ktrader.h>
#include <kservice.h>
#include <kconfig.h>
#include <kapp.h>
#include <kdebug.h>


#include <kdevcore.h>


#include "partselector_widget.h"


PartSelectorWidget::PartSelectorWidget(QWidget *parent, const char *name)
 : PartSelector(parent, name)
{
  PluginList->header()->hide();

  KTrader::OfferList offers = KTrader::self()->query(QString::fromLatin1("KDevelop/Part"), QString::null);
  if (offers.isEmpty())
    return;
	       
  KConfig *config = kapp->config();
  config->setGroup("Plugins");
  
  for (KTrader::OfferList::ConstIterator it = offers.begin(); it != offers.end(); ++it) 
    {
      KService *service = *it;

      // TODO: perhaps we should have a flag 'required' in the desktop files?
      if (service->name() == "KDevPartSelector")
	continue;
      
      QString text = service->comment();
      if (text.isEmpty())
	text = service->name();
      
      QCheckListItem *item = new QCheckListItem(PluginList, text, QCheckListItem::CheckBox);
      item->setText(2, service->name());
      item->setOn(config->readBoolEntry(service->name(), true));
    }
}


PartSelectorWidget::~PartSelectorWidget()
{
}


void PartSelectorWidget::accept()
{
  KConfig *config = kapp->config();
  config->setGroup("Plugins");

  QListViewItem *item = PluginList->firstChild();
  while (item)
  {
    QCheckListItem *cli = (QCheckListItem*)item;
    config->writeEntry(item->text(2), cli->isOn());
    item = item->nextSibling();
  }
  
  config->sync();
}


#include "partselector_widget.moc"
