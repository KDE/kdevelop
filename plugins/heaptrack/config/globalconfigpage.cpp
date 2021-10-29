/*
    SPDX-FileCopyrightText: 2017 Anton Anikin <anton.anikin@htower.ru>

    SPDX-License-Identifier: GPL-2.0-or-later
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
