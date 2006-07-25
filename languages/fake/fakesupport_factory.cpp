
#include <kinstance.h>
#include <kstandarddirs.h>
#include <kdevplugininfo.h>
#include "fakesupport_factory.h"

K_EXPORT_COMPONENT_FACTORY( libkdevfakesupport, FakeSupportFactory )

static const KDevPluginInfo data( "kdevfakesupport" );

FakeSupportFactory::FakeSupportFactory()
        : KDevGenericFactory<FakeLanguageSupport>( data )
{}

KInstance *FakeSupportFactory::createInstance()
{
    KInstance * instance = KDevGenericFactory<FakeLanguageSupport>::createInstance();
    KStandardDirs *dirs = instance->dirs();
    dirs->addResourceType( "newclasstemplates", KStandardDirs::kde_default( "data" ) + "kdevfakesupport/newclass/" );
    dirs->addResourceType( "pcs", KStandardDirs::kde_default( "data" ) + "kdevfakesupport/pcs/" );

    return instance;
}

const KDevPluginInfo * FakeSupportFactory::info()
{
    return & data;
}

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
