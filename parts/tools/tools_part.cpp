#include <qwhatsthis.h>
#include <qvbox.h>
#include <qfile.h>


#include <kiconloader.h>
#include <klocale.h>
#include <kdialogbase.h>
#include <kconfig.h>
#include <kapplication.h>
#include <kdesktopfile.h>
#include <kdebug.h>


#include <kdevcore.h>


#include "tools_factory.h"
#include "tools_part.h"
#include "toolsconfig.h"


ToolsPart::ToolsPart(KDevApi *api, QObject *parent, const char *name)
    : KDevPart(api, parent, name)
{
  setInstance(ToolsFactory::instance());

  setXMLFile("kdevpart_tools.rc");

  connect(core(), SIGNAL(configWidget(KDialogBase*)), this, SLOT(configWidget(KDialogBase*)));

  connect(core(), SIGNAL(coreInitialized()), this, SLOT(updateMenu()));
}


ToolsPart::~ToolsPart()
{
}


void ToolsPart::configWidget(KDialogBase *dlg)
{
  QVBox *vbox = dlg->addVBoxPage(i18n("Tools menu"));
  ToolsConfig *w = new ToolsConfig(vbox, "tools config widget");
  connect(dlg, SIGNAL(okClicked()), w, SLOT(accept()));
  connect(dlg, SIGNAL(destroyed()), this, SLOT(updateMenu()));
}



void ToolsPart::updateMenu()
{
  QList<KAction> actions;

  unplugActionList("tools_list");

  KConfig *config = kapp->config();
  config->setGroup("Tools");

  QStringList list = config->readListEntry("Tools");
  for (QStringList::Iterator it = list.begin(); it != list.end(); ++it)
	{
	  QString name = *it;

	  KDesktopFile df(name, true);
	  if (df.readName().isNull())
		continue;

	  KAction *action = new KAction(df.readName(), df.readIcon(), 0,
                                        this, SLOT(slotToolActivated()), (QObject*)0, name.latin1());
	  actions.append(action);
	}

  plugActionList("tools_list", actions);
}


void ToolsPart::slotToolActivated()
{
  QString df = sender()->name();
  kapp->startServiceByDesktopPath(df);
}


#include "tools_part.moc"
