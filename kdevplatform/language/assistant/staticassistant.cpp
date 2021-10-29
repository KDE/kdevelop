/*
    SPDX-FileCopyrightText: 2014 Kevin Funk <kfunk@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "staticassistant.h"

#include <KTextEditor/Range>

using namespace KDevelop;

class KDevelop::StaticAssistantPrivate
{
public:
    ILanguageSupport* m_supportedLanguage;
};

StaticAssistant::StaticAssistant(ILanguageSupport* supportedLanguage)
    : d_ptr(new StaticAssistantPrivate)
{
    Q_D(StaticAssistant);

    d->m_supportedLanguage = supportedLanguage;
}

StaticAssistant::~StaticAssistant()
{
}

ILanguageSupport* StaticAssistant::supportedLanguage() const
{
    Q_D(const StaticAssistant);

    return d->m_supportedLanguage;
}
