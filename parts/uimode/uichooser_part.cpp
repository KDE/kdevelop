#include "uichooser_part.h"

#include <qvbox.h>

#include <kdialogbase.h>
#include <kgenericfactory.h>
#include <kiconloader.h>
#include <klocale.h>

#include "kdevcore.h"

#include "uichooser_widget.h"


typedef KGenericFactory<UIChooserPart> UIChooserFactory;
K_EXPORT_COMPONENT_FACTORY( libkdevuichooser, UIChooserFactory( "kdevuichooser" ) );

UIChooserPart::UIChooserPart(QObject *parent, const char *name, const QStringList &)
  : KDevPlugin(parent, name ? name : "UIChooserPart")
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
