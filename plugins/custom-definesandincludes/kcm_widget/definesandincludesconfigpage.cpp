/*
    SPDX-FileCopyrightText: 2010 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-or-later
*/

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
        KDevelop::ICore::self()->projectController()->reparseProject(project());
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
    return i18nc("@title:tab", "Language Support");
}

QString DefinesAndIncludesConfigPage::fullName() const
{
    return i18nc("@title:tab", "Configure Language Support");
}

QIcon DefinesAndIncludesConfigPage::icon() const
{
    return QIcon::fromTheme(QStringLiteral("kdevelop"));
}

#include "moc_definesandincludesconfigpage.cpp"
