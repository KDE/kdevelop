
#include "fakesupport_part.h"
#include "fakesupport_factory.h"

#include <kdevcodemodel.h>

#include <QtCore/QMetaObject>
#include <QtCore/QTimer>
#include <QtCore/qdebug.h>

FakeLanguageSupport::FakeLanguageSupport(QObject *parent, const char *name, const QStringList &)
  : KDevLanguageSupport(FakeSupportFactory::info(), parent)
{
  setObjectName(QString::fromUtf8(name));
}

FakeLanguageSupport::~FakeLanguageSupport()
{
}

int FakeLanguageSupport::features() const
{
  return 0;
}

QStringList FakeLanguageSupport::mimeTypes() const
{
  return QStringList();
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

void FakeLanguageSupport::test_codemodel()
{
}

#include "fakesupport_part.moc"
