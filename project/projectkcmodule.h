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

#ifndef KDEVPLATFORM_PROJECTKCMODULE_H
#define KDEVPLATFORM_PROJECTKCMODULE_H

#include <kcmodule.h>

#include <QtCore/QVariant>

#include <interfaces/icore.h>
#include <interfaces/iprojectcontroller.h>

#include <util/path.h>

class KComponentData;
class QWidget;
class QStringList;

template <typename T> class ProjectKCModule : public KCModule
{
    public:
        ProjectKCModule( KAboutData* data, QWidget* parent, const QVariantList& args = QVariantList() )
            : KCModule( data, parent, args )
        {
            Q_ASSERT( args.count() > 3 );
            T::instance( args.at(0).toString() );
            T::self()->setDeveloperTempFile( args.at(0).toString() );
            T::self()->setProjectTempFile( args.at(1).toString() );
            T::self()->setProjectFile( KDevelop::Path(args.at(2).toString()) );
            T::self()->setDeveloperFile( KDevelop::Path(args.at(3).toString()) );
            projectName = args.at(4).toString();
        }
        virtual ~ProjectKCModule() {}
        KDevelop::IProject* project() const {
            return KDevelop::ICore::self()->projectController()->findProjectByName( projectName );
        }
private:
    QString projectName;
};

#endif

