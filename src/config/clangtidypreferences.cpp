/*************************************************************************************
 *  Copyright (C) 2016 by Carlos Nihelton <carlosnsoliveira@gmail.com>               *
 *                                                                                   *
 *  This program is free software; you can redistribute it and/or                    *
 *  modify it under the terms of the GNU General Public License                      *
 *  as published by the Free Software Foundation; either version 2                   *
 *  of the License, or (at your option) any later version.                           *
 *                                                                                   *
 *  This program is distributed in the hope that it will be useful,                  *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of                   *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                    *
 *  GNU General Public License for more details.                                     *
 *                                                                                   *
 *  You should have received a copy of the GNU General Public License                *
 *  along with this program; if not, write to the Free Software                      *
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA   *
 *************************************************************************************/

#include "clangtidypreferences.h"

#include <QVBoxLayout>

#include "clangtidyconfig.h"

#include "configgroup.h"
#include "ui_clangtidysettings.h"

using KDevelop::IPlugin;
using ClangTidy::ConfigGroup;
using KDevelop::ConfigPage;

ClangtidyPreferences::ClangtidyPreferences(IPlugin* plugin, QWidget* parent)
    : ConfigPage(plugin, ClangtidySettings::self(), parent)
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    QWidget* widget = new QWidget(this);
    ui = new Ui::ClangtidySettings();
    ui->setupUi(widget);
    layout->addWidget(widget);
}

ClangtidyPreferences::~ClangtidyPreferences()
{
    delete ui;
}

ConfigPage::ConfigPageType ClangtidyPreferences::configPageType() const
{
    return ConfigPage::AnalyzerConfigPage;
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
    return QIcon::fromTheme(QStringLiteral("dialog-ok"));
}

void ClangtidyPreferences::apply()
{
    ConfigGroup projConf = KSharedConfig::openConfig()->group("Clangtidy");
    projConf.writeEntry(ConfigGroup::ExecutablePath, ui->kcfg_clangtidyPath->text());
}
