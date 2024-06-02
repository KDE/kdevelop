/*
    SPDX-FileCopyrightText: 2010 Benjamin Port <port.benjamin@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "test_qthelpplugin.h"
#include "../qthelpplugin.h"
#include "../qthelpprovider.h"
#include "../qthelp_config_shared.h"
#include "../qthelpnetwork.h"
#include "../qthelpdocumentation.h"

#include <QCoreApplication>
#include <QHelpLink>
#include <QRegularExpression>
#include <QTest>
#include <QSignalSpy>

#include <interfaces/idocumentation.h>
#include <language/duchain/duchainlock.h>
#include <language/duchain/declaration.h>
#include <language/duchain/types/identifiedtype.h>
#include <language/duchain/types/pointertype.h>
#include <tests/autotestshell.h>
#include <tests/testfile.h>

#include "testqthelpconfig.h"

const QString VALID1 = QTHELP_FILES "/valid1.qch";
const QString VALID2 = QTHELP_FILES "/valid2.qch";
const QString INVALID = QTHELP_FILES "/invalid.qch";

QTEST_MAIN(TestQtHelpPlugin)
using namespace KDevelop;

TestQtHelpPlugin::TestQtHelpPlugin()
    : m_testCore(nullptr)
    , m_plugin(nullptr)
{
}

void TestQtHelpPlugin::initTestCase()
{
    AutoTestShell::init({"kdevqthelp"});

    // Prevent SegFault, then "ICE default IO error handler doing an exit(), pid = <PID>, errno = 32"
    // crash when the test runs for at least 60 seconds. This is a workaround for QTBUG-58709.
    QCoreApplication::processEvents();

    m_testCore = new TestCore();
    m_testCore->initialize();
}

void TestQtHelpPlugin::init()
{
    m_plugin = new QtHelpPlugin(m_testCore, QVariantList());
    // write default config and read it
    qtHelpWriteConfig(QStringList(), QStringList(), QStringList(), QStringList(), QString(), true);
    m_plugin->readConfig();
    QTRY_VERIFY(m_plugin->isQtHelpQtDocLoaded());
}

void TestQtHelpPlugin::cleanup()
{
    delete m_plugin;
}

void TestQtHelpPlugin::cleanupTestCase()
{
    m_testCore->cleanup();
    delete m_testCore;
}

void TestQtHelpPlugin::testDefaultValue()
{
    QCOMPARE(m_plugin->isQtHelpQtDocLoaded(), true);
    QCOMPARE(m_plugin->qtHelpProviderLoaded().size(), 0);
    QCOMPARE(m_plugin->providers().size(), 1);

    auto provider = dynamic_cast<QtHelpProviderAbstract*>(m_plugin->providers().at(0));
    if (!provider->isValid()) {
        QSKIP("Qt help not available");
    }

    QTRY_VERIFY(provider->indexModel()->rowCount() > 0);
}

void TestQtHelpPlugin::testUnsetQtHelpDoc()
{
    qtHelpWriteConfig(QStringList(), QStringList(), QStringList(), QStringList(), QString(), false);
    m_plugin->readConfig();

    QCOMPARE(m_plugin->providers().size(), 0);
}

void TestQtHelpPlugin::testAddOneValidProvider()
{
    QStringList path, name, icon, ghns;
    path << VALID1;
    name << QStringLiteral("file1");
    icon << QStringLiteral("myIcon");
    ghns << QStringLiteral("0");
    qtHelpWriteConfig(icon, name, path, ghns, QString(), true);
    m_plugin->readConfig();

    QCOMPARE(m_plugin->qtHelpProviderLoaded().size(), 1);
    QCOMPARE(m_plugin->qtHelpProviderLoaded().at(0)->fileName(), path.at(0));
    QCOMPARE(m_plugin->qtHelpProviderLoaded().at(0)->name(), name.at(0));
    QCOMPARE(m_plugin->qtHelpProviderLoaded().at(0)->iconName(), icon.at(0));
}

void TestQtHelpPlugin::testAddTwoDifferentValidProvider()
{
    QStringList path, name, icon, ghns;
    path << VALID1 << VALID2;
    name << QStringLiteral("file1") << QStringLiteral("file2");
    icon << QStringLiteral("myIcon") << QStringLiteral("myIcon");
    ghns << QStringLiteral("0") << QStringLiteral("0");
    qtHelpWriteConfig(icon, name, path, ghns, QString(), true);
    m_plugin->readConfig();

    QCOMPARE(m_plugin->qtHelpProviderLoaded().size(), 2);
    // first provider
    QCOMPARE(m_plugin->qtHelpProviderLoaded().at(0)->fileName(), path.at(0));
    QCOMPARE(m_plugin->qtHelpProviderLoaded().at(0)->name(), name.at(0));
    QCOMPARE(m_plugin->qtHelpProviderLoaded().at(0)->iconName(), icon.at(0));
    // second provider
    QCOMPARE(m_plugin->qtHelpProviderLoaded().at(1)->fileName(), path.at(1));
    QCOMPARE(m_plugin->qtHelpProviderLoaded().at(1)->name(), name.at(1));
    QCOMPARE(m_plugin->qtHelpProviderLoaded().at(1)->iconName(), icon.at(1));
}

void TestQtHelpPlugin::testAddInvalidProvider()
{
    QStringList path, name, icon, ghns;
    path << INVALID;
    name << QStringLiteral("file1");
    icon << QStringLiteral("myIcon");
    ghns << QStringLiteral("0");
    qtHelpWriteConfig(icon, name, path, ghns, QString(), true);
    m_plugin->readConfig();

    QCOMPARE(m_plugin->qtHelpProviderLoaded().size(), 0);
}

void TestQtHelpPlugin::testAddTwiceSameProvider()
{
    QStringList path, name, icon, ghns;
    path << VALID1 << VALID1;
    name << QStringLiteral("file1") << QStringLiteral("file2");
    icon << QStringLiteral("myIcon") << QStringLiteral("myIcon");
    ghns << QStringLiteral("0") << QStringLiteral("0");
    qtHelpWriteConfig(icon, name, path, ghns, QString(), true);
    m_plugin->readConfig();

    QCOMPARE(m_plugin->qtHelpProviderLoaded().size(), 1);
}

void TestQtHelpPlugin::testRemoveOneProvider()
{
    QStringList path, name, icon, ghns;
    path << VALID1 << VALID2;
    name << QStringLiteral("file1") << QStringLiteral("file2");
    icon << QStringLiteral("myIcon") << QStringLiteral("myIcon");
    ghns << QStringLiteral("0") << QStringLiteral("0");
    qtHelpWriteConfig(icon, name, path, ghns, QString(), true);
    m_plugin->readConfig();

    QCOMPARE(m_plugin->qtHelpProviderLoaded().size(), 2);
    // we remove the second provider
    QtHelpProvider *provider = m_plugin->qtHelpProviderLoaded().at(0);
    path.removeAt(1);
    name.removeAt(1);
    icon.removeAt(1);
    ghns.removeAt(1);
    qtHelpWriteConfig(icon, name, path, ghns, QString(), true);
    m_plugin->readConfig();

    QCOMPARE(m_plugin->qtHelpProviderLoaded().size(), 1);
    QCOMPARE(m_plugin->qtHelpProviderLoaded().at(0), provider);
}

void TestQtHelpPlugin::testDeclarationLookup_Class()
{
    init();

    TestFile file(QStringLiteral("class QObject; QObject* o;"), QStringLiteral("cpp"));
    QVERIFY(file.parseAndWait());

    DUChainReadLocker lock;
    auto ctx = file.topContext();
    QVERIFY(ctx);
    auto decl = ctx->findDeclarations(QualifiedIdentifier(QStringLiteral("o"))).first();
    QVERIFY(decl);
    auto typeDecl = dynamic_cast<const IdentifiedType*>(decl->type<PointerType>()->baseType().data())->declaration(nullptr);
    QVERIFY(typeDecl);

    auto provider = dynamic_cast<QtHelpProviderAbstract*>(m_plugin->providers().at(0));
    QVERIFY(provider);
    if (!provider->isValid() || provider->engine()->documentsForIdentifier(QStringLiteral("QObject")).isEmpty()) {
        QSKIP("Qt help not available");
    }

    auto doc = provider->documentationForDeclaration(typeDecl);
    QVERIFY(doc);
    QCOMPARE(doc->name(), QStringLiteral("QObject"));
    const auto description = doc->description();
    QVERIFY(description.contains("QObject"));
}

void TestQtHelpPlugin::testDeclarationLookup_Method()
{
    init();

    TestFile file(QStringLiteral("class QString { static QString fromLatin1(const QByteArray&); };"), QStringLiteral("cpp"));
    QVERIFY(file.parseAndWait());

    DUChainReadLocker lock;
    auto ctx = file.topContext();
    QVERIFY(ctx);
    auto decl = ctx->findDeclarations(QualifiedIdentifier(QStringLiteral("QString"))).first();
    auto declFromLatin1 = decl->internalContext()->findDeclarations(QualifiedIdentifier(QStringLiteral("fromLatin1"))).first();
    QVERIFY(decl);

    auto provider = dynamic_cast<QtHelpProviderAbstract*>(m_plugin->providers().at(0));
    QVERIFY(provider);
    if (!provider->isValid()
        || provider->engine()->documentsForIdentifier(QStringLiteral("QString::fromLatin1")).isEmpty()) {
        QSKIP("Qt help not available");
    }

    auto doc = provider->documentationForDeclaration(declFromLatin1);
    QVERIFY(doc);
    QCOMPARE(doc->name(), QStringLiteral("QString::fromLatin1"));
    const auto description = doc->description();
    QVERIFY(description.contains(QRegularExpression{"See also.*toLatin1"}));
    QVERIFY(!description.contains("href"));
    QVERIFY(!description.contains("<p"));
    QVERIFY(!description.contains("<h3"));

    auto qtDoc = dynamic_cast<QtHelpDocumentation*>(doc.data());
    QVERIFY(qtDoc);
    QVERIFY(qtDoc->currentUrl().isValid());

    auto nam = provider->networkAccess();
    auto reply = nam->get(QNetworkRequest(qtDoc->currentUrl()));
    connect(reply, &QNetworkReply::readyRead, this, [reply]() {
        QVERIFY(!reply->readAll().isEmpty());
    });

    auto finishedSpy = QSignalSpy(reply, &QNetworkReply::finished);
    QVERIFY(finishedSpy.wait());

    QVERIFY(reply->isFinished());
    QCOMPARE(finishedSpy.size(), 1);
}

void TestQtHelpPlugin::testDeclarationLookup_OperatorFunction()
{
    init();

    TestFile file(QStringLiteral("class C {}; bool operator<(const C& a, const C& b) { return true; }"), QStringLiteral("cpp"));
    QVERIFY(file.parseAndWait());

    DUChainReadLocker lock;
    auto ctx = file.topContext();
    auto decl = ctx->findDeclarations(QualifiedIdentifier(QStringLiteral("operator<"))).first();
    QVERIFY(decl);

    auto provider = dynamic_cast<QtHelpProviderAbstract*>(m_plugin->providers().at(0));
    QVERIFY(provider);
    if (!provider->isValid() || provider->engine()->documentsForIdentifier(QStringLiteral("QObject")).isEmpty()) {
        QSKIP("Qt help not available");
    }

    auto doc = provider->documentationForDeclaration(decl);
    // TODO: We should never find a documentation entry for this (but instead, the operator< for QChar is found here)
    QEXPECT_FAIL("", "doc should be null here", Continue);
    QVERIFY(!doc);
}

#include "moc_test_qthelpplugin.cpp"
