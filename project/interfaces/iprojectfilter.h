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

#ifndef KDEVPLATFORM_IPROJECTFILTER_H
#define KDEVPLATFORM_IPROJECTFILTER_H

#include "../projectexport.h"

#include <QObject>

class KUrl;

namespace KDevelop
{

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
    virtual bool isValid(const KUrl& path, bool isFolder) const = 0;
};

}

#endif // KDEVPLATFORM_IPROJECTFILTER_H
