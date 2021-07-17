/*
    SPDX-FileCopyrightText: 2021 Igor Kushnir <igorkuo@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "test_languagecontroller.h"

#include <ilanguagecontroller.h>
#include <language/interfaces/ilanguagesupport.h>
#include <tests/testhelpers.h>

#include <QString>
#include <QTest>
#include <QUrl>

#include <future>

void TestLanguageController::testLanguagesForUrlInTheMainThread()
{
    QFETCH(QUrl, url);
    QFETCH(QString, languageName);

    const auto languages = m_subject->languagesForUrl(url);
    QCOMPARE(languages.size(), 1);
    QCOMPARE(languages.back()->name(), languageName);
}

void TestLanguageController::testLanguagesForUrlInTheMainThread_data()
{
    matchingLanguagesForUrlTestData();
}

void TestLanguageController::testLanguagesForUrlWithCache()
{
    QFETCH(QUrl, url);
    QFETCH(QString, languageName);

    // Add the MIME type to LanguageController's cache for use in the background thread.
    const auto languages = m_subject->languagesForUrl(url);
    QCOMPARE(languages.size(), 1);
    QCOMPARE(languages.back()->name(), languageName);

    auto future = std::async(std::launch::async, [&] {
        const auto languages = m_subject->languagesForUrl(url);
        QCOMPARE_RETURN(languages.size(), 1, false);
        QCOMPARE_RETURN(languages.back()->name(), languageName, false);
        return true;
    });
    QVERIFY(future.get());
}

void TestLanguageController::testLanguagesForUrlWithCache_data()
{
    matchingLanguagesForUrlInBackgroundThreadTestData();
}

void TestLanguageController::testLanguagesForUrlNoCache()
{
    QFETCH(QUrl, url);

    auto future = std::async(std::launch::async, [&] {
        const auto languages = m_subject->languagesForUrl(url);
        // When languagesForUrl() is called from a non-main thread, it cannot
        // determine languages for a MIME type without a cache.
        QVERIFY_RETURN(languages.empty(), false);
        return true;
    });
    QVERIFY(future.get());
}

void TestLanguageController::testLanguagesForUrlNoCache_data()
{
    matchingLanguagesForUrlTestData();
}

void TestLanguageController::testLanguagesForUrlNoMatch()
{
    QFETCH(QUrl, url);

    const auto languages = m_subject->languagesForUrl(url);
    QVERIFY(languages.empty());
}

void TestLanguageController::testLanguagesForUrlNoMatch_data()
{
    nonmatchingLanguagesForUrlTestData();
}

QTEST_MAIN(TestLanguageController)
