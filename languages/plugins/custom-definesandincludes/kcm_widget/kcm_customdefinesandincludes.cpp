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
#include "definesandincludesmanager.h"
#include "../compilerprovider/icompilerprovider.h"

#include <interfaces/iruncontroller.h>
#include <interfaces/iproject.h>
#include <interfaces/iprojectcontroller.h>
#include <interfaces/iplugincontroller.h>
#include <interfaces/icore.h>
#include <interfaces/idocumentcontroller.h>
#include <interfaces/idocument.h>
#include <language/duchain/indexedstring.h>

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

QStringList compilerNames(QVector<CompilerPointer> compilers)
{
    QStringList names;
    for (const auto& compiler : compilers) {
        names << compiler->name();
    }
    return names;
}
}

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
    if (auto cp = compilerProvider()) {
        auto name = configWidget->currentCompilerName();
        for (auto& c : cp->compilers()) {
            if (c->name() == name) {
                configWidget->setCompilerPath(c->defaultPath());
            }
        }
    }

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
        configWidget->setCompilers(compilerNames(cp->compilers()));
        configWidget->setCurrentCompiler(cp->currentCompiler(project())->name());
        configWidget->setCompilerPath(!settings->pathToCompiler(cfg).isEmpty() ? settings->pathToCompiler(cfg) : cp->currentCompiler(project())->defaultPath());
    }
}

void DefinesAndIncludes::saveTo(KConfig* cfg, KDevelop::IProject*)
{
    auto iadm = KDevelop::IDefinesAndIncludesManager::manager();
    auto settings = static_cast<KDevelop::DefinesAndIncludesManager*>( iadm );
    settings->writePaths( cfg, configWidget->paths() );

    if (auto cp = compilerProvider()) {
        settings->writeCompiler(cfg ,configWidget->currentCompilerName());

        cp->setCompiler(project(), settings->currentCompiler(cfg), settings->pathToCompiler(cfg));
    }

    if ( settings->needToReparseCurrentProject( cfg ) ) {
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
