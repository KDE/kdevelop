/* KDevelop CMake Support
 *
 * Copyright 2006 Matt Rogers <mattr@kde.org>
 * Copyright 2007 Aleix Pol <aleixpol@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#ifndef CMAKEMODELITEMS_H
#define CMAKEMODELITEMS_H

#include <QHash>

#include <projectmodel.h>
#include "cmakelistsparser.h"
#include "cmakeexport.h"
#include "cmaketypes.h"
#include "cmakeast.h"

namespace KDevelop {
class IProject;
}

/**
 * The project model item for CMake folders.
 *
 * @author Matt Rogers <mattr@kde.org>
 * @author Aleix Pol <aleixpol@gmail.com>
 */
class KDEVCMAKECOMMON_EXPORT CMakeFolderItem : public KDevelop::ProjectBuildFolderItem
{
    public:
        CMakeFolderItem( KDevelop::IProject *project, const QString &name, QStandardItem* item = 0 );

        void setIncludeDirectories(const KUrl::List &l) { m_includeList=l; }
        KUrl::List includeDirectories() const;
        Definitions definitions() const { return m_defines; }
        void setDefinitions(const Definitions& defs) { m_defines=defs; }
    private:
        KUrl::List m_includeList;
        Definitions m_defines;
};

/**
 * The project model item for CMake targets.
 *
 * @author Matt Rogers <mattr@kde.org>
 * @author Aleix Pol <aleixpol@gmail.com>
 */
class KDEVCMAKECOMMON_EXPORT CMakeTargetItem : public KDevelop::ProjectTargetItem
{
public:
    CMakeTargetItem( KDevelop::IProject *project, const QString& name, CMakeFolderItem* item );
    ~CMakeTargetItem();

    KUrl::List includeDirectories() const;
    Definitions defines() const;
private:
    CMakeFolderItem *m_parent;
};

#endif
