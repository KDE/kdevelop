/*
    SPDX-FileCopyrightText: 2013 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "projectfilter.h"

#include <interfaces/iproject.h>

#include <QFile>

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
        ///TODO: enable again once the project page is ready for consumption
        return false;
    } else if (isFolder && path == m_project) {
        // always show the project root
        return true;
    }

    if (isFolder && path.isLocalFile() && QFile::exists(path.toLocalFile() + QLatin1String("/.kdev_ignore"))) {
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
    for (const Filter& filter : m_filters) {
        if (isFolder && !(filter.targets & Filter::Folders)) {
            continue;
        } else if (!isFolder && !(filter.targets & Filter::Files)) {
            continue;
        }
        if ((!isValid && filter.type == Filter::Inclusive) || (isValid && filter.type == Filter::Exclusive)) {
            const auto match = filter.pattern.match( relativePath );
            qDebug() << "match" << filter.type << relativePath << filter.pattern.pattern() << match;
            if (filter.type == Filter::Inclusive) {
                isValid = match.hasMatch();
            } else {
                isValid = !match.hasMatch();
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

    return QLatin1Char('/') + m_project.relativePath(path);
}
