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


#include "tipofday_factory.h"
#include "tipofday_part.h"
#include "tipofday.h"
#include "ktipdatabase.h"


TipOfDayPart::TipOfDayPart(KDevApi *api, QObject *parent, const char *name)
    : KDevPart(api, parent, name), _tipWidget(0), _tipDatabase(0)
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


TipOfDayPart::~TipOfDayPart()
{
  delete _tipDatabase;
}


void TipOfDayPart::showTip()
{
  if (!_tipWidget)
    {
      _tipWidget = new TipOfDay(kapp->mainWidget());

      KTextBrowser *text = _tipWidget->TipText;
      text->mimeSourceFactory()->addFilePath(KGlobal::dirs()->findResourceDir("data", "kdevtipofday/pics") + "kdevtipofday/pics/");
      QStringList icons = KGlobal::dirs()->resourceDirs("icon");
      QStringList::Iterator it;
      for (it = icons.begin(); it != icons.end(); ++it)
        text->mimeSourceFactory()->addFilePath(*it);

      KConfig *config = kapp->config();
      config->setGroup("TipOfDay");
      _tipWidget->RunOnStart->setChecked(config->readBoolEntry("RunOnStart", true));
      
      QObject::connect(_tipWidget->NextButton, SIGNAL(clicked()), this, SLOT(nextTip()));
      QObject::connect(_tipWidget->RunOnStart, SIGNAL(toggled(bool)), this, SLOT(toggleOnStart(bool)));
    }

  if (!_tipDatabase)
    _tipDatabase = new KTipDatabase("kdevtipofday/tips");

  nextTip();
  _tipWidget->show();
  _tipWidget->raise();
}


void TipOfDayPart::nextTip()
{
  _tipDatabase->nextTip();
  _tipWidget->TipText->setText(_tipDatabase->tip());
}


void TipOfDayPart::toggleOnStart(bool on)
{
  KConfig *config = kapp->config();
  config->setGroup("TipOfDay");
  config->writeEntry("RunOnStart", on);
  config->sync();
}


void TipOfDayPart::showOnStart()
{
  KConfig *config = kapp->config();
  config->setGroup("TipOfDay");
  if (config->readBoolEntry("RunOnStart", true))
    showTip();
}


#include "tipofday_part.moc"
