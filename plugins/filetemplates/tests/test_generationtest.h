/*
 *
 */

#ifndef KDEVPLATFORM_PLUGIN_TEST_GENERATIONTEST_H
#define KDEVPLATFORM_PLUGIN_TEST_GENERATIONTEST_H

#include <QtTest/QtTest>
#include <QUrl>
#include <QTemporaryDir>

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
    QScopedPointer<QTemporaryDir> dir;
    QUrl baseUrl;

    QHash<QString,QUrl> urls(const KDevelop::SourceFileTemplate& file);
};

#endif // KDEVPLATFORM_PLUGIN_TEST_GENERATIONTEST_H
