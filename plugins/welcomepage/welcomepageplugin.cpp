/* This file is part of KDevelop
  Copyright 2010 Aleix Pol Gonzalez <aleixpol@kde.org>

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to
  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
  Boston, MA 02110-1301, USA.
*/

#include "welcomepageplugin.h"
#include "welcomepageview.h"

#include <KPluginFactory>

#include <interfaces/icore.h>
#include <interfaces/iuicontroller.h>
#include <sublime/mainwindow.h>

#include <QDebug>

K_PLUGIN_FACTORY_WITH_JSON(KDevWelcomePagePluginFactory, "kdevwelcomepage.json", registerPlugin<KDevWelcomePagePlugin>();)

using namespace KDevelop;

namespace {
WelcomePageWidget* createWelcomePageWidget()
{
    // don't attempt to load any QML if CPU doesn't have SSE2 support (cf. bug 381999)
    // Note: Clang 4.0 moved the definition of __cpu_model (used by __builtin_cpu_supporst) into compiler-rt lib it seems -- let's just disable this feature on this compiler
#if defined(Q_OS_LINUX) && defined(Q_PROCESSOR_X86_32) && (defined(Q_CC_GNU) || (defined(Q_CC_CLANG) && __clang_major__ >= 3 && __clang_minor__ >= 7 && __clang_major__ < 4))
    if (!__builtin_cpu_supports("sse2")) {
        qWarning() << "Welcome Page won't load any QML -- lacking SSE2 support on this processor";
        return nullptr;
    }
#endif

    return new WelcomePageWidget({});
}
}

KDevWelcomePagePlugin::KDevWelcomePagePlugin( QObject* parent, const QVariantList& )
    : IPlugin(QStringLiteral("kdevwelcomepage"), parent )
{
    m_welcomePageWidget = createWelcomePageWidget();
    if (m_welcomePageWidget) {
        auto mainWindow = qobject_cast<Sublime::MainWindow*>(ICore::self()->uiController()->activeMainWindow());
        mainWindow->setBackgroundCentralWidget(m_welcomePageWidget);
    }
}

void KDevWelcomePagePlugin::unload()
{
    auto mainWindow = qobject_cast<Sublime::MainWindow*>(ICore::self()->uiController()->activeMainWindow());
    if (mainWindow && m_welcomePageWidget) {
        mainWindow->setBackgroundCentralWidget(nullptr);
    }
}

#include "welcomepageplugin.moc"
