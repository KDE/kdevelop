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
#include <QVBoxLayout>

#include "projectpathswidget.h"
#include "customdefinesandincludes.h"

#include <language/interfaces/idefinesandincludesmanager.h>

#include <interfaces/iruncontroller.h>

#include "kcm_customdefinesandincludes.h"

K_PLUGIN_FACTORY(DefinesAndIncludesFactory, registerPlugin<DefinesAndIncludes>(); )
K_EXPORT_PLUGIN(DefinesAndIncludesFactory("kcm_kdevcustomdefinesandincludes", "kdevcustomdefinesandincludes"))

DefinesAndIncludes::DefinesAndIncludes( QWidget* parent, const QVariantList& args )
    : ProjectKCModule<CustomDefinesAndIncludes>( DefinesAndIncludesFactory::componentData(), parent, args )
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
    auto manager = KDevelop::IDefinesAndIncludesManager::manager();
    if ( manager ) {
        configWidget->setPaths( manager->readSettings( cfg ) );
    }
}

void DefinesAndIncludes::saveTo(KConfig* cfg, KDevelop::IProject*)
{
    auto manager = KDevelop::IDefinesAndIncludesManager::manager();
    if ( manager ) {
        manager->writeSettings( cfg, configWidget->paths() );
    }
}

void DefinesAndIncludes::load()
{
   KCModule::load();
   loadFrom( CustomDefinesAndIncludes::self()->config() );
}

void DefinesAndIncludes::save()
{
    saveTo( CustomDefinesAndIncludes::self()->config(), project() );
    KCModule::save();
}

void DefinesAndIncludes::defaults()
{
    KCModule::defaults();
    //configWidget->loadDefaults();
}

#include "kcm_customdefinesandincludes.moc"
