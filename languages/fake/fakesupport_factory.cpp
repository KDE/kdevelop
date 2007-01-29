
#include <kcomponentdata.h>
#include <kstandarddirs.h>
#include <kdevplugininfo.h>
#include "fakesupport_factory.h"

K_EXPORT_COMPONENT_FACTORY( libkdevfakesupport, FakeSupportFactory )

static const Koncrete::PluginInfo data( "kdevfakesupport" );

FakeSupportFactory::FakeSupportFactory()
        : KDevGenericFactory<FakeLanguageSupport>( data )
{}

KComponentData *FakeSupportFactory::createComponentData()
{
    KComponentData *componentData = KDevGenericFactory<FakeLanguageSupport>::createComponentData();
    KStandardDirs *dirs = componentData->dirs();
    dirs->addResourceType( "newclasstemplates", KStandardDirs::kde_default( "data" ) + "kdevfakesupport/newclass/" );
    dirs->addResourceType( "pcs", KStandardDirs::kde_default( "data" ) + "kdevfakesupport/pcs/" );

    return componentData;
}

const Koncrete::PluginInfo * FakeSupportFactory::info()
{
    return & data;
}

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
