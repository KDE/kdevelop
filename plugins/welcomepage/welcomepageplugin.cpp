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
#include <KActionCollection>

#include <interfaces/icore.h>
#include <interfaces/idocumentcontroller.h>
#include <interfaces/context.h>
#include <interfaces/contextmenuextension.h>
#include <interfaces/iprojectcontroller.h>
#include <interfaces/iproject.h>
#include <project/projectmodel.h>
#include <QTimer>
#include "welcomepagedocument.h"

K_PLUGIN_FACTORY_WITH_JSON(KDevWelcomePagePluginFactory, "kdevwelcomepage.json", registerPlugin<KDevWelcomePagePlugin>();)

using namespace KDevelop;

class WelcomePageFactory : public KDevelop::IDocumentFactory
{
public:
    virtual IDocument* create(const QUrl& url, ICore*)
    {
        return new WelcomePageDocument();
    }
};

KDevWelcomePagePlugin::KDevWelcomePagePlugin( QObject* parent, const QVariantList& )
    : IPlugin("kdevwelcomepage", parent )
{
    ICore::self()->documentController()->registerDocumentForMimetype("text/x-kdevelop-internal", new WelcomePageFactory);

    //FIXME: When and where to open the welcome page?
    //QTimer::singleShot(500, this, SLOT(openWelcomePage()));

    QAction* action = actionCollection()->addAction("show-welcomepage");
    action->setText("Show Welcome Page");
    action->setIcon(QIcon::fromTheme("meeting-organizer"));
}

void KDevWelcomePagePlugin::openWelcomePage()
{
    ICore::self()->documentController()->openDocument(WelcomePageDocument::welcomePageUrl());
}

#include "welcomepageplugin.moc"
