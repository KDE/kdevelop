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

#include <QVBoxLayout>
#include <QIcon>

#include <KPluginFactory>
#include <KAboutData>

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

#include "definesandincludesconfigpage.h"

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

DefinesAndIncludesConfigPage::DefinesAndIncludesConfigPage(KDevelop::IPlugin* plugin, const KDevelop::ProjectConfigOptions& options, QWidget* parent)
    : ProjectConfigPage<CustomDefinesAndIncludes>(plugin, options, parent)
{
    QVBoxLayout* layout = new QVBoxLayout( this );
    configWidget = new ProjectPathsWidget( this );
    configWidget->setProject( project() );
    connect(configWidget, &ProjectPathsWidget::changed, this, &DefinesAndIncludesConfigPage::changed);
    layout->addWidget( configWidget );
}

DefinesAndIncludesConfigPage::~DefinesAndIncludesConfigPage()
{
}

void DefinesAndIncludesConfigPage::loadFrom( KConfig* cfg )
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

void DefinesAndIncludesConfigPage::saveTo(KConfig* cfg, KDevelop::IProject*)
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

void DefinesAndIncludesConfigPage::reset()
{
   ProjectConfigPage::reset();
   loadFrom(CustomDefinesAndIncludes::self()->config());
}

void DefinesAndIncludesConfigPage::apply()
{
    ProjectConfigPage::apply();
    saveTo(CustomDefinesAndIncludes::self()->config(), project());
}

QString DefinesAndIncludesConfigPage::name() const
{
    return i18n("Custom Defines And Includes");
}

QString DefinesAndIncludesConfigPage::fullName() const
{
    return i18n("Configure which macros and include directories/files will be added to the parser during project parsing.");
}

QIcon DefinesAndIncludesConfigPage::icon() const
{
    return QIcon::fromTheme("kdevelop");
}
