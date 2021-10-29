/*
    SPDX-FileCopyrightText: 2012 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_TESTLANGUAGECONTROLLER_H
#define KDEVPLATFORM_TESTLANGUAGECONTROLLER_H

#include "testsexport.h"

#include <shell/languagecontroller.h>

namespace KDevelop {
class ILanguageSupport;

class KDEVPLATFORMTESTS_EXPORT TestLanguageController
    : public LanguageController
{
    Q_OBJECT

public:
    explicit TestLanguageController(QObject* parent);
    ~TestLanguageController() override;

    void addTestLanguage(KDevelop::ILanguageSupport* languageSupport, const QStringList& mimetypes);
};
}

#endif // KDEVPLATFORM_TESTLANGUAGECONTROLLER_H
