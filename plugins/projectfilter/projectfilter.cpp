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

#include <interfaces/iproject.h>

#include "projectfilterdebug.h"

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
}

void ProjectFilter::updateIncludeRules( IProject* project )
{
    KConfigGroup filtersConfig = project->projectConfiguration()->group("Filters");
    QStringList includes = filtersConfig.readEntry("Includes", QStringList("*"));
    QStringList excludes = filtersConfig.readEntry("Excludes", QStringList() << "*/.*" << "*~");

    m_includeRules[project] = qMakePair(includes, excludes);
}

bool ProjectFilter::includeInProject( const KUrl &url, const bool isFolder, IProject* project ) const
{
    if ( isFolder && url.fileName() == ".kdev4" && url.upUrl() == project->folder() ) {
        return false;
    } else if ( url == project->projectFileUrl() ) {
        return false;
    }

    bool ok = isFolder;

    // we operate on the path of this url relative to the project base
    // by prepending a slash we can filter hidden files with the pattern "*/.*"
    // by appending a slash to folders we can filter them with "*/"
    const QString relativePath = '/' + project->relativeUrl( url ).path(
        isFolder ? KUrl::AddTrailingSlash : KUrl::RemoveTrailingSlash
    );

    Q_ASSERT( m_includeRules.contains( project ) );
    const IncludeRules& rules = m_includeRules.value( project );

    QStringList::ConstIterator it;
    for ( it = rules.first.constBegin(); !ok && it != rules.first.constEnd(); ++it ) {
        QRegExp rx( *it, Qt::CaseSensitive, QRegExp::Wildcard );
        if ( rx.exactMatch( relativePath ) ) {
            ok = true;
            break;
        }
    }

    if ( !ok ) {
        return false;
    }

    for ( it = rules.second.constBegin(); it != rules.second.constEnd(); ++it ) {
        QRegExp rx( *it, Qt::CaseSensitive, QRegExp::Wildcard );
        if ( rx.exactMatch( relativePath ) ) {
            return false;
        }
    }

    return true;
}

#include "projectfilter.moc"
