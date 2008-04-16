/* KDevelop CMake Support
 *
 * Copyright 2006 Matt Rogers <mattr@kde.org>
 * Copyright 2007-2008 Aleix Pol <aleixpol@gmail.com>
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
    class TopDUContext;
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

        void setIncludeDirectories(const QStringList &l) { m_includeList=l; }
        QStringList includeDirectories() const;
        Definitions definitions() const { return m_defines; }
        void setDefinitions(const Definitions& defs) { m_defines=defs; }
        
        void setTopDUContext(KDevelop::TopDUContext* ctx) { m_topcontext=ctx; }
        KDevelop::TopDUContext* topDUContext() const { return m_topcontext;}
    private:
        KDevelop::TopDUContext* m_topcontext;
        QStringList m_includeList;
        Definitions m_defines;
};

typedef KDevelop::ProjectTargetItem CMakeTargetItem;

#endif
