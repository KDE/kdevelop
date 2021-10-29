/*
    SPDX-FileCopyrightText: 2015 Sergey Kalinichev <kalinichev.so.0@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "sessionsettings.h"

#include "sessionconfig.h"
#include "ui_sessionsettings.h"

SessionSettings::SessionSettings(QWidget* parent)
    : ConfigPage(nullptr, SessionConfig::self(), parent)
    , m_settings(new Ui::SessionSettings)
{
    m_settings->setupUi(this);
}

void SessionSettings::reset()
{
     ConfigPage::reset();

    Q_ASSERT(ICore::self()->activeSession());
}

SessionSettings::~SessionSettings( )
{}

void SessionSettings::apply()
{
     ConfigPage::apply();
}

QString SessionSettings::name() const
{
    return i18nc("@title:tab", "C/C++ Helpers");
}

QString SessionSettings::fullName() const
{
    return i18nc("@title:tab", "Configure C/C++ Helpers");
}

QIcon SessionSettings::icon() const
{
    return QIcon::fromTheme(QStringLiteral("kdevelop"));
}

KDevelop::ConfigPage::ConfigPageType SessionSettings::configPageType() const
{
    return ConfigPage::LanguageConfigPage;
}
