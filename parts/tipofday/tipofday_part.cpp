#include <qwhatsthis.h>
#include <qpushbutton.h>
#include <qcheckbox.h>


#include <kiconloader.h>
#include <klocale.h>
#include <kapp.h>
#include <kstddirs.h>
#include <ktextbrowser.h>
#include <kconfig.h>


#include <kdevcore.h>
#include <kdevapi.h>
#include <kdevcore.h>
#include <ktip.h>


#include "tipofday_factory.h"
#include "tipofday_part.h"


TipOfDayPart::TipOfDayPart(KDevApi *api, QObject *parent, const char *name)
  : KDevPart(api, parent, name)
{
  setInstance(TipOfDayFactory::instance());

  setXMLFile("kdevpart_tipofday.rc");

  KAction *action;

  action = new KAction(i18n("&Tip of the day..."), "kdevelop_tip", 0,
                       this, SLOT(showTip()), actionCollection(), "help_tipofday");
  action->setStatusText(i18n("Gives you another good tip how to use KDevelop"));
  action->setWhatsThis(i18n("Tip of the day\n\n"
                            "Will display another good tip \n"
                            "contributed by KDevelop users."));

  connect(api->core, SIGNAL(coreInitialized()), this, SLOT(showOnStart()));
}


void TipOfDayPart::showTip()
{
  KTipDialog::showTip("kdevtipofday/tips", true);
}


void TipOfDayPart::showOnStart()
{
  KTipDialog::showTip("kdevtipofday/tips");
}


#include "tipofday_part.moc"
