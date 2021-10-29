/*
    SPDX-FileCopyrightText: 2012 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "testlanguagecontroller.h"

#include <language/interfaces/ilanguagesupport.h>

using namespace KDevelop;

TestLanguageController::TestLanguageController(QObject* parent)
    : LanguageController(parent)
{
}

TestLanguageController::~TestLanguageController()
{
}

void TestLanguageController::addTestLanguage(ILanguageSupport* languageSupport, const QStringList& mimetypes)
{
    addLanguageSupport(languageSupport, mimetypes);
}
