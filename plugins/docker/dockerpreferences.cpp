/*
    SPDX-FileCopyrightText: 2017 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "dockerpreferences.h"
#include "ui_dockerpreferences.h"

DockerPreferences::DockerPreferences(KDevelop::IPlugin* plugin, KCoreConfigSkeleton* config, QWidget* parent)
    : KDevelop::ConfigPage(plugin, config, parent)
{
    auto m_prefsUi = new Ui::DockerPreferences;
    m_prefsUi->setupUi(this);
}

DockerPreferences::~DockerPreferences() = default;

KDevelop::ConfigPage::ConfigPageType DockerPreferences::configPageType() const
{
    return KDevelop::ConfigPage::RuntimeConfigPage;
}

QString DockerPreferences::name() const
{
    return QStringLiteral("Docker");
}

#include "moc_dockerpreferences.cpp"
