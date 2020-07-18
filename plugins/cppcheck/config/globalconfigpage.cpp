/*
 * Copyright 2015 Laszlo Kis-Adam <laszlo.kis-adam@kdemail.net>
 * Copyright 2016 Anton Anikin <anton.anikin@htower.ru>
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

#include "globalconfigpage.h"
#include "ui_globalconfigpage.h"

#include "globalsettings.h"

namespace cppcheck
{

GlobalConfigPage::GlobalConfigPage(KDevelop::IPlugin* plugin, QWidget* parent)
    : ConfigPage(plugin, GlobalSettings::self(), parent)
{
    Ui::GlobalConfigPage ui;
    ui.setupUi(this);
}

GlobalConfigPage::~GlobalConfigPage()
{
}

KDevelop::ConfigPage::ConfigPageType GlobalConfigPage::configPageType() const
{
    return KDevelop::ConfigPage::AnalyzerConfigPage;
}

QString GlobalConfigPage::name() const
{
    return i18nc("@title:tab", "Cppcheck");
}

QString GlobalConfigPage::fullName() const
{
    return i18nc("@title:tab", "Configure Cppcheck Settings");
}

QIcon GlobalConfigPage::icon() const
{
    return QIcon::fromTheme(QStringLiteral("cppcheck"));
}

}
