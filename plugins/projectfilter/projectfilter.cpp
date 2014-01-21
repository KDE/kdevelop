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
    , m_projectFile( project->projectFile() )
    , m_project( project->path() )
{

}

ProjectFilter::~ProjectFilter()
{

}

bool ProjectFilter::isValid( const Path &path, const bool isFolder ) const
{
    if (!isFolder && path == m_projectFile) {
        // do not show the project file
        ///TODO: enable egain once the project page is ready for consumption
        return false;
    } else if (isFolder && path == m_project) {
        // always show the project root
        return true;
    }

    if (isFolder && path.isLocalFile() && QFile::exists(path.toLocalFile() + "/.kdev_ignore")) {
        return false;
    }

    // from here on the user can configure what he wants to see or not.

    // we operate on the path relative to the project base
    // by prepending a slash we can filter hidden files with the pattern "*/.*"

    const QString relativePath = makeRelative(path);

    if (isFolder && relativePath.endsWith(QLatin1String("/.kdev4"))) {
        return false;
    }

    bool isValid = true;
    foreach( const Filter& filter, m_filters ) {
        if (isFolder && !(filter.targets & Filter::Folders)) {
            continue;
        } else if (!isFolder && !(filter.targets & Filter::Files)) {
            continue;
        }
        if ((!isValid && filter.type == Filter::Inclusive) || (isValid && filter.type == Filter::Exclusive)) {
            const bool match = filter.pattern.exactMatch( relativePath );
            if (filter.type == Filter::Inclusive) {
                isValid = match;
            } else {
                isValid = !match;
            }
        }
    }
    return isValid;
}

QString ProjectFilter::makeRelative(const Path& path) const
{
    if (!m_project.isParentOf(path)) {
        return path.path();
    }

    return '/' + m_project.relativePath(path);
}
