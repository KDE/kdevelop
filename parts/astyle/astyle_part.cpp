#include <qwhatsthis.h>
#include <qvbox.h>
#include <qtextstream.h>


#include <kiconloader.h>
#include <klocale.h>
#include <kdialogbase.h>
#include <kparts/partmanager.h>
#include <kdebug.h>


#include <kdevcore.h>


#include "keditor/editor.h"
#include "keditor/edit_iface.h"


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

  connect(core()->editor(), SIGNAL(documentActivated(KEditor::Document*)),
		  this, SLOT(documentActivated(KEditor::Document*)));
}


AStylePart::~AStylePart()
{
}


void AStylePart::beautifySource()
{
  KEditor::Document *doc = core()->editor()->currentDocument();
  if (!doc)
	return;

  KEditor::EditDocumentIface *iface = KEditor::EditDocumentIface::interface(doc);
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


void AStylePart::configWidget(KDialogBase *dlg)
{
  QVBox *vbox = dlg->addVBoxPage(i18n("Source formatter"));
  AStyleWidget *w = new AStyleWidget(vbox, "astyle config widget");
  connect(dlg, SIGNAL(okClicked()), w, SLOT(accept()));
}


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
	if (!KEditor::EditDocumentIface::interface(doc))
	  enabled = false;
  }

  _action->setEnabled(enabled);
}


#include "astyle_part.moc"
