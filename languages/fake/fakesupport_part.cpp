
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

FakeLanguageSupport::Features FakeLanguageSupport::features() const
{
  return Features();
}

KMimeType::List FakeLanguageSupport::mimeTypes() const
{
  return KMimeType::List();
}

QString FakeLanguageSupport::formatTag(const Tag& tag) const
{
  return QString();
}

QString FakeLanguageSupport::formatModelItem(const KDevCodeItem *item, bool shortDescription) const
{
  return QString();
}

QString FakeLanguageSupport::formatClassName(const QString &name) const
{
  return QString();
}

QString FakeLanguageSupport::unformatClassName(const QString &name) const
{
  return QString();
}

#include "fakesupport_part.moc"
