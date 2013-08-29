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

#include <QSet>
#include <QFile>
#include <QDebug>

using namespace KDevelop;

ProjectFilter::ProjectFilter( const IProject* const project, const QVector<Filter>& filters )
    : m_filters( filters )
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

    if (isFolder && url.isLocalFile() && QFile::exists(url.toLocalFile() + "/.kdev_ignore")) {
        return false;
    }

    const QString& name = url.fileName();

    if (isFolder && name == QLatin1String(".kdev4")) {
        return false;
    }

    // from here on the user can configure what he wants to see or not.

    // we operate on the path of this url relative to the project base
    // by prepending a slash we can filter hidden files with the pattern "*/.*"
    // by appending a slash to folders we can filter them with "*/"

    const QString relativePath = makeRelative(url, isFolder);

    bool isValid = true;
    foreach( const Filter& filter, m_filters ) {
        if (isFolder && !(filter.targets & Filter::Folders)) {
            continue;
        } else if (!isFolder && !(filter.targets & Filter::Files)) {
            continue;
        }
        if ((!isValid && filter.type == Filter::Inclusive) || (isValid && filter.type == Filter::Exclusive)) {
            const bool match = filter.pattern.exactMatch( filter.matchOn == Filter::Basename ? name : relativePath );
            if (filter.type == Filter::Inclusive) {
                isValid = match;
            } else {
                isValid = !match;
            }
        }
    }
    return isValid;
}

QString ProjectFilter::makeRelative(const KUrl& url, bool isFolder) const
{
    if (!m_project.isParentOf(url)) {
        return url.path(KUrl::RemoveTrailingSlash);
    }

    QString ret = '/' + KUrl::relativeUrl( m_project, url );
    if (isFolder && ret.endsWith('/')) {
        ret.chop(1);
    }

    return ret;
}
