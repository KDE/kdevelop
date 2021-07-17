/*
    SPDX-FileCopyrightText: 2021 Igor Kushnir <igorkuo@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "bench_languagecontroller.h"

#include <ilanguagecontroller.h>
#include <language/interfaces/ilanguagesupport.h>

#include <QString>
#include <QTest>
#include <QUrl>

void BenchLanguageController::benchLanguagesForUrlNoCache()
{
    benchmarkLanguagesForUrl();
}

void BenchLanguageController::benchLanguagesForUrlNoCache_data()
{
    matchingLanguagesForUrlTestData();
}

void BenchLanguageController::benchLanguagesForUrlFilledCache()
{
    fillLanguageControllerMimeTypeCache();
    benchmarkLanguagesForUrl();
}

void BenchLanguageController::benchLanguagesForUrlFilledCache_data()
{
    matchingLanguagesForUrlTestData();
}

void BenchLanguageController::benchLanguagesForUrlNoMatchNoCache()
{
    benchmarkLanguagesForUrlNoMatch();
}

void BenchLanguageController::benchLanguagesForUrlNoMatchNoCache_data()
{
    nonmatchingLanguagesForUrlTestData();
}

void BenchLanguageController::benchLanguagesForUrlNoMatchFilledCache()
{
    fillLanguageControllerMimeTypeCache();
    benchmarkLanguagesForUrlNoMatch();
}

void BenchLanguageController::benchLanguagesForUrlNoMatchFilledCache_data()
{
    nonmatchingLanguagesForUrlTestData();
}

void BenchLanguageController::benchmarkLanguagesForUrl()
{
    QFETCH(QUrl, url);
    QFETCH(QString, languageName);

    // Warm up and check correctness.
    const auto languages = m_subject->languagesForUrl(url);
    QCOMPARE(languages.size(), 1);
    QCOMPARE(languages.back()->name(), languageName);

    QBENCHMARK {
        m_subject->languagesForUrl(url);
    }
}

void BenchLanguageController::benchmarkLanguagesForUrlNoMatch()
{
    QFETCH(QUrl, url);

    // Warm up and check correctness.
    const auto languages = m_subject->languagesForUrl(url);
    QCOMPARE(languages.size(), 0);

    QBENCHMARK {
        m_subject->languagesForUrl(url);
    }
}

QTEST_MAIN(BenchLanguageController)
