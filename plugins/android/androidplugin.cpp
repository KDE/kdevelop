/*
    SPDX-FileCopyrightText: 2017 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "androidplugin.h"
#include "androidruntime.h"
#include "androidpreferences.h"
#include "androidpreferencessettings.h"
#include <interfaces/icore.h>
#include <interfaces/iruntimecontroller.h>
#include <KPluginFactory>

K_PLUGIN_FACTORY_WITH_JSON(KDevAndroidFactory, "kdevandroid.json", registerPlugin<AndroidPlugin>();)

using namespace KDevelop;

AndroidPlugin::AndroidPlugin(QObject* parent, const KPluginMetaData& metaData, const QVariantList& /*args*/)
    : KDevelop::IPlugin(QStringLiteral("kdevandroid"), parent, metaData)
    , m_settings(new AndroidPreferencesSettings)
{
    setXMLFile( QStringLiteral("kdevandroidplugin.rc") );

    AndroidRuntime::s_settings = m_settings.data();

    ICore::self()->runtimeController()->addRuntimes(new AndroidRuntime);
}

AndroidPlugin::~AndroidPlugin()
{
    AndroidRuntime::s_settings = nullptr;
}

int AndroidPlugin::configPages() const
{
    return 1;
}

KDevelop::ConfigPage* AndroidPlugin::configPage(int number, QWidget* parent)
{
    if (number == 0) {
        return new AndroidPreferences(this, m_settings.data(), parent);
    }
    return nullptr;
}

#include "androidplugin.moc"
#include "moc_androidplugin.cpp"
