#include <qwhatsthis.h>


#include <kiconloader.h>
#include <klocale.h>


#include <kdevcore.h>


#include "$APPNAMELC$_widget.h"
#include "$APPNAMELC$_factory.h"
#include "$APPNAMELC$_part.h"


$APPNAME$Part::$APPNAME$Part(KDevApi *api, QObject *parent, const char *name)
  : KDevPart(api, parent, name)
{
  m_widget = new $APPNAME$Widget(this);
  
  QWhatsThis::add(m_widget, i18n("WHAT DOES THIS PART DO?"));
  
  core()->embedWidget(m_widget, KDevCore::OutputView, i18n("$APPNAME$"));
}


$APPNAME$Part::~$APPNAME$Part()
{
  delete m_widget;
}


#include "$APPNAMELC$_part.moc"
