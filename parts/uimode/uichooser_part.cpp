#include <qvbox.h>


#include <kiconloader.h>
#include <klocale.h>
#include <kdialogbase.h>


#include <kdevcore.h>


#include "uichooser_factory.h"
#include "uichooser_part.h"
#include "uichooser_widget.h"


UIChooserPart::UIChooserPart(KDevApi *api, QObject *parent, const char *name)
  : KDevPart(api, parent, name)
{
  setInstance(UIChooserFactory::instance());

  connect(core(), SIGNAL(configWidget(KDialogBase*)), this, SLOT(configWidget(KDialogBase*)));
}


UIChooserPart::~UIChooserPart()
{
}


void UIChooserPart::configWidget(KDialogBase *dlg)
{
  QVBox *vbox = dlg->addVBoxPage(i18n("UI"));
  UIChooserWidget *w = new UIChooserWidget(vbox);
  connect(dlg, SIGNAL(okClicked()), w, SLOT(accept()));
}



#include "uichooser_part.moc"
