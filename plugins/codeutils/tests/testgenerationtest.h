/*
 *
 */

#ifndef TESTGENERATIONTEST_H
#define TESTGENERATIONTEST_H

#include <QtTest/QtTest>
#include <KUrl>

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
    KUrl baseUrl;

    QHash<QString,KUrl> urls(const KDevelop::SourceFileTemplate& file);
};

#endif // TESTGENERATIONTEST_H
