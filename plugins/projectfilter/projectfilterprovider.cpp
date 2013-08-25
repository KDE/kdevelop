/*
    This file is part of KDevelop

    Copyright 2013 Milian Wolff <mail@milianw.de>

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

#include "projectfilterprovider.h"

#include <KPluginLoader>
#include <KPluginFactory>
#include <KAboutData>
#include <KSettings/Dispatcher>
#include <KConfigGroup>

#include <interfaces/iproject.h>
#include <interfaces/icore.h>
#include <interfaces/iprojectcontroller.h>

#include "projectfilterdebug.h"

using namespace KDevelop;

namespace {

Filters filtersForProject(const IProject* const project)
{
    Filters filters;
    if (!project->projectConfiguration()->hasGroup("Filters")) {
        return defaultFilters();
    }
    const KConfigGroup& config = project->projectConfiguration()->group("Filters");

    foreach(const QString& group, config.groupList()) {
        const KConfigGroup& subConfig = config.group(group);
        const QString pattern = subConfig.readEntry("pattern", QString());
        Filter::Targets targets(subConfig.readEntry("targets", 0));
        Filter::MatchOn matchOn = static_cast<Filter::MatchOn>(subConfig.readEntry("matchOn", 0));
        bool inclusive = subConfig.readEntry("inclusive", false);
        filters << Filter(pattern, targets, matchOn, inclusive);
    }

    return filters;
}

}

K_PLUGIN_FACTORY(ProjectFilterProviderFactory, registerPlugin<ProjectFilterProvider>(); )
K_EXPORT_PLUGIN(ProjectFilterProviderFactory(
    KAboutData("kdevprojectfilter", "kdevprojectfilter", ki18n("Project Filter"),
               "0.1", ki18n("Configure which files and folders inside the project folder should be included or excluded."),
               KAboutData::License_GPL)))

ProjectFilterProvider::ProjectFilterProvider( QObject* parent, const QVariantList& /*args*/ )
    : IPlugin( ProjectFilterProviderFactory::componentData(), parent )
{
    KDEV_USE_EXTENSION_INTERFACE( IProjectFilterProvider )

    connect(core()->projectController(), SIGNAL(projectClosing(KDevelop::IProject*)),
            SLOT(projectClosing(KDevelop::IProject*)));
    connect(core()->projectController(), SIGNAL(projectAboutToBeOpened(KDevelop::IProject*)),
            SLOT(projectAboutToBeOpened(KDevelop::IProject*)));

    updateProjectFilters();

    KSettings::Dispatcher::registerComponent(componentData(), this, "updateProjectFilters");
}

QSharedPointer<IProjectFilter> ProjectFilterProvider::createFilter(IProject* project) const
{
    return QSharedPointer<IProjectFilter>(new ProjectFilter(project, m_filters[project]));
}

void ProjectFilterProvider::updateProjectFilters()
{
    foreach(IProject* project, core()->projectController()->projects()) {
        Filters newFilters = filtersForProject(project);
        Filters& filters = m_filters[project];
        if (filters != newFilters) {
            projectFilterDebug() << "project filter changed:" << project->name();
            filters = newFilters;
            emit filterChanged(this, project);
        }
    }
}

void ProjectFilterProvider::projectAboutToBeOpened(IProject* project)
{
    m_filters[project] = filtersForProject(project);
}

void ProjectFilterProvider::projectClosing(IProject* project)
{
    m_filters.remove(project);
}

#include "projectfilterprovider.moc"
