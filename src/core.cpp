#include <qtimer.h>


#include <kapplication.h>
#include <kdebug.h>
#include <kstatusbar.h>
#include <kmainwindow.h>
#include <kconfig.h>
#include <kdeversion.h>
#include <kstandarddirs.h>
#include <kglobal.h>

#include "toplevel.h"
#include "partcontroller.h"
#include "api.h"
#include "projectmanager.h"
#include "kdevlicense.h"

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

void Core::insertNewAction( KAction* action )
{
    Q_UNUSED( action );
}


bool Core::queryClose()
{
  // save the the project to open it automaticly on startup if needed
  KConfig* config = kapp->config();
  config->setGroup("General Options");
  config->writePathEntry("Last Project",ProjectManager::getInstance()->projectFile().url());
  
//  if ( !PartController::getInstance()->querySaveFiles() )
  if ( !PartController::getInstance()->closeAllFiles() )
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

QDict< KDevLicense > Core::licenses()
{
	return m_licenses;
}

void Core::loadLicenses()
{
	// kdDebug(9010) << "======================== Entering loadLicenses" << endl;
	KStandardDirs* dirs = KGlobal::dirs();
	dirs->addResourceType( "licenses", KStandardDirs::kde_default( "data" ) + "kdevelop/licenses/" );
	QStringList licNames = dirs->findAllResources( "licenses", QString::null, false, true );
	
	QStringList::Iterator it;
	for (it = licNames.begin(); it != licNames.end(); ++it)
	{
		QString licPath( dirs->findResource( "licenses", *it ) );
		// kdDebug(9000) << "Loading license file: " << licPath << endl;
		QString licName = licPath.mid( licPath.findRev('/') + 1 );
		KDevLicense* lic = new KDevLicense( licName, licPath );
		m_licenses.insert( licName, lic );
	}
	// kdDebug(9000) << "======================== Done loadLicenses" << endl;
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
