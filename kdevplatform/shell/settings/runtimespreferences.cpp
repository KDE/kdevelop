/*
    SPDX-FileCopyrightText: 2017 Friedrich W. H. Kossebau <kossebau@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "runtimespreferences.h"

#include <QIcon>

namespace KDevelop
{

RuntimesPreferences::RuntimesPreferences(QWidget* parent)
    : ConfigPage(nullptr, nullptr, parent)
{
}

RuntimesPreferences::~RuntimesPreferences() = default;

QString RuntimesPreferences::name() const
{
    return i18n("Runtimes");
}

QIcon RuntimesPreferences::icon() const
{
    return QIcon::fromTheme(QStringLiteral("file-library-symbolic"));
}

QString RuntimesPreferences::fullName() const
{
    return i18n("Configure Runtimes");
}

void RuntimesPreferences::apply()
{
}

void RuntimesPreferences::defaults()
{
}

void RuntimesPreferences::reset()
{
}

}
