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
  m_widget = new distpartWidget(this);

  QWhatsThis::add(m_widget, i18n("This will help users package and publish there software."));
  KAction *action;

  action = new KAction( i18n("Project Distrabution and Publishing"), 0,
                          m_widget, SLOT(show()),
                          actionCollection(), "make_dist" );
  core()->embedWidget(m_widget, KDevCore::OutputView, i18n("distpart"));
}


distpartPart::~distpartPart()
{
  delete m_widget;
}


#include "distpart_part.moc"
