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

#include "cppcheckpreferences.h"

#include <QVBoxLayout>

#include "cppcheckconfig.h"

#include "ui_cppchecksettings.h"

using namespace KDevelop;

CppCheckPreferences::CppCheckPreferences(IPlugin *plugin, QWidget* parent)
    : ConfigPage(plugin, CppCheckSettings::self(), parent)
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    QWidget* widget = new QWidget(this);
    Ui::CppCheckSettings ui;
    ui.setupUi(widget);
    layout->addWidget(widget);
}

CppCheckPreferences::~CppCheckPreferences()
{
}

QString CppCheckPreferences::name() const
{
    return i18n("Cppcheck");
}

QString CppCheckPreferences::fullName() const
{
   return i18n("Configure Cppcheck settings");
}

QIcon CppCheckPreferences::icon() const
{
   return QIcon::fromTheme(QStringLiteral("kdevelop"));
}
