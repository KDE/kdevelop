/* This file is part of KDevelop
   Copyright 2017 Anton Anikin <anton.anikin@htower.ru>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "globalconfigpage.h"
#include "ui_globalconfigpage.h"

#include "globalsettings.h"

namespace Heaptrack
{

GlobalConfigPage::GlobalConfigPage(KDevelop::IPlugin* plugin, QWidget* parent)
    : ConfigPage(plugin, GlobalSettings::self(), parent)
{
    Ui::GlobalConfigPage ui;
    ui.setupUi(this);
}

KDevelop::ConfigPage::ConfigPageType GlobalConfigPage::configPageType() const
{
    return KDevelop::ConfigPage::AnalyzerConfigPage;
}

QString GlobalConfigPage::name() const
{
    return i18nc("@title:tab", "Heaptrack");
}

QString GlobalConfigPage::fullName() const
{
    return i18nc("@title:tab", "Configure Heaptrack Settings");
}

QIcon GlobalConfigPage::icon() const
{
    return QIcon::fromTheme(QStringLiteral("office-chart-area"));
}

}
