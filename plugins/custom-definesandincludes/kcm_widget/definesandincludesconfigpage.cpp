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

#include "definesandincludesconfigpage.h"

#include <QVBoxLayout>
#include <QIcon>

#include "projectpathswidget.h"
#include "customdefinesandincludes.h"
#include "../compilerprovider/compilerprovider.h"

#include <interfaces/iproject.h>
#include <interfaces/iprojectcontroller.h>
#include <interfaces/icore.h>
#include <interfaces/idocument.h>
#include <project/projectmodel.h>

DefinesAndIncludesConfigPage::DefinesAndIncludesConfigPage(KDevelop::IPlugin* plugin, const KDevelop::ProjectConfigOptions& options, QWidget* parent)
    : ProjectConfigPage<CustomDefinesAndIncludes>(plugin, options, parent)
{
    auto* layout = new QVBoxLayout( this );
    layout->setContentsMargins(0, 0, 0, 0);
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

    auto settings = SettingsManager::globalInstance();

    configWidget->setPaths( settings->readPaths( cfg ) );
}

void DefinesAndIncludesConfigPage::saveTo(KConfig* cfg, KDevelop::IProject*)
{
    auto settings = SettingsManager::globalInstance();
    settings->writePaths( cfg, configWidget->paths() );

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
    return i18n("Language Support");
}

QString DefinesAndIncludesConfigPage::fullName() const
{
    return i18n("Configure Language Support");
}

QIcon DefinesAndIncludesConfigPage::icon() const
{
    return QIcon::fromTheme(QStringLiteral("kdevelop"));
}
