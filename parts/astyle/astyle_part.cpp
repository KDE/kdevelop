#include <qwhatsthis.h>
#include <qvbox.h>
#include <qtextstream.h>


#include <kiconloader.h>
#include <klocale.h>
#include <kdialogbase.h>
#include <kparts/part.h>
#include <kparts/partmanager.h>
#include <kdebug.h>
#include <kgenericfactory.h>

#include <kdevcore.h>
#include <kdevapi.h>
#include <kdevpartcontroller.h>


#include <ktexteditor/editinterface.h>

#include "astyle_part.h"
#include "astyle_widget.h"
#include "astyle_adaptor.h"

typedef KGenericFactory<AStylePart> AStyleFactory;
K_EXPORT_COMPONENT_FACTORY( libkdevastyle, AStyleFactory( "kdevastyle" ) );

AStylePart::AStylePart(QObject *parent, const char *name, const QStringList &)
  : KDevPlugin(parent, name)
{
  setInstance(AStyleFactory::instance());
	 
  setXMLFile("kdevpart_astyle.rc");

  _action = new KAction(i18n("&Reformat Source"), 0,
			  this, SLOT(beautifySource()), actionCollection(), "edit_astyle");
  _action->setEnabled(false);

  connect(core(), SIGNAL(configWidget(KDialogBase*)), this, SLOT(configWidget(KDialogBase*)));

  connect(partController(), SIGNAL(activePartChanged(KParts::Part*)), this, SLOT(activePartChanged(KParts::Part*)));
}


AStylePart::~AStylePart()
{
}


void AStylePart::beautifySource()
{
  KParts::Part *part = partController()->activePart();
  if (!part)
    return;
  
  KTextEditor::EditInterface *iface = dynamic_cast<KTextEditor::EditInterface*>(part);
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


void AStylePart::activePartChanged(KParts::Part *part)
{
  bool enabled = false;

  KParts::ReadWritePart *rw_part = dynamic_cast<KParts::ReadWritePart*>(part);
  
  if (rw_part)
  {
    KTextEditor::EditInterface *iface = dynamic_cast<KTextEditor::EditInterface*>(rw_part);

    if (iface)
    {
      QString extension = rw_part->url().path();

      int pos = extension.findRev('.');
      if (pos >= 0)
        extension = extension.mid(pos);
      if (extension == ".h" || extension == ".c" || extension == ".java"
	  || extension == ".cpp" || extension == ".cc" || extension == ".C"
	  || extension == ".cxx" || extension == ".hxx")
	enabled = true;
    }
  }

  _action->setEnabled(enabled);
}


#include "astyle_part.moc"
