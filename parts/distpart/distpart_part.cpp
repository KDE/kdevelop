#include <qwhatsthis.h>


#include <kiconloader.h>
#include <klocale.h>


#include <kdevcore.h>


#include "distpart_widget.h"
#include "distpart_factory.h"
#include "distpart_part.h"


distpartPart::distpartPart(KDevApi *api, QObject *parent, const char *name)
  : KDevPart(api, parent, name)
{
  setInstance(distpartFactory::instance());
 
  setXMLFile("kdevpart_distpart.rc");

  m_action =  new KAction( i18n("Project Distrabution and Publishing"), "package", 1,
                          this, SLOT(show()),
                          actionCollection(), "make_dist" );

  m_action->setStatusText(i18n("Make Source and Binary Distrabuition"));
  m_action->setWhatsThis(i18n("Distrabution and Publishing:\n\n"
                 	    "fnork fnork blub.... \n"
                            "bork bork bork....."));
  m_widget = new distpartWidget(this);

  QWhatsThis::add(m_widget, i18n("This will help users package and publish there software."));

}


distpartPart::~distpartPart()
{
  delete m_widget;
}


void distpartPart::show()
{
// configure the widget

  m_widget->show();
//  core()->embedWidget(m_widget, KDevCore::SelectView, i18n("distpart"));
}
#include "distpart_part.moc"
