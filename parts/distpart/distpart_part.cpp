#include <qwhatsthis.h>


#include <kiconloader.h>
#include <klocale.h>
#include <kgenericfactory.h>

#include <kdevcore.h>


#include "distpart_widget.h"
#include "distpart_part.h"

typedef KGenericFactory<DistpartPart> DistpartFactory;
K_EXPORT_COMPONENT_FACTORY( libkdevdistpart, DistpartFactory( "kdevdistpart" ) );

DistpartPart::DistpartPart(QObject *parent, const char *name, const QStringList &)
  : KDevPlugin(parent, name)
{
  setInstance(DistpartFactory::instance());
 
  setXMLFile("kdevpart_distpart.rc");

  m_action =  new KAction( i18n("Project Distribution and Publishing"), "package", 0,
                          this, SLOT(show()),
                          actionCollection(), "make_dist" );

  m_action->setStatusText(i18n("Make Source and Binary Distribution"));
  m_action->setWhatsThis(i18n("Distribution and Publishing:\n\n"
                 	    "fnork fnork blub.... \n"
                            "bork bork bork....."));
  //QWhatsThis::add(m_widget, i18n("This will help users package and publish there software."));

  m_dialog = 0;

  // set up package
  KURL projectURL;  // we need to get this from the base project
  thePackage = new packageBase();
  thePackage->loadFile(projectURL);
}


DistpartPart::~DistpartPart()
{
  delete m_dialog;
}


void DistpartPart::show()
{
  if (!m_dialog)
    m_dialog = new DistpartDialog(this);

  m_dialog->show();
}

#include "distpart_part.moc"
