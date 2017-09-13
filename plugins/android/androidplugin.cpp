/*
   Copyright 2017 Aleix Pol Gonzalez <aleixpol@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
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

AndroidPlugin::AndroidPlugin(QObject *parent, const QVariantList & /*args*/)
    : KDevelop::IPlugin( QStringLiteral("kdevandroid"), parent )
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
