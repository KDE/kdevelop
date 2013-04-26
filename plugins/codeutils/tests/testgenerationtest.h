/*
 *
 */

#ifndef KDEVPLATFORM_PLUGIN_TESTGENERATIONTEST_H
#define KDEVPLATFORM_PLUGIN_TESTGENERATIONTEST_H

#include <QtTest/QtTest>
#include <KUrl>
#include <KTempDir>

namespace KDevelop
{
class TemplateRenderer;
class SourceFileTemplate;
}

class TestGenerationTest : public QObject
{
    Q_OBJECT
private slots:
    void initTestCase();
    void cleanupTestCase();

    void init();

    void yamlTemplate();
    void cppTemplate();

private:
    KDevelop::TemplateRenderer* renderer;
    QScopedPointer<KTempDir> dir;
    QUrl baseUrl;

    QHash<QString,KUrl> urls(const KDevelop::SourceFileTemplate& file);
};

#endif // KDEVPLATFORM_PLUGIN_TESTGENERATIONTEST_H
