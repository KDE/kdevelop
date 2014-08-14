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

#include <QAction>

#include <KPluginFactory>
#include <KPluginLoader>
#include <KAboutData>

#include <interfaces/icore.h>
#include <interfaces/idocumentcontroller.h>
#include <interfaces/context.h>
#include <interfaces/contextmenuextension.h>
#include <interfaces/iprojectcontroller.h>
#include <interfaces/iproject.h>
#include <project/projectmodel.h>
#include <QTimer>
#include "welcomepagedocument.h"

K_PLUGIN_FACTORY(KDevWelcomePagePluginFactory, registerPlugin<KDevWelcomePagePlugin>(); )
// K_EXPORT_PLUGIN(KDevWelcomePagePluginFactory(KAboutData("kdevprojectdashboard","kdevprojectdashboard", ki18n("Project Dashboard"),
//                                                        "0.1", ki18n("This plugin shows relevant information of some project when it's invoked"), KAboutData::License_LGPL)))

using namespace KDevelop;

class WelcomePageFactory : public KDevelop::IDocumentFactory
{
public:
    virtual IDocument* create(const KUrl& url, ICore*)
    {
        return new WelcomePageDocument(QList<IProject*>());
    }
};

KDevWelcomePagePlugin::KDevWelcomePagePlugin( QObject* parent, const QVariantList& )
    : IPlugin("welcomepageplugin", parent )
{
    QByteArray disable = qgetenv("KDEV_DISABLE_WELCOMEPAGE");
    if (!disable.isEmpty())
        return;
    ICore::self()->documentController()->registerDocumentForMimetype("text/x-kdevelop-internal", new WelcomePageFactory);

    QTimer::singleShot(500, this, SLOT(openWelcomePage()));
}

void KDevWelcomePagePlugin::openWelcomePage()
{
    ICore::self()->documentController()->openDocument(WelcomePageDocument::welcomePageUrl());
}

#include "welcomepageplugin.moc"
