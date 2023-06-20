/*
    SPDX-FileCopyrightText: 2016 Anton Anikin <anton.anikin@htower.ru>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "documentationpreferences.h"

#include <QIcon>

namespace KDevelop
{

DocumentationPreferences::DocumentationPreferences(QWidget* parent)
    : ConfigPage(nullptr, nullptr, parent)
{
}

DocumentationPreferences::~DocumentationPreferences()
{
}

QString DocumentationPreferences::name() const
{
    return i18n("Documentation");
}

QIcon DocumentationPreferences::icon() const
{
    return QIcon::fromTheme(QStringLiteral("help-contents"));
}

QString DocumentationPreferences::fullName() const
{
    return i18n("Configure Documentation");
}

void DocumentationPreferences::apply()
{
}

void DocumentationPreferences::defaults()
{
}

void DocumentationPreferences::reset()
{
}

}

#include "moc_documentationpreferences.cpp"
