/************************************************************************
 *                                                                      *
 * Copyright 2010 Andreas Pakulat <apaku@gmx.de>                        *
 *                                                                      *
 * This program is free software; you can redistribute it and/or modify *
 * it under the terms of the GNU General Public License as published by *
 * the Free Software Foundation; either version 2 or version 3 of the License, or    *
 * (at your option) any later version.                                  *
 *                                                                      *
 * This program is distributed in the hope that it will be useful, but  *
 * WITHOUT ANY WARRANTY; without even the implied warranty of           *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU     *
 * General Public License for more details.                             *
 *                                                                      *
 * You should have received a copy of the GNU General Public License    *
 * along with this program; if not, see <http://www.gnu.org/licenses/>. *
 ************************************************************************/

#include <KPluginFactory>
#include <KAboutData>
#include <QVBoxLayout>

#include "projectpathswidget.h"
#include "customdefinesandincludes.h"

#include "definesandincludesmanager.h"

#include <interfaces/iruncontroller.h>
#include <interfaces/iproject.h>
#include <interfaces/iprojectcontroller.h>
#include <interfaces/icore.h>
#include <interfaces/idocumentcontroller.h>
#include <interfaces/idocument.h>
#include <language/duchain/indexedstring.h>

#include "kcm_customdefinesandincludes.h"

K_PLUGIN_FACTORY(DefinesAndIncludesFactory, registerPlugin<DefinesAndIncludes>(); )
//K_EXPORT_PLUGIN(DefinesAndIncludesFactory("kcm_kdevcustomdefinesandincludes", "kdevcustomdefinesandincludes"))

DefinesAndIncludes::DefinesAndIncludes( QWidget* parent, const QVariantList& args )
    : ProjectKCModule<CustomDefinesAndIncludes>(KAboutData::pluginData("kcm_kdevcustomdefinesandincludes"), parent, args)
{
    QVBoxLayout* layout = new QVBoxLayout( this );
    configWidget = new ProjectPathsWidget( this );
    configWidget->setProject( project() );
    connect( configWidget, SIGNAL(changed()), SLOT(dataChanged()) );
    layout->addWidget( configWidget );

    addConfig( CustomDefinesAndIncludes::self(), configWidget );
}

void DefinesAndIncludes::dataChanged()
{
    emit changed(true);
}

DefinesAndIncludes::~DefinesAndIncludes()
{
}

void DefinesAndIncludes::loadFrom(KConfig* cfg)
{
    configWidget->clear();
    auto IManager = KDevelop::IDefinesAndIncludesManager::manager();
    if ( IManager ) {
        auto manager = static_cast<KDevelop::DefinesAndIncludesManager*>(IManager);
        configWidget->setPaths( manager->readSettings( cfg ) );
    }
}

void DefinesAndIncludes::saveTo(KConfig* cfg, KDevelop::IProject*)
{
    auto IManager = KDevelop::IDefinesAndIncludesManager::manager();
    if ( IManager ) {
        auto manager = static_cast<KDevelop::DefinesAndIncludesManager*>(IManager);
        manager->writeSettings( cfg, configWidget->paths() );
    }

    auto grp = cfg->group("Custom Defines And Includes");
    if (grp.readEntry("reparse", true)) {
        using namespace KDevelop;
        ICore::self()->projectController()->reparseProject(project(), true);

        //TODO: BackgroundParser should check whether a document is currently opened and prioritize it then. The _focused_ one should be prioritized even further.
        for (auto document : ICore::self()->documentController()->openDocuments()) {
            if (!project()->filesForPath(IndexedString(document->url())).isEmpty()) {
                document->reload();
            }
        }
    }
}

void DefinesAndIncludes::load()
{
   KCModule::load();
   loadFrom( CustomDefinesAndIncludes::self()->config() );
}

void DefinesAndIncludes::save()
{
    KCModule::save();
    saveTo( CustomDefinesAndIncludes::self()->config(), project() );
}

#include "kcm_customdefinesandincludes.moc"
