#include "uichooser_part.h"

#include <qvbox.h>

#include <kdialogbase.h>
#include <kdevgenericfactory.h>
#include <kiconloader.h>
#include <klocale.h>

#include "kdevcore.h"

#include "uichooser_widget.h"


typedef KDevGenericFactory<UIChooserPart> UIChooserFactory;
static const KAboutData data("kdevuichooser", I18N_NOOP("User Interface"), "1.0");
K_EXPORT_COMPONENT_FACTORY( libkdevuichooser, UIChooserFactory( &data ) )

UIChooserPart::UIChooserPart(QObject *parent, const char *name, const QStringList &)
  : KDevPlugin( "UIChooser", "uichooser", parent, name ? name : "UIChooserPart")
{
  setInstance(UIChooserFactory::instance());

  connect(core(), SIGNAL(configWidget(KDialogBase*)), this, SLOT(configWidget(KDialogBase*)));
}


UIChooserPart::~UIChooserPart()
{
}


void UIChooserPart::configWidget(KDialogBase *dlg)
{
  QVBox *vbox = dlg->addVBoxPage(i18n("User Interface"));
  UIChooserWidget *w = new UIChooserWidget(vbox);
  w->setPart(this);
  connect(dlg, SIGNAL(okClicked()), w, SLOT(accept()));
}



#include "uichooser_part.moc"
