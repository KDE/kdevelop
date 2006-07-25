
#ifndef FAKESUPPORT_FACTORY_H
#define FAKESUPPORT_FACTORY_H

#include <kdevgenericfactory.h>
#include "fakesupport_part.h"

class KDevPluginInfo;

class FakeSupportFactory : public KDevGenericFactory<FakeLanguageSupport>
{
public:
    FakeSupportFactory();

    static const KDevPluginInfo *info();

protected:
    virtual KInstance *createInstance();
};

#endif // FAKESUPPORT_FACTORY_H

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on

