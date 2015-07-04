/*
 * This file is part of KDevelop
 *
 * Copyright 2015 Sergey Kalinichev <kalinichev.so.0@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "sessionsettings.h"

#include <QVBoxLayout>

#include "sessionconfig.h"
#include "ui_sessionsettings.h"

SessionSettings::SessionSettings(QWidget* parent)
    : ConfigPage(nullptr, SessionConfig::self(), parent)
    , m_settings(new Ui::SessionSettings)
{
    auto l = new QVBoxLayout(this);
    auto w = new QWidget(this);

    m_settings->setupUi(w);

    l->addWidget(w);
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
    return i18n("Clang Language Support");
}

QString SessionSettings::fullName() const
{
    return i18n("Clang Language Support");
}

QIcon SessionSettings::icon() const
{
    return QIcon::fromTheme(QStringLiteral("kdevelop"));
}

KDevelop::ConfigPage::ConfigPageType SessionSettings::configPageType() const
{
    return ConfigPage::LanguageConfigPage;
}
