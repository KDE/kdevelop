#include <qdom.h>

#include "kdevplugin.h"
#include "kdevpartcontroller.h"


#include "kdevapi.h"

#include <assert.h>

KDevPlugin::KDevPlugin(QObject *parent, const char *name)
    : QObject(parent, name)
{
   assert( parent->inherits( "KDevApi" ) );
   m_api = static_cast<KDevApi *>( parent );
}

KDevPlugin::~KDevPlugin()
{
}

KDevMainWindow *KDevPlugin::mainWindow()
{
    return m_api->mainWindow();
}

bool KDevPlugin::mainWindowValid()
{
    return m_api->mainWindowValid();
}

KDevCore *KDevPlugin::core()
{
    return m_api->core();
}

KDevProject *KDevPlugin::project()
{
    return m_api->project();
}

ClassStore *KDevPlugin::classStore()
{
    return m_api->classStore();
}

/* added by daniel */
ClassStore* KDevPlugin::ccClassStore( )
{
    return m_api->ccClassStore();
}

QDomDocument *KDevPlugin::projectDom() const
{
    return m_api->projectDom();
}

KDevVersionControl *KDevPlugin::versionControl()
{
    return m_api->versionControl();
}

KDevLanguageSupport *KDevPlugin::languageSupport()
{
    return m_api->languageSupport();
}

KDevMakeFrontend *KDevPlugin::makeFrontend()
{
    return m_api->makeFrontend();
}

KDevDiffFrontend *KDevPlugin::diffFrontend()
{
    return m_api->diffFrontend();
}

KDevAppFrontend *KDevPlugin::appFrontend()
{
    return m_api->appFrontend();
}

KDevPartController *KDevPlugin::partController()
{
    return m_api->partController();
}

KDevDebugger *KDevPlugin::debugger()
{
    return m_api->debugger();
}

KDevCreateFile *KDevPlugin::createFileSupport()
{
    return m_api->createFile();
}

void KDevPlugin::restorePartialProjectSession(const QDomElement* /*el*/)
{
  // there's still nothing to do in the base class
}

void KDevPlugin::savePartialProjectSession(QDomElement* /*el*/)
{
  // there's still nothing to do in the base class
}

#include "kdevplugin.moc"
