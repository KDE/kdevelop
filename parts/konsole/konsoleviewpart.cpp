#include <qwhatsthis.h>


#include <kiconloader.h>
#include <klocale.h>


#include "kdevcore.h"
#include "konsoleviewwidget.h"
#include "konsoleviewfactory.h"
#include "konsoleviewpart.h"


KonsoleViewPart::KonsoleViewPart(KDevApi *api, QObject *parent, const char *name)
  : KDevPart(api, parent, name)
{
  m_widget = new KonsoleViewWidget(this);
  
    QWhatsThis::add(m_widget, i18n("Konsole\n\n"
                                   "This window contains an embedded konsole "
				   "window. It will try to follow you when "
				   "you navigate in the source directories"));
  
  core()->embedWidget(m_widget, KDevCore::OutputView, i18n("Konsole"));
}


KonsoleViewPart::~KonsoleViewPart()
{
  delete m_widget;
}


#include "konsoleviewpart.moc"
