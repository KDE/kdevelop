/*  This file is part of KDevelop
    Copyright 2010 Benjamin Port <port.benjamin@gmail.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "testqthelpplugin.h"
#include "../qthelpplugin.h"
#include "../qthelpprovider.h"
#include "../qthelp_config_shared.h"

#include <interfaces/idocumentation.h>
#include <language/duchain/duchainlock.h>
#include <language/duchain/declaration.h>
#include <language/duchain/types/identifiedtype.h>
#include <language/duchain/types/pointertype.h>
#include <tests/autotestshell.h>
#include <tests/testfile.h>

#include <kdebug.h>

#include "testqthelpconfig.h"

const QString VALID1 = QTHELP_FILES + QString("/valid1.qch");
const QString VALID2 = QTHELP_FILES + QString("/valid2.qch");
const QString INVALID = QTHELP_FILES + QString("/invalid.qch");

QTEST_MAIN(TestQtHelpPlugin)
using namespace KDevelop;

TestQtHelpPlugin::TestQtHelpPlugin()
{
}

void TestQtHelpPlugin::initTestCase()
{
    KDevelop::AutoTestShell::init();
    m_testCore = new KDevelop::TestCore();
    m_testCore->initialize(KDevelop::Core::NoUi);
}

void TestQtHelpPlugin::init()
{
    m_plugin = new QtHelpPlugin(m_testCore, QVariantList());
    // write default config and read it
    qtHelpWriteConfig(QStringList(), QStringList(), QStringList(), QStringList(), QString(), true);
    m_plugin->readConfig();
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
    QCOMPARE(m_plugin->qtHelpQtDocLoaded(), true);
    QCOMPARE(m_plugin->qtHelpProviderLoaded().size(), 0);
    QCOMPARE(m_plugin->providers().size(), 1);
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
    name << "file1";
    icon << "myIcon";
    ghns << "0";
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
    name << "file1" << "file2";
    icon << "myIcon" << "myIcon";
    ghns << "0" << "0";
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
    name << "file1";
    icon << "myIcon";
    ghns << "0";
    qtHelpWriteConfig(icon, name, path, ghns, QString(), true);
    m_plugin->readConfig();

    QCOMPARE(m_plugin->qtHelpProviderLoaded().size(), 0);
}

void TestQtHelpPlugin::testAddTwiceSameProvider()
{
    QStringList path, name, icon, ghns;
    path << VALID1 << VALID1;
    name << "file1" << "file2";
    icon << "myIcon" << "myIcon";
    ghns << "0" << "0";
    qtHelpWriteConfig(icon, name, path, ghns, QString(), true);
    m_plugin->readConfig();

    QCOMPARE(m_plugin->qtHelpProviderLoaded().size(), 1);
}

void TestQtHelpPlugin::testRemoveOneProvider()
{
    QStringList path, name, icon, ghns;
    path << VALID1 << VALID2;
    name << "file1" << "file2";
    icon << "myIcon" << "myIcon";
    ghns << "0" << "0";
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

    TestFile file("class QObject; QObject* o;", "cpp");
    QVERIFY(file.parseAndWait());

    DUChainReadLocker lock;
    auto ctx = file.topContext();
    auto decl = ctx->findDeclarations(QualifiedIdentifier("o")).first();
    QVERIFY(decl);
    auto typeDecl = dynamic_cast<const IdentifiedType*>(decl->type<PointerType>()->baseType().unsafeData())->declaration(0);
    QVERIFY(typeDecl);

    auto provider = dynamic_cast<QtHelpProviderAbstract*>(m_plugin->providers().at(0));
    QVERIFY(provider);
    if (!provider->isValid()) {
        QSKIP("Qt help not available", SkipSingle);
    }

    auto doc = provider->documentationForDeclaration(typeDecl);
    QVERIFY(doc);
    QCOMPARE(doc->name(), QString("QObject"));
    QVERIFY(doc->description().contains("The QObject class"));
}

void TestQtHelpPlugin::testDeclarationLookup_OperatorFunction()
{
    init();

    TestFile file("class C {}; bool operator<(const C& a, const C& b) { return true; }", "cpp");
    QVERIFY(file.parseAndWait());

    DUChainReadLocker lock;
    auto ctx = file.topContext();
    auto decl = ctx->findDeclarations(QualifiedIdentifier("operator<")).first();
    QVERIFY(decl);

    auto provider = dynamic_cast<QtHelpProviderAbstract*>(m_plugin->providers().at(0));
    QVERIFY(provider);
    if (!provider->isValid()) {
        QSKIP("Qt help not available", SkipSingle);
    }

    auto doc = provider->documentationForDeclaration(decl);
    // TODO: We should never find a documentation entry for this (but instead, the operator< for QChar is found here)
    QEXPECT_FAIL("", "doc should be null here", Continue);
    QVERIFY(!doc);
}
