/*
    SPDX-FileCopyrightText: 2013 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef PROJECTFILTER_H
#define PROJECTFILTER_H

#include <project/interfaces/iprojectfilter.h>
#include <util/path.h>

#include "filter.h"

namespace KDevelop {

class IProject;

class ProjectFilter : public IProjectFilter
{
public:
    ProjectFilter(const IProject* const project, const Filters& filters);
    ~ProjectFilter() override;

    bool isValid(const Path& path, bool isFolder) const override;

private:
    QString makeRelative(const Path& path) const;

    Filters m_filters;
    Path m_projectFile;
    Path m_project;
};

}

#endif // PROJECTFILTER_H
