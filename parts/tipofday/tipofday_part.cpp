#include "tipofday_part.h"

#include <qcheckbox.h>
#include <qpushbutton.h>
#include <qwhatsthis.h>

#include <kaction.h>
#include <kapplication.h>
#include <kconfig.h>
#include <kgenericfactory.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <ktextbrowser.h>
#include <ktip.h>

#include "kdevapi.h"
#include "kdevcore.h"


typedef KGenericFactory<TipOfDayPart> TipOfDayFactory;
K_EXPORT_COMPONENT_FACTORY( libkdevtipofday, TipOfDayFactory( "kdevtipofday" ) );

TipOfDayPart::TipOfDayPart(QObject *parent, const char *name, const QStringList &)
  : KDevPlugin(parent, name)
{
  setInstance(TipOfDayFactory::instance());

  setXMLFile("kdevpart_tipofday.rc");

  KAction *action;

  action = new KAction(i18n("&Tip of the Day"), "idea", 0,
                       this, SLOT(showTip()), actionCollection(), "help_tipofday");
  action->setStatusText(i18n("Gives you another good tip how to use KDevelop"));
  action->setWhatsThis(i18n("Tip of the day\n\n"
                            "Will display another good tip \n"
                            "contributed by KDevelop users."));

  connect(core(), SIGNAL(coreInitialized()), this, SLOT(showOnStart()));
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
