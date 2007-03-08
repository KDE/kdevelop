/* KDevelop QMake Support
 *
 * Copyright 2006 Andreas Pakulat <apaku@gmx.de>
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

#include "qmakebuilder.h"

#include <config.h>

#include <QtCore/QStringList>

#include <projectmodel.h>

#include <iproject.h>
#include <icore.h>
#include <iplugincontroller.h>
#include <domutil.h>
#include <QtDesigner/QExtensionFactory>


#include <kgenericfactory.h>
#include <kprocess.h>
#include <kdialog.h>
#include <kglobal.h>
#include <klocale.h>
#include <kdebug.h>

#define QMAKE_COMMAND "qmake"

typedef KGenericFactory<QMakeBuilder> QMakeBuilderFactory ;
K_EXPORT_COMPONENT_FACTORY( kdevqmakebuilder,
                            QMakeBuilderFactory( "kdevqmakebuilder" ) )


// class QMakeBuilderIQMakeBuilderFactory: public QExtensionFactory {
// public:
//     QMakeBuilderIQMakeBuilderFactory(QIQMakeBuilderManager *parent = 0)
//         :QIQMakeBuilderFactory(parent)
//     {
//         Q_UNUSED(parent)
//     }
//     protected:
//     virtual QObject *createIQMakeBuilder(QObject* object, const QString& iid, QObject* parent ) const
//     {
//         Q_UNUSED(parent)
//         if( iid != Q_TYPEID( IQMakeBuilder ) )
//             return 0;
//         QMakeBuilder* p = qobject_cast<QMakeBuilder *>(object);
//         if( !p )
//             return 0;
//         return object;
//     }
// };
KDEV_ADD_EXTENSION_FACTORY( IQMakeBuilder, QMakeBuilder )
KDEV_ADD_EXTENSION_FACTORY_NS( KDevelop, IProjectBuilder, QMakeBuilder )


QMakeBuilder::QMakeBuilder(QObject *parent, const QStringList &)
    : KDevelop::IPlugin(QMakeBuilderFactory::componentData(), parent)
{
}

QMakeBuilder::~QMakeBuilder()
{
}

bool QMakeBuilder::build(KDevelop::ProjectBaseItem *dom)
{
    Q_UNUSED( dom )
    return false;
}

bool QMakeBuilder::clean(KDevelop::ProjectBaseItem *dom)
{
    Q_UNUSED( dom )
    return false;
}

void QMakeBuilder::commandFinished(const QString &command)
{
    Q_UNUSED(command);
}

void QMakeBuilder::commandFailed(const QString &command)
{
    Q_UNUSED(command);
}

QStringList QMakeBuilder::extensions() const
{
    return QStringList() << "IQMakeBuilder" << "IProjectBuilder";
}


void QMakeBuilder::registerExtensions()
{
    extensionManager()->registerExtensions( new QMakeBuilderIQMakeBuilderFactory(
    extensionManager() ), Q_TYPEID( IQMakeBuilder ) );
    extensionManager()->registerExtensions( new QMakeBuilderIProjectBuilderFactory(
    extensionManager() ), Q_TYPEID( KDevelop::IProjectBuilder ) );
}
void QMakeBuilder::unregisterExtensions()
{
    extensionManager()->unregisterExtensions( new QMakeBuilderIQMakeBuilderFactory(
    extensionManager() ), Q_TYPEID( IQMakeBuilder ) );
    extensionManager()->unregisterExtensions( new QMakeBuilderIProjectBuilderFactory(
    extensionManager() ), Q_TYPEID( KDevelop::IProjectBuilder ) );
}


#include "qmakebuilder.moc"
// kate: space-indent on; indent-width 4; tab-width: 4; replace-tabs on; auto-insert-doxygen on
