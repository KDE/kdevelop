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

#include "projectfilter.h"

#include <KPluginLoader>
#include <KPluginFactory>
#include <KAboutData>
#include <KConfigGroup>
#include <KSettings/Dispatcher>
#include <QFile>

#include <interfaces/iproject.h>
#include <interfaces/icore.h>
#include <interfaces/iprojectcontroller.h>

#include "projectfilterdebug.h"
#include <project/interfaces/iprojectfilemanager.h>

using namespace KDevelop;

K_PLUGIN_FACTORY(ProjectFilterFactory, registerPlugin<ProjectFilter>(); )
K_EXPORT_PLUGIN(ProjectFilterFactory(
    KAboutData("kdevprojectfilter", "kdevprojectfilter", ki18n("Project Filter"),
               "0.1", ki18n("Configure which files and folders inside the project folder should be included or excluded."),
               KAboutData::License_GPL)))

ProjectFilter::ProjectFilter( QObject* parent, const QVariantList& /*args*/ )
    : IPlugin( ProjectFilterFactory::componentData(), parent )
{
    KDEV_USE_EXTENSION_INTERFACE( IProjectFilter )

    connect(core()->projectController(), SIGNAL(projectClosing(KDevelop::IProject*)),
            SLOT(projectClosing(KDevelop::IProject*)));

    updateProjectFilters();

    KSettings::Dispatcher::registerComponent(componentData(), this, "updateProjectFilters");
}

Filters filtersForProject( IProject* project )
{
    const KConfigGroup& config = project->projectConfiguration()->group("Filters");
    Filters filters;

    foreach(const QString& includePattern, config.readEntry("Includes", QStringList("*"))) {
        filters.include << QRegExp( includePattern, Qt::CaseSensitive, QRegExp::Wildcard );
    }

    foreach(const QString& excludePattern, config.readEntry("Excludes", QStringList() << "*/.*")) {
        filters.exclude << QRegExp( excludePattern, Qt::CaseSensitive, QRegExp::Wildcard );
    }

    return filters;
}

bool ProjectFilter::includeInProject( const KUrl &url, const bool isFolder, IProject* project ) const
{
    if (!isFolder && url == project->projectFileUrl()) {
        // do not show the project file
        ///TODO: enable egain once the project page is ready for consumption
        return false;
    }

    // filter some common files out that probably (hopefully?) noone will ever want to see
    const QString& name = url.fileName();
    // simple equals-matching, fast and efficient and suitable for most cases
    static const QSet<QString> invalidFolders = QSet<QString>()
        << ".git" << "CVS" << ".svn" << "_svn"
        << ".kdev4" << "SCCS" << "_darcs" << ".hg" << ".bzr";
    if (isFolder && invalidFolders.contains(name)) {
        return false;
    } else if (!isFolder && (name.endsWith(".o") || name.endsWith(".a")
                          || name.startsWith("moc_") || name.endsWith(".moc")
                          || name.endsWith(".so") || name.contains(".so.")
                          || name.startsWith(".swp.") || name.endsWith('~')
                          || (name.startsWith('.') && (name.endsWith(".kate-swp") || name.endsWith(".swp")))))
    {
        return false;
    } else if (isFolder && url.isLocalFile() && QFile::exists(url.toLocalFile() + "/.kdev_ignore")) {
        return false;
    }

    // from here on the user can configure what he wants to see or not.

    QHash< IProject*, Filters >::iterator it = m_filters.find(project);
    if (it == m_filters.end()) {
        it = m_filters.insert(project, filtersForProject(project));
    }
    const Filters& filters = *it;

    // we operate on the path of this url relative to the project base
    // by prepending a slash we can filter hidden files with the pattern "*/.*"
    // by appending a slash to folders we can filter them with "*/"
    const QString relativePath = '/' + project->relativeUrl( url ).path(
        isFolder ? KUrl::AddTrailingSlash : KUrl::RemoveTrailingSlash
    );

    bool ok = isFolder;

    foreach( const QRegExp& include, filters.include ) {
        if ( include.exactMatch( relativePath ) ) {
            ok = true;
            break;
        }
    }

    if ( !ok ) {
        return false;
    }

    foreach( const QRegExp& exclude, filters.exclude ) {
        if ( exclude.exactMatch( relativePath ) ) {
            return false;
        }
    }

    return true;
}

void ProjectFilter::updateProjectFilters()
{
    foreach(IProject* project, core()->projectController()->projects()) {
        Filters newFilters = filtersForProject(project);
        Filters& filters = m_filters[project];
        if (filters != newFilters) {
            projectFilterDebug() << "reload project to take project filter changes into account:" << project->name();
            filters = newFilters;
            if (project->projectFileManager()) {
                project->projectFileManager()->reload(project->projectItem());
            }
        }
    }
}

void ProjectFilter::projectClosing(IProject* project)
{
    m_filters.remove(project);
}

#include "projectfilter.moc"
