
#include "kdevplugin.h"
#include "kdevpartcontroller.h"
#include "kdevapi.h"

#include <dcopclient.h>
#include <qdom.h>

#include <assert.h>

struct KDevPlugin::Private
{
    DCOPClient *dcopClient;
    QCString name;
    QString pluginName;
    QString icon;
};

KDevPlugin::KDevPlugin( const QString& pluginName, const QString& icon, QObject *parent, const char *name)
    : QObject( parent, name ), d( new Private )
{
     assert( parent->inherits( "KDevApi" ) );
     m_api = static_cast<KDevApi *>( parent );

    d->name = name;
    d->icon = icon;
    d->pluginName = pluginName;
    d->dcopClient = 0L;
}

KDevPlugin::~KDevPlugin()
{
   delete( d->dcopClient );
   delete( d );
}

QString KDevPlugin::pluginName() const
{
    return d->name;
}

QString KDevPlugin::icon() const
{
    return d->icon;
}

DCOPClient* KDevPlugin::dcopClient() const
{
    if (!d->dcopClient)
    {
        d->dcopClient = new DCOPClient();
        d->dcopClient->registerAs(d->name, false);
    }

    return d->dcopClient;
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
