
#include "fakesupport_part.h"
#include "fakesupport_factory.h"

FakeLanguageSupport::FakeLanguageSupport(QObject *parent, const char *name, const QStringList &)
    : KDevLanguageSupport(FakeSupportFactory::info(), parent)
{
    setObjectName(QString::fromUtf8(name));
}

FakeLanguageSupport::~FakeLanguageSupport()
{
}


