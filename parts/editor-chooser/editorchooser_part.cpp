#include <qvbox.h>


#include <kiconloader.h>
#include <klocale.h>
#include <kdialogbase.h>


#include <kdevcore.h>


#include "editorchooser_factory.h"
#include "editorchooser_part.h"
#include "editorchooser_widget.h"


EditorChooserPart::EditorChooserPart(KDevApi *api, QObject *parent, const char *name)
  : KDevPart(api, parent, name)
{
  setInstance(EditorChooserFactory::instance());

  connect(core(), SIGNAL(configWidget(KDialogBase*)), this, SLOT(configWidget(KDialogBase*)));
}


EditorChooserPart::~EditorChooserPart()
{
}


void EditorChooserPart::configWidget(KDialogBase *dlg)
{
  QVBox *vbox = dlg->addVBoxPage(i18n("Editor"));
  EditorChooserWidget *w = new EditorChooserWidget(vbox);
  connect(dlg, SIGNAL(okClicked()), w, SLOT(accept()));
}



#include "editorchooser_part.moc"
