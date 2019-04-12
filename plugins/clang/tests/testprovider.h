#ifndef KDEV_CLANG_TEST_PROVIDER_H
#define KDEV_CLANG_TEST_PROVIDER_H

#include <custom-definesandincludes/idefinesandincludesmanager.h>

#include <QHash>

class TestEnvironmentProvider final : public KDevelop::IDefinesAndIncludesManager::BackgroundProvider
{
public:
    ~TestEnvironmentProvider() override = default;
    QHash<QString, QString> definesInBackground(const QString& /*path*/) const override
    {
        return defines;
    }

    KDevelop::Path::List includesInBackground(const QString& /*path*/) const override
    {
        return includes;
    }

    KDevelop::Path::List frameworkDirectoriesInBackground(const QString&) const override
    {
        return {};
    }

    KDevelop::IDefinesAndIncludesManager::Type type() const override
    {
        return KDevelop::IDefinesAndIncludesManager::UserDefined;
    }

    QString parserArgumentsInBackground(const QString&) const override
    {
        return parserArguments;
    }

    void clear()
    {
        defines.clear();
        includes.clear();
        parserArguments.clear();
    }

    QHash<QString, QString> defines;
    KDevelop::Path::List includes;
    QString parserArguments;
};

#endif // KDEV_CLANG_TEST_PROVIDER_H

