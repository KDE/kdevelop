#include <kapp.h>
#include <kconfig.h>


#include "toplevel.h"


#include "toplevel_mdi.h"
#include "toplevel_sdi.h"


KDevTopLevel *TopLevel::s_instance = 0;


KDevTopLevel *TopLevel::getInstance()
{
  if (!s_instance)
  {
    KConfig *config = kapp->config();
    config->setGroup("UI");

    if (config->readEntry("MajorUIMode", "mdi") == "mdi")
    {
      TopLevelMDI *mdi = new TopLevelMDI;
      s_instance = mdi;

      mdi->init();

      kapp->setMainWidget(mdi);
    }
    else
    {
      TopLevelSDI *sdi = new TopLevelSDI;
      s_instance = sdi;

      sdi->init();

      kapp->setMainWidget(sdi);
    }
  }

  return s_instance;
}
