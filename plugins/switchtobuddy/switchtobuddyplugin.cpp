/*
 * This file is part of KDevelop
 * Copyright 2012 André Stein <andre.stein@rwth-aachen.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "switchtobuddyplugin.h"

#include <KLocale>
#include <KPluginFactory>
#include <KPluginLoader>
#include <KAboutData>
#include <QAction>
#include <QFile>
#include <QSignalMapper>

#include <interfaces/icore.h>
#include <interfaces/idocumentcontroller.h>
#include <interfaces/ibuddydocumentfinder.h>
#include <language/interfaces/editorcontext.h>

using namespace KDevelop;

K_PLUGIN_FACTORY(SwitchToBuddyPluginFactory, registerPlugin<SwitchToBuddyPlugin>(); )
K_EXPORT_PLUGIN(SwitchToBuddyPluginFactory(KAboutData(
                                            "kdevswitchtobuddy"
                                           ,"kdevswitchtobuddy"
                                           , ki18n("Switch to Buddy")
                                           , "0.1"
                                           , ki18n("Allows switching between buddy documents like implementation and header file.")
                                           , KAboutData::License_GPL)
                                           .addAuthor(ki18n("André Stein")
                                           , ki18n("Author")
                                           , "andre.stein@rwth-aachen.de"
                                           , "http://steinsoft.net")
))

SwitchToBuddyPlugin::SwitchToBuddyPlugin ( QObject* parent, const QVariantList& )
    : IPlugin ( SwitchToBuddyPluginFactory::componentData(), parent )
    , m_signalMapper(0)
{
}

SwitchToBuddyPlugin::~SwitchToBuddyPlugin()
{
}

ContextMenuExtension SwitchToBuddyPlugin::contextMenuExtension(Context* context)
{
    EditorContext* ctx = dynamic_cast<EditorContext*>(context);
    if (!ctx) {
        return ContextMenuExtension();
    }

    KUrl currentUrl = ctx->url();
    IBuddyDocumentFinder* buddyFinder = IBuddyDocumentFinder::finderForMimeType(KMimeType::findByUrl(currentUrl)->name());
    if (!buddyFinder)
        return ContextMenuExtension();

    // Get all potential buddies for the current document and add a switch-to action
    // for each buddy who really exists in the file system. Note: if no buddies could be calculated
    // no extension actions are generated.
    const QVector<KUrl>& potentialBuddies = buddyFinder->getPotentialBuddies(currentUrl);

    ContextMenuExtension extension;
    if (m_signalMapper) {
        delete m_signalMapper;
    }
    m_signalMapper = new QSignalMapper(this);

    foreach(const KUrl& url, potentialBuddies) {
        if (!QFile::exists(url.toLocalFile())) {
            continue;
        }

        QAction* action = new QAction(i18n("Switch to '%1'", url.fileName()), this);
        connect(action, SIGNAL(triggered()), m_signalMapper, SLOT(map()), Qt::QueuedConnection);
        m_signalMapper->setMapping(action, url.toLocalFile());
        connect(m_signalMapper, SIGNAL(mapped(const QString&)),
                this, SLOT(switchToBuddy(const QString&)), Qt::QueuedConnection);
        extension.addAction(ContextMenuExtension::ExtensionGroup, action);
    }

    return extension;
}

void SwitchToBuddyPlugin::switchToBuddy(const QString& url)
{
    KDevelop::ICore::self()->documentController()->openDocument(url);
}

