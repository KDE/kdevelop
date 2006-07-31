#include "tipofday_part.h"

#include <QCheckBox>
#include <QPushButton>

#include <kaboutdata.h>
#include <kaction.h>
#include <kapplication.h>
#include <kconfig.h>
#include <kdevgenericfactory.h>
#include <kdevplugininfo.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <ktextbrowser.h>
#include <ktip.h>

#include "kdevcore.h"

static const KDevPluginInfo data("kdevtipofday");

typedef KDevGenericFactory<TipOfDayPart> TipOfDayFactory;
K_EXPORT_COMPONENT_FACTORY( libkdevtipofday, TipOfDayFactory( data ) )

TipOfDayPart::TipOfDayPart(QObject *parent, const char *name, const QStringList &)
  : KDevPlugin(&data, parent)
{
  setObjectName(QString::fromUtf8(name));
  setInstance(TipOfDayFactory::instance());

  setXMLFile("kdevpart_tipofday.rc");

  KAction *action;

  action = new KAction(i18n("&Tip of the Day"), "idea", 0,
                       this, SLOT(showTip()), actionCollection(), "help_tipofday");
  action->setToolTip(i18n("A tip how to use KDevelop"));
  action->setWhatsThis(i18n("<b>Tip of the day</b><p>"
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
