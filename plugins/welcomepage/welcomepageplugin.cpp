/*
    SPDX-FileCopyrightText: 2010 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "welcomepageplugin.h"

// plugin
#include "welcomepageview.h"
// KDevPlatform
#include <interfaces/icore.h>
#include <interfaces/iuicontroller.h>
#include <sublime/mainwindow.h>
// KF
#include <KPluginFactory>

K_PLUGIN_FACTORY_WITH_JSON(KDevWelcomePagePluginFactory, "kdevwelcomepage.json",
                           registerPlugin<KDevWelcomePagePlugin>();)

using namespace KDevelop;

KDevWelcomePagePlugin::KDevWelcomePagePlugin(QObject* parent, const KPluginMetaData& metaData, const QVariantList&)
    : IPlugin(QStringLiteral("kdevwelcomepage"), parent, metaData)
{
    m_welcomePageWidget = new WelcomePageWidget();

    auto mainWindow = qobject_cast<Sublime::MainWindow*>(ICore::self()->uiController()->activeMainWindow());
    mainWindow->setBackgroundCentralWidget(m_welcomePageWidget);
}

void KDevWelcomePagePlugin::unload()
{
    auto mainWindow = qobject_cast<Sublime::MainWindow*>(ICore::self()->uiController()->activeMainWindow());
    // Sublime::MainWindow::setBackgroundCentralWidget(widget) took ownership of the widget.
    // Resetting to blank page by passing a nullptr will also trigger its deletion.
    if (mainWindow && m_welcomePageWidget) {
        mainWindow->setBackgroundCentralWidget(nullptr);
    }
}

#include "welcomepageplugin.moc"
#include "moc_welcomepageplugin.cpp"
