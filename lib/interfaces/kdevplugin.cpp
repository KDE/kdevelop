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

KDevTopLevel *KDevPlugin::topLevel()
{
    return m_api->topLevel();
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

QDomDocument *KDevPlugin::projectDom()
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

#include "kdevplugin.moc"
