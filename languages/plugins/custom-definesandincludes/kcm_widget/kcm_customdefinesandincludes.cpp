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
#include "../compilerprovider/icompilerprovider.h"

#include <interfaces/iruncontroller.h>
#include <interfaces/iproject.h>
#include <interfaces/iprojectcontroller.h>
#include <interfaces/iplugincontroller.h>
#include <interfaces/icore.h>
#include <interfaces/idocumentcontroller.h>
#include <interfaces/idocument.h>
#include <serialization/indexedstring.h>

#include "kcm_customdefinesandincludes.h"

namespace
{
ICompilerProvider* compilerProvider()
{
    auto compilerProvider = KDevelop::ICore::self()->pluginController()->pluginForExtension("org.kdevelop.ICompilerProvider");
    if (!compilerProvider || !compilerProvider->extension<ICompilerProvider>()) {
        return {};
    }

    return compilerProvider->extension<ICompilerProvider>();
}
}

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

void DefinesAndIncludes::loadFrom( KConfig* cfg )
{
    configWidget->clear();
    auto iadm = KDevelop::IDefinesAndIncludesManager::manager();
    auto settings = static_cast<KDevelop::DefinesAndIncludesManager*>( iadm );
    configWidget->setPaths( settings->readPaths( cfg ) );

    if (auto cp = compilerProvider()) {
        configWidget->setCompilers(cp->compilers());
        configWidget->setCurrentCompiler(cp->currentCompiler(project())->name());
    }
}

void DefinesAndIncludes::saveTo(KConfig* cfg, KDevelop::IProject*)
{
    auto iadm = KDevelop::IDefinesAndIncludesManager::manager();
    auto settings = static_cast<KDevelop::DefinesAndIncludesManager*>( iadm );
    settings->writePaths( cfg, configWidget->paths() );

    if (auto cp = compilerProvider()) {
        settings->writeUserDefinedCompilers(configWidget->compilers());

        settings->writeCurrentCompiler(cfg, configWidget->currentCompiler());

        cp->setCompiler(project(), settings->currentCompiler(cfg));

        auto compilers = compilerProvider()->compilers();

        for (auto c: configWidget->compilers()) {
            if (!compilers.contains(c)) {
                compilerProvider()->registerCompiler(c);
            }
        }

        compilers = compilerProvider()->compilers();
        for (auto compiler: compilers) {
            if (!configWidget->compilers().contains(compiler)) {
                compilerProvider()->unregisterCompiler(compiler);
            }
        }
    }

    if ( settings->needToReparseCurrentProject( cfg ) ) {
        KDevelop::ICore::self()->projectController()->reparseProject(project(), true);
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
