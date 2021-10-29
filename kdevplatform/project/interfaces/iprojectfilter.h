/*
    SPDX-FileCopyrightText: 2013 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_IPROJECTFILTER_H
#define KDEVPLATFORM_IPROJECTFILTER_H

#include <project/projectexport.h>

namespace KDevelop
{

class Path;

/**
 * @short The actual project filter object.
 *
 * Instances of this class should allow threadsafe filtering of project contents.
 *
 * @author Milian Wolff
 */
class KDEVPLATFORMPROJECT_EXPORT IProjectFilter
{
public:
    virtual ~IProjectFilter();

    /**
     * Check whether the given @p path should be included in a project.
     *
     * @param path is the path that you want to be checked.
     * @param isFolder distinguishes between files and folders.
     *
     * @return true when the given path should be included in the project,
     *         false otherwise, i.e. when the path should be hidden.
     */
    virtual bool isValid(const Path& path, bool isFolder) const = 0;
};

}

#endif // KDEVPLATFORM_IPROJECTFILTER_H
