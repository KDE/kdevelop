/*
    SPDX-FileCopyrightText: 2021 Igor Kushnir <igorkuo@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "languagecontrollertestbase.h"

#include "testfilepaths.h"

#include <interfaces/ilanguagecontroller.h>
#include <interfaces/iplugincontroller.h>
#include <language/backgroundparser/backgroundparser.h>
#include <languagecontroller.h>
#include <shell/core.h>
#include <tests/autotestshell.h>
#include <tests/testcore.h>
#include <tests/testhelpers.h>
#include <vcs/interfaces/ipatchsource.h>

#include <QByteArray>
#include <QFileInfo>
#include <QString>
#include <QTest>
#include <QUrl>

using namespace KDevelop;

namespace {
QUrl testUrl(const QString& filename)
{
    return QUrl::fromLocalFile(filename);
}

QUrl existentTestUrl(const QString& filename)
{
    const QString filePath = TEST_FILES_DIR "/" + filename;
    QVERIFY_RETURN(QFileInfo::exists(filePath), QUrl{});
    return QUrl::fromLocalFile(filePath);
}
}

LanguageControllerTestBase::LanguageControllerTestBase(QObject* parent)
    : QObject(parent)
    , m_differentLanguagesUrls{
          {testUrl("plus.cc"), "clang"},
          {testUrl("project.cmake"), "CMake"},
          {testUrl("patch.diff"), "diff"},
          // TODO: re-enable once we have a working QML plugin again
          // {testUrl("gui.qml"), "qml/js"},
      }
{
}

void LanguageControllerTestBase::initTestCase()
{
    AutoTestShell::init({"kdevclangsupport", "kdevpatchreview", "kdevqmljs",
                         "KDevCMakeManager", "KDevCMakeBuilder", "KDevMakeBuilder", "KDevStandardOutputView"});
    TestCore::initialize();
    m_subject = Core::self()->languageController();

    m_havePatchReview = ICore::self()->pluginController()->extensionForPlugin<IPatchReview>() != nullptr;
    if (!m_havePatchReview) {
        const auto it = m_differentLanguagesUrls.cbegin() + 2;
        QCOMPARE(it->languageName, "diff");
        m_differentLanguagesUrls.erase(it);
    }
}

void LanguageControllerTestBase::init()
{
    Core::self()->languageControllerInternal()->initialize();
    m_subject->backgroundParser()->suspend();
}

void LanguageControllerTestBase::cleanup()
{
    Core::self()->languageControllerInternal()->cleanup();
}

void LanguageControllerTestBase::cleanupTestCase()
{
    TestCore::shutdown();
}

void LanguageControllerTestBase::fillLanguageControllerMimeTypeCache() const
{
    for (const auto& url : m_differentLanguagesUrls) {
        const auto languages = m_subject->languagesForUrl(url.url);
        QCOMPARE(languages.size(), 1);
        QCOMPARE(languages.back()->name(), url.languageName);
    }
}

void LanguageControllerTestBase::matchingLanguagesForUrlInBackgroundThreadTestData() const
{
    QTest::addColumn<QUrl>("url");
    QTest::addColumn<QString>("languageName");

    QTest::newRow("CMakeLists") << testUrl("CMakeLists.txt") << "CMake";
    QTest::newRow("cmakelists wrong case") << testUrl("cmakelists.TXT") << "CMake";

    QTest::newRow("lower-case") << testUrl("x.cpp") << "clang";
    QTest::newRow("upper-case") << testUrl("Y.CPP") << "clang";
    QTest::newRow("mixed-case") << testUrl("aBc.CpP") << "clang";

    QTest::newRow(".C") << testUrl("ambiguous.C") << "clang";
    QTest::newRow(".cl") << testUrl("Open.cl") << "clang";

    QTest::newRow("existent C with extension") << existentTestUrl("t.c") << "clang";

    for (const auto& url : m_differentLanguagesUrls) {
        const auto filename = url.url.fileName();
        const auto extension = filename.mid(filename.lastIndexOf('.'));
        QTest::newRow(extension.toUtf8().constData()) << url.url << url.languageName;
    }
}

void LanguageControllerTestBase::matchingLanguagesForUrlTestData() const
{
    matchingLanguagesForUrlInBackgroundThreadTestData();

    QTest::newRow("existent C w/o extension") << existentTestUrl("X") << "clang";
    if (m_havePatchReview) {
        QTest::newRow("existent patch w/o extension") << existentTestUrl("y") << "diff";
    }
}

void LanguageControllerTestBase::nonmatchingLanguagesForUrlTestData()
{
    QTest::addColumn<QUrl>("url");

    QTest::newRow("empty") << testUrl(QString());
    QTest::newRow("archive") << testUrl("a.tar.gz");
    QTest::newRow("OpenDocument Text") << testUrl("b.odt");
    QTest::newRow("existent archive with extension") << existentTestUrl("N.tar.gz");
    QTest::newRow("existent archive w/o extension") << existentTestUrl("z");
}

#include "moc_languagecontrollertestbase.cpp"
