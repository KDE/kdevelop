#include <qtimer.h>


#include <kapplication.h>
#include <kdebug.h>
#include <kstatusbar.h>
#include <kmainwindow.h>
#include <kconfig.h>
#include <kdeversion.h>
#include <kstandarddirs.h>
#include <kglobal.h>
#include <kactioncollection.h>

#include "toplevel.h"
#include "partcontroller.h"
#include "api.h"
#include "projectmanager.h"

#include "core.h"


Core *Core::s_instance = 0;


Core *Core::getInstance()
{
  if (!s_instance)
    s_instance = new Core;
  return s_instance;
}

void Core::setupShourtcutTips(KXMLGUIClient * client)
{
  QPtrList<KXMLGUIClient> clients;
  if (client != 0)
    clients.append(client);
  else
    clients = TopLevel::getInstance()->main()->guiFactory()->clients();
  
  for( QPtrListIterator<KXMLGUIClient> it(clients); it.current(); ++it ) {
    KActionCollection *actionCollection = (*it)->actionCollection();
    for (int i = 0; i < actionCollection->count(); i++) {
      KAction *action = actionCollection->action(i);
            
      QString tooltip = action->toolTip();
      if (tooltip.isEmpty())
        tooltip = action->text().remove('&');
      else {
        int i = tooltip.findRev('(');
        if (i > 0) tooltip = tooltip.left(i).stripWhiteSpace();
      }

      QString shortcut = action->shortcutText();
      if (!shortcut.isEmpty())
        tooltip += " (" + shortcut + ")";
        action->setToolTip(tooltip);
      }
  }
}

Core::Core()
  : KDevCore()
{
}


Core::~Core()
{
}

bool Core::queryClose()
{
  // save the the project to open it automaticly on startup if needed
  KConfig* config = kapp->config();
  config->setGroup("General Options");
  config->writePathEntry("Last Project",ProjectManager::getInstance()->projectFile().url());
  
  if ( !PartController::getInstance()->querySaveFiles() )
	  return false;
  
  if ( !ProjectManager::getInstance()->closeProject( true ) )
      return false;
  
  if ( !PartController::getInstance()->readyToClose() )
      return false;
  
  return true;
}


void Core::running(KDevPlugin * which, bool runs)
{
  emit activeProcessChanged( which, runs );
}


void Core::fillContextMenu(QPopupMenu *popup, const Context *context)
{
  emit contextMenu(popup, context);
}


void Core::openProject(const QString& projectFileName)
{
  ProjectManager::getInstance()->loadProject(KURL( projectFileName ));
}

namespace MainWindowUtils{
QString beautifyToolTip(const QString& text)
{
    QString temp = text;
    temp.replace(QRegExp("&"), "");
    temp.replace(QRegExp("\\.\\.\\."), "");
    return temp;
}
}

#include "core.moc"
