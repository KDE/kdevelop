#include <qwhatsthis.h>
#include <qvbox.h>
#include <qtextstream.h>


#include <kiconloader.h>
#include <klocale.h>
#include <kdialogbase.h>
#include <kparts/partmanager.h>
#include <kdebug.h>


#include <kdevcore.h>


#ifdef NEW_EDITOR
#include "keditor/editor.h"
#include "keditor/edit_iface.h"
#else
#include "texteditor.h"
#include "editorpart.h"
#endif


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

#ifdef NEW_EDITOR
  connect(core()->editor(), SIGNAL(documentActivated(KEditor::Document*)),
		  this, SLOT(documentActivated(KEditor::Document*)));
#else
  connect(core()->partManager(), SIGNAL(activePartChanged(KParts::Part *)),
		  this, SLOT(activePartChanged(KParts::Part *)));
#endif
}


AStylePart::~AStylePart()
{
}


#ifdef NEW_EDITOR
void AStylePart::beautifySource()
{
  KEditor::Document *doc = core()->editor()->currentDocument();
  if (!doc)
	return;

  KEditor::EditDocumentIface *iface = static_cast<KEditor::EditDocumentIface*>(doc->queryInterface("KEditor::EditDocumentIface"));
  if (!iface)
	return;

  ASStringIterator is(iface->text());
  KDevFormatter formatter;

  formatter.init(&is);

  QString output;
  QTextStream os(&output, IO_WriteOnly);
  
  while (formatter.hasMoreLines())
	os << formatter.nextLine().c_str() << endl;
  
  iface->setText(output);
}
#else
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
#endif


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


#ifdef NEW_EDITOR
void AStylePart::documentActivated(KEditor::Document *doc)
{
  bool enabled = false;
  
  if (doc)
  {
	QString extension = doc->url().path();
	int pos = extension.findRev('.');
	if (pos >= 0)
	  extension = extension.mid(pos);
	if (extension == ".h" || extension == ".c" || extension == ".java"
		|| extension == ".cpp" || extension == ".cc" || extension == ".C"
		|| extension == ".cxx" || extension == ".hxx")
	  enabled = true;

	if (!doc->queryInterface("KEditor::EditDocumentIface"))
	  enabled = false;
  }

  _action->setEnabled(enabled);
}
#endif


#include "astyle_part.moc"
