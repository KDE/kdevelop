#include <kiconloader.h>
#include <klocale.h>


#include "kdevcore.h"
#include "konsoleviewwidget.h"
#include "konsoleviewfactory.h"
#include "konsoleviewpart.h"


KonsoleViewPart::KonsoleViewPart(KDevApi *api, QObject *parent, const char *name)
  : KDevPart(api, parent, name)
{
  connect( core(), SIGNAL(projectOpened()), this, SLOT(projectOpened()) );
  connect( core(), SIGNAL(projectClosed()), this, SLOT(projectClosed()) );

  m_widget = new KonsoleViewWidget(this);
  m_widget->setIcon(SmallIcon("konsole"));
  m_widget->setCaption(i18n("Embedded Konsole"));
/*    QWhatsThis::add(m_widget, i18n("Grep\n\n"
                                   "This window contains the output of a grep "
                                   "command. Clicking on an item in the list "
                                   "will automatically open the corresponding "
                                   "source file and set the cursor to the line "
                                   "with the match."));
*/
  core()->embedWidget(m_widget, KDevCore::OutputView, i18n("Konsole"));
}


KonsoleViewPart::~KonsoleViewPart()
{
  delete m_widget;
}


void KonsoleViewPart::projectOpened()
{
}


void KonsoleViewPart::projectClosed()
{
}


#include "konsoleviewpart.moc"
