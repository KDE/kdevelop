#include <qtabdialog.h>
#include <qvbox.h>


#include <kiconloader.h>
#include <klocale.h>
//#include <kdialogbase.h>


#include <kdevcore.h>


#include "partselector_factory.h"
#include "partselector_part.h"
#include "partselector_widget.h"


PartSelectorPart::PartSelectorPart(KDevApi *api, QObject *parent, const char *name)
  : KDevPart(api, parent, name)
{
  connect(core(), SIGNAL(configWidget(QTabDialog*)), this, SLOT(configWidget(QTabDialog*)));
}


PartSelectorPart::~PartSelectorPart()
{
}


void PartSelectorPart::configWidget(QTabDialog *dlg)
{
#if 0
  QVBox *vbox = dlg->addVBoxPage(i18n("Plugins"));
  PartSelectorWidget *w = new PartSelectorWidget(vbox, "plugin config widget");
  connect(dlg, SIGNAL(okClicked()), w, SLOT(accept()));
#endif
  PartSelectorWidget *w = new PartSelectorWidget(dlg, "plugin config widget");
  dlg->addTab(w, i18n("Plugins"));
  connect(dlg, SIGNAL(applyButtonPressed()), w, SLOT(accept()));
}


#include "partselector_part.moc"
