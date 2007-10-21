/* KDevelop
 *
 * Copyright 2007 Andreas Pakulat <apaku@gmx.de>
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

#ifndef PROJECTKCMODULE_H
#define PROJECTKCMODULE_H

#include <kcmodule.h>
#include <QtCore/QVariant>

class KComponentData;
class QWidget;
class QStringList;

template <typename T> class ProjectKCModule : public KCModule
{
    public:
        ProjectKCModule( const KComponentData& componentData, QWidget* parent, const QVariantList& args = QVariantList() )
            : KCModule( componentData, parent, args )
        {
            Q_ASSERT( args.count() > 3 );
            T::instance( args.first().toString() );
            T::self()->setDeveloperTempFile( args.at(1).toString() );
            T::self()->setProjectFileUrl( args.at(2).toString() );
            T::self()->setDeveloperFileUrl( args.at(3).toString() );
        }
        virtual ~ProjectKCModule() {}
};

#endif

