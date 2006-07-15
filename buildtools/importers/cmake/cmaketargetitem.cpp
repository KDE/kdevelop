/* KDevelop CMake Support
 *
 * Copyright 2006 Matt Rogers <mattr@kde.org>
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

#include <QString>
#include "cmaketargetitem.h"

CMakeTargetItem::CMakeTargetItem( cmTarget target, KDevProjectItem* item)
    : KDevProjectTargetItem( QString::null, item )
{
    m_target = target;
    QString targetName = QLatin1String( target.GetFullName().c_str() );
    setName( targetName );

    std::vector<std::string> includes = m_target.GetMakefile()->GetIncludeDirectories();
    std::vector<std::string>::iterator it = includes.begin(), itEnd = includes.end();
    for ( ; it != itEnd; ++it )
        m_includeList.append( KUrl( QLatin1String( ( *it ).c_str() ) ) );

}


CMakeTargetItem::~CMakeTargetItem()
{
}


const DomUtil::PairList& CMakeTargetItem::defines() const
{
    return m_defines;
}

const KUrl::List& CMakeTargetItem::includeDirectories() const
{
    return m_includeList;
}

const QHash< QString, QString >& CMakeTargetItem::environment() const
{
    return m_environment;
}

