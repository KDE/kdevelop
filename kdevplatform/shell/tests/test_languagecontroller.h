/*
    SPDX-FileCopyrightText: 2021 Igor Kushnir <igorkuo@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_TEST_LANGUAGECONTROLLER_H
#define KDEVPLATFORM_TEST_LANGUAGECONTROLLER_H

#include "languagecontrollertestbase.h"

class TestLanguageController : public LanguageControllerTestBase
{
    Q_OBJECT

private Q_SLOTS:
    void testLanguagesForUrlInTheMainThread();
    void testLanguagesForUrlInTheMainThread_data();
    void testLanguagesForUrlWithCache();
    void testLanguagesForUrlWithCache_data();
    void testLanguagesForUrlNoCache();
    void testLanguagesForUrlNoCache_data();
    void testLanguagesForUrlNoMatch();
    void testLanguagesForUrlNoMatch_data();
};

#endif // KDEVPLATFORM_TEST_LANGUAGECONTROLLER_H
