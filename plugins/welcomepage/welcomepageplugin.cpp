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

K_PLUGIN_FACTORY_WITH_JSON(KDevWelcomePagePluginFactory, "kdevwelcomepage.json", registerPlugin<KDevWelcomePagePlugin>();)

using namespace KDevelop;

KDevWelcomePagePlugin::KDevWelcomePagePlugin( QObject* parent, const QVariantList& )
    : IPlugin(QStringLiteral("kdevwelcomepage"), parent )
{
    Sublime::MainWindow* mw = qobject_cast<Sublime::MainWindow*>(ICore::self()->uiController()->activeMainWindow());
    mw->setBackgroundCentralWidget(new WelcomePageWidget(QList<IProject*>(), mw));
}

#include "welcomepageplugin.moc"
