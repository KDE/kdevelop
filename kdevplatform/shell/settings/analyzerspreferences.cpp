/*
    SPDX-FileCopyrightText: 2016 Anton Anikin <anton.anikin@htower.ru>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "analyzerspreferences.h"

#include <QIcon>

namespace KDevelop
{

AnalyzersPreferences::AnalyzersPreferences(QWidget* parent)
    : ConfigPage(nullptr, nullptr, parent)
{
}

AnalyzersPreferences::~AnalyzersPreferences()
{
}

QString AnalyzersPreferences::name() const
{
    return i18n("Analyzers");
}

QIcon AnalyzersPreferences::icon() const
{
    return QIcon::fromTheme(QStringLiteral("dialog-ok"));
}

QString AnalyzersPreferences::fullName() const
{
    return i18n("Configure Analyzers");
}

void AnalyzersPreferences::apply()
{
}

void AnalyzersPreferences::defaults()
{
}

void AnalyzersPreferences::reset()
{
}

}
