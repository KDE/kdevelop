#include <qwhatsthis.h>


#include <kiconloader.h>
#include <klocale.h>


#include <kdevcore.h>


#include "sourcebrowser_widget.h"
#include "sourcebrowser_factory.h"
#include "sourcebrowser_part.h"


SourceBrowserPart::SourceBrowserPart(KDevApi *api, QObject *parent, const char *name)
  : KDevPart(api, parent, name)
{
  m_widget = new SourceBrowserWidget(this);
  
  QWhatsThis::add(m_widget, i18n("Browse source files"));
  
  core()->embedWidget(m_widget, KDevCore::OutputView, i18n("SourceBrowser"));
}


SourceBrowserPart::~SourceBrowserPart()
{
  delete m_widget;
}


#include "sourcebrowser_part.moc"
