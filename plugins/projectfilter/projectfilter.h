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

#ifndef PROJECTFILTER_H
#define PROJECTFILTER_H

#include <QRegExp>
#include <QVector>

#include <KUrl>

#include <project/interfaces/iprojectfilter.h>

#include "filter.h"

namespace KDevelop {

class IProject;

class ProjectFilter : public IProjectFilter
{
public:
    ProjectFilter(const IProject* const project, const Filters& filters);
    virtual ~ProjectFilter();

    virtual bool isValid(const KUrl& path, bool isFolder) const;

private:
    QString makeRelative(const KUrl& url, bool isFolder) const;

    Filters m_filters;
    KUrl m_projectFile;
    KUrl m_project;
};

}

#endif // PROJECTFILTER_H
