/*
 * Copyright 2015 Laszlo Kis-Adam <laszlo.kis-adam@kdemail.net>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "clangtidypreferences.h"

#include <QVBoxLayout>

#include "clangtidyconfig.h"

#include "ui_clangtidysettings.h"

using namespace KDevelop;

ClangtidyPreferences::ClangtidyPreferences(IPlugin *plugin, QWidget* parent)
    : ConfigPage(plugin, ClangtidySettings::self(), parent)
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    QWidget* widget = new QWidget(this);
    Ui::ClangtidySettings ui;
    ui.setupUi(widget);
    layout->addWidget(widget);
}

ClangtidyPreferences::~ClangtidyPreferences()
{
}

QString ClangtidyPreferences::name() const
{
    return i18n("clang-tidy");
}

QString ClangtidyPreferences::fullName() const
{
   return i18n("Configure clang-tidy settings");
}

QIcon ClangtidyPreferences::icon() const
{
   return QIcon::fromTheme(QStringLiteral("kdevelop"));
}
