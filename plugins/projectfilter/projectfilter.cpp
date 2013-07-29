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

#include <interfaces/iproject.h>

#include <KConfigGroup>
#include <QSet>
#include <QFile>

namespace KDevelop {

Filters filtersForProject( const IProject* const project )
{
    const KConfigGroup& config = project->projectConfiguration()->group("Filters");
    Filters filters;

    foreach(const QString& includePattern, config.readEntry("Includes", QStringList())) {
        if (includePattern == "*") {
            // optimize: this always matches, no need to run it.
            continue;
        }
        filters.include << QRegExp( includePattern, Qt::CaseSensitive, QRegExp::Wildcard );
    }

    foreach(const QString& excludePattern, config.readEntry("Excludes", QStringList() << "*/.*")) {
        filters.exclude << QRegExp( excludePattern, Qt::CaseSensitive, QRegExp::Wildcard );
    }

    return filters;
}

}

using namespace KDevelop;

ProjectFilter::ProjectFilter( const IProject* const project )
    : m_filters( filtersForProject(project) )
    , m_projectFile( project->projectFileUrl() )
    , m_project( project->folder() )
{

}

ProjectFilter::~ProjectFilter()
{

}

bool ProjectFilter::isValid( const KUrl &url, const bool isFolder ) const
{
    if (!isFolder && url == m_projectFile) {
        // do not show the project file
        ///TODO: enable egain once the project page is ready for consumption
        return false;
    } else if (isFolder && url == m_project) {
        // always show the project root
        return true;
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

    // we operate on the path of this url relative to the project base
    // by prepending a slash we can filter hidden files with the pattern "*/.*"
    // by appending a slash to folders we can filter them with "*/"

    const QString relativePath = makeRelative(url, isFolder);

    if (!isFolder && !m_filters.include.isEmpty()) {
        // only run the include pattern on files
        bool ok = false;
        foreach( const QRegExp& include, m_filters.include ) {
            if ( include.exactMatch( relativePath ) ) {
                ok = true;
                break;
            }
        }

        if ( !ok ) {
            return false;
        }
    }

    foreach( const QRegExp& exclude, m_filters.exclude ) {
        if ( exclude.exactMatch( relativePath ) ) {
            return false;
        }
    }

    return true;
}

QString ProjectFilter::makeRelative(const KUrl& url, bool isFolder) const
{
    if (!m_project.isParentOf(url)) {
        return url.path(isFolder ? KUrl::AddTrailingSlash : KUrl::RemoveTrailingSlash);
    }

    QString ret = '/' + KUrl::relativeUrl( m_project, url );
    if (isFolder && !ret.endsWith('/')) {
        ret.append('/');
    }

    return ret;
}
