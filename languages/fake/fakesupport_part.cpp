
#include "fakesupport_part.h"

FakeLanguageSupport::FakeLanguageSupport(QObject *parent, const char *name, const QStringList &)
    : KDevLanguageSupport(info(), parent)
{
    setObjectName(QString::fromUtf8(name));
}

FakeLanguageSupport::~FakeLanguageSupport()
{
}


