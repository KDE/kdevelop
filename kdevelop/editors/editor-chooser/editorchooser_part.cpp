#include <qvbox.h>


#include <kiconloader.h>
#include <klocale.h>
#include <kdialogbase.h>
#include <kdevgenericfactory.h>

#include <kdevcore.h>
#include <kdevplugininfo.h>

#include "editorchooser_part.h"
#include "editorchooser_widget.h"

typedef KDevGenericFactory<EditorChooserPart> EditorChooserFactory;
static const KDevPluginInfo data("kdeveditorchooser");
K_EXPORT_COMPONENT_FACTORY( libkdeveditorchooser, EditorChooserFactory( data ) )

EditorChooserPart::EditorChooserPart(QObject *parent, const char *name, const QStringList &)
  : KDevPlugin(&data, parent, name ? name : "EditorChooserPart")
{
  setInstance(EditorChooserFactory::instance());

  connect(core(), SIGNAL(configWidget(KDialogBase*)), this, SLOT(configWidget(KDialogBase*)));
}


EditorChooserPart::~EditorChooserPart()
{
}


void EditorChooserPart::configWidget(KDialogBase *dlg)
{
  QVBox *vbox = dlg->addVBoxPage(i18n("Editor"), i18n("Editor"), BarIcon("kate", KIcon::SizeMedium) );
  EditorChooserWidget *w = new EditorChooserWidget(vbox);
  connect(dlg, SIGNAL(okClicked()), w, SLOT(accept()));
}



#include "editorchooser_part.moc"
