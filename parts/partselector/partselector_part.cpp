#include <qvbox.h>


#include <kiconloader.h>
#include <klocale.h>
#include <kdialogbase.h>


#include <kdevcore.h>


#include "partselector_factory.h"
#include "partselector_part.h"
#include "partselector_widget.h"


PartSelectorPart::PartSelectorPart(KDevApi *api, QObject *parent, const char *name)
  : KDevPart(api, parent, name)
{
  connect(core(), SIGNAL(configWidget(KDialogBase*)), this, SLOT(configWidget(KDialogBase*)));
}


PartSelectorPart::~PartSelectorPart()
{
}


void PartSelectorPart::configWidget(KDialogBase *dlg)
{
  QVBox *vbox = dlg->addVBoxPage(i18n("Plugins"));
  PartSelectorWidget *w = new PartSelectorWidget(vbox, "plugin config widget");
  connect(dlg, SIGNAL(okClicked()), w, SLOT(accept()));
}


#include "partselector_part.moc"
