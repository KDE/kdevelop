/*
    SPDX-FileCopyrightText: 2017 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "androidpreferences.h"
#include "ui_androidpreferences.h"

AndroidPreferences::AndroidPreferences(KDevelop::IPlugin* plugin, KCoreConfigSkeleton* config, QWidget* parent)
    : KDevelop::ConfigPage(plugin, config, parent)
{
    auto m_prefsUi = new Ui::AndroidPreferences;
    m_prefsUi->setupUi(this);
}

AndroidPreferences::~AndroidPreferences() = default;

KDevelop::ConfigPage::ConfigPageType AndroidPreferences::configPageType() const
{
    return KDevelop::ConfigPage::RuntimeConfigPage;
}

QString AndroidPreferences::name() const
{
    return QStringLiteral("Android");
}

#include "moc_androidpreferences.cpp"
