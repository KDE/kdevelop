#include <qwhatsthis.h>
#include <qvbox.h>
#include <qtextstream.h>


#include <kiconloader.h>
#include <klocale.h>
#include <kdialogbase.h>
#include <kparts/partmanager.h>


#include <kdevcore.h>


// FIXME: Use official interfaces instead
#include "texteditor.h"
#include "editorpart.h"


#include "astyle_factory.h"
#include "astyle_part.h"
#include "astyle_widget.h"
#include "astyle_adaptor.h"


AStylePart::AStylePart(KDevApi *api, QObject *parent, const char *name)
  : KDevPart(api, parent, name)
{
  setInstance(AStyleFactory::instance());
	 
  setXMLFile("kdevpart_astyle.rc");
	   
  _action = new KAction(i18n("&Reformat source"), "kdevelop_astyle", 0,
			  this, SLOT(beautifySource()), actionCollection(), "edit_astyle");
  _action->setEnabled(false);

  connect(core(), SIGNAL(configWidget(KDialogBase*)), this, SLOT(configWidget(KDialogBase*)));

  connect(core()->partManager(), SIGNAL(activePartChanged(KParts::Part *)),
		  this, SLOT(activePartChanged(KParts::Part *)));
}


AStylePart::~AStylePart()
{
}


void AStylePart::beautifySource()
{
  KParts::Part *active = core()->partManager()->activePart();
  if (!active || !active->inherits("EditorPart"))
	return;

  // FIXME: This is ugly hackery. We should really put the
  // editor stuff into 'official' interfaces...
  EditorPart *editor = (EditorPart*)active;
  TextEditorDocument *doc = editor->editorDocument();
  if (!doc)
    return;

  ASStringIterator is(doc->text());
  KDevFormatter formatter;
  
  formatter.init(&is);

  QString output;
  QTextStream os(&output, IO_WriteOnly);
  
  while (formatter.hasMoreLines())
    os << formatter.nextLine().c_str() << endl;

  // FIXME: This breaks undo! Find something better... 
  doc->setText(output); 
  doc->setModified(true);
  doc->updateViews();
}


void AStylePart::configWidget(KDialogBase *dlg)
{
  QVBox *vbox = dlg->addVBoxPage(i18n("Source formatter"));
  AStyleWidget *w = new AStyleWidget(vbox, "astyle config widget");
  connect(dlg, SIGNAL(okClicked()), w, SLOT(accept()));
}


void AStylePart::activePartChanged(KParts::Part *newPart)
{
  _action->setEnabled(newPart && newPart->inherits("EditorPart"));
}


#include "astyle_part.moc"
