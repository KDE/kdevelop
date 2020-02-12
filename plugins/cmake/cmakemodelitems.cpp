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

#include "cmakemodelitems.h"
#include "cmakeutils.h"

CMakeTargetItem::CMakeTargetItem(KDevelop::ProjectBaseItem* parent, const QString& name, const KDevelop::Path &builtUrl)
    : KDevelop::ProjectExecutableTargetItem(parent->project(), name, parent)
    , m_builtUrl(builtUrl)
{}

QUrl CMakeTargetItem::builtUrl() const
{
    if (!m_builtUrl.isEmpty())
        return m_builtUrl.toUrl();

    const KDevelop::Path buildDir = CMake::currentBuildDir(project());
    if (buildDir.isEmpty())
        return QUrl();

    QString p = project()->path().relativePath(parent()->path());
    return KDevelop::Path(KDevelop::Path(buildDir, p), text()).toUrl();
}

QUrl CMakeTargetItem::installedUrl() const
{
    return QUrl();
}
