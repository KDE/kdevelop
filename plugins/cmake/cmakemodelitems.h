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

#include <project/projectmodel.h>
#include <interfaces/iproject.h>
#include <language/duchain/topducontext.h>
#include <parser/cmakelistsparser.h>
#include <util/path.h>

class CMakeTargetItem : public KDevelop::ProjectExecutableTargetItem
{
    public:
        CMakeTargetItem(KDevelop::ProjectFolderItem* parent, const QString& name, const KDevelop::Path &builtUrl);

        void setBuiltUrl(const KDevelop::Path &builtUrl) { m_builtUrl = builtUrl; }
        QUrl builtUrl() const override;
        QUrl installedUrl() const override;

    private:
        KDevelop::Path m_builtUrl;
};

#endif
