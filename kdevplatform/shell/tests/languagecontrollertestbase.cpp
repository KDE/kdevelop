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

#include <QByteArray>
#include <QFileInfo>
#include <QMimeDatabase>
#include <QString>
#include <QTest>
#include <QtLogging>
#include <QUrl>

#include <algorithm>

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
        {testUrl("gui.qml"), "qml/js"},
    }
{
}

void LanguageControllerTestBase::initTestCase()
{
    AutoTestShell::init({"kdevclangsupport", "kdevpatchreview", "kdevqmljs",
                         "KDevCMakeManager", "KDevCMakeBuilder", "KDevMakeBuilder", "KDevStandardOutputView"});
    TestCore::initialize();
    m_subject = Core::self()->languageController();

    // Remove entries for languages supported by unavailable optional plugins.
    for (auto it = m_differentLanguagesUrls.cbegin(); it != m_differentLanguagesUrls.cend();) {
        const auto mimeType = QMimeDatabase().mimeTypeForFile(it->url.fileName(), QMimeDatabase::MatchExtension);
        QVERIFY(mimeType.isValid());
        QVERIFY(!mimeType.isDefault());

        const QVariantMap constraints{{"X-KDevelop-SupportedMimeTypes", mimeType.name()}};
        const bool languagePluginLoaded =
            !ICore::self()->pluginController()->queryExtensionPlugins("ILanguageSupport", constraints).empty();
        if (languagePluginLoaded) {
            ++it;
        } else {
            qWarning() << "expected language plugin" << it->languageName << "for MIME type" << mimeType.name()
                       << "has not been loaded => skipping test data rows for this language";
            it = m_differentLanguagesUrls.erase(it);
        }
    }

    if (m_differentLanguagesUrls.empty()) {
        QSKIP("zero language plugins => skip the entire test to prevent an assertion failure in QTest::fetchData()");
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

    newOptionalRow("CMakeLists", testUrl("CMakeLists.txt"), "CMake");
    newOptionalRow("cmakelists wrong case", testUrl("cmakelists.TXT"), "CMake");

    newOptionalRow("lower-case", testUrl("x.cpp"), "clang");
    newOptionalRow("upper-case", testUrl("Y.CPP"), "clang");
    newOptionalRow("mixed-case", testUrl("aBc.CpP"), "clang");

    newOptionalRow(".C", testUrl("ambiguous.C"), "clang");
    newOptionalRow(".cl", testUrl("Open.cl"), "clang");

    newOptionalRow("existent C with extension", existentTestUrl("t.c"), "clang");

    for (const auto& url : m_differentLanguagesUrls) {
        const auto filename = url.url.fileName();
        const auto extension = filename.mid(filename.lastIndexOf('.'));
        newOptionalRow(extension.toUtf8().constData(), url.url, url.languageName);
    }
}

void LanguageControllerTestBase::matchingLanguagesForUrlTestData() const
{
    matchingLanguagesForUrlInBackgroundThreadTestData();

    newOptionalRow("existent C w/o extension", existentTestUrl("X"), "clang");
    newOptionalRow("existent patch w/o extension", existentTestUrl("y"), "diff");
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

void LanguageControllerTestBase::newOptionalRow(const char* dataTag, const QUrl& url, const QString& languageName) const
{
    const bool languagePluginLoaded = std::any_of(m_differentLanguagesUrls.cbegin(), m_differentLanguagesUrls.cend(),
                                                  [&languageName](const UrlEntry& entry) {
                                                      return entry.languageName == languageName;
                                                  });
    if (languagePluginLoaded) {
        QTest::newRow(dataTag) << url << languageName;
    } else {
        qWarning() << "skipping test data row because its language plugin is unavailable:" << dataTag << url.fileName()
                   << languageName;
    }
}

#include "moc_languagecontrollertestbase.cpp"
