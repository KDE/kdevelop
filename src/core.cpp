#include <QTimer>
#include <qmenu.h>

#include <kapplication.h>
#include <kdebug.h>
#include <kstatusbar.h>
#include <kmainwindow.h>
#include <kconfig.h>
#include <kdeversion.h>
#include <kstandarddirs.h>

#include "toplevel.h"
#include "projectcontroller.h"
#include "documentcontroller.h"
#include "kdevapi.h"
#include "kdevconfig.h"

#include "core.h"


Core *Core::s_instance = 0;


Core *Core::getInstance()
{
  if (!s_instance)
    s_instance = new Core;
  return s_instance;
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
    if ( !ProjectController::getInstance()->closeProject() )
        return false;

    if ( !DocumentController::getInstance()->querySaveDocuments() )
      return false;

    if ( !DocumentController::getInstance()->readyToClose() )
      return false;

  return true;
}


void Core::running(KDevPlugin * which, bool runs)
{
  emit activeProcessChanged( which, runs );
}


void Core::fillContextMenu(QMenu *popup, const Context *context)
{
  emit contextMenu(popup, context);
}


void Core::openProject(const KUrl& projectFileName)
{
    ProjectController::getInstance()->openProject( projectFileName );
}

KUrl Core::localFile() const
{
    return ProjectController::getInstance()->localFile();
}

KUrl Core::globalFile() const
{
    return ProjectController::getInstance()->globalFile();
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

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
