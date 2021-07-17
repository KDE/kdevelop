/*
    SPDX-FileCopyrightText: 2021 Igor Kushnir <igorkuo@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_BENCH_LANGUAGECONTROLLER_H
#define KDEVPLATFORM_BENCH_LANGUAGECONTROLLER_H

#include "languagecontrollertestbase.h"

class BenchLanguageController : public LanguageControllerTestBase
{
    Q_OBJECT

private Q_SLOTS:
    void benchLanguagesForUrlNoCache();
    void benchLanguagesForUrlNoCache_data();
    void benchLanguagesForUrlFilledCache();
    void benchLanguagesForUrlFilledCache_data();
    void benchLanguagesForUrlNoMatchNoCache();
    void benchLanguagesForUrlNoMatchNoCache_data();
    void benchLanguagesForUrlNoMatchFilledCache();
    void benchLanguagesForUrlNoMatchFilledCache_data();

private:
    void benchmarkLanguagesForUrl();
    void benchmarkLanguagesForUrlNoMatch();
};

#endif // KDEVPLATFORM_BENCH_LANGUAGECONTROLLER_H
