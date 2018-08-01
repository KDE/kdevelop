/*
 * This file is part of KDevelop
 *
 * Copyright 2016 Carlos Nihelton <carlosnsoliveira@gmail.com>
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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#include "config/clangtidypreferences.h"
#include "clangtidyconfig.h"
#include "ui_clangtidypreferences.h"

using KDevelop::IPlugin;
using KDevelop::ConfigPage;

ClangTidyPreferences::ClangTidyPreferences(IPlugin* plugin, QWidget* parent)
    : ConfigPage(plugin, ClangTidySettings::self(), parent)
{
    ui = new Ui::ClangTidyPreferences();
    ui->setupUi(this);
}

ClangTidyPreferences::~ClangTidyPreferences()
{
    delete ui;
}

ConfigPage::ConfigPageType ClangTidyPreferences::configPageType() const
{
    return ConfigPage::AnalyzerConfigPage;
}

QString ClangTidyPreferences::name() const
{
    return i18n("Clang-Tidy");
}

QString ClangTidyPreferences::fullName() const
{
    return i18n("Configure Clang-Tidy Settings");
}

QIcon ClangTidyPreferences::icon() const
{
    return QIcon::fromTheme(QStringLiteral("dialog-ok"));
}
