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

#include "cmakeimporter.h"

#include <QList>
#include <QVector>
#include <QCoreApplication>

#include <kurl.h>

#include "kdevproject.h"
#include "kgenericfactory.h"
#include "kdevprojectmodel.h"

#include "config.h"
#include "cmaketargetitem.h"

typedef KGenericFactory<CMakeImporter> CMakeSupportFactory ;
K_EXPORT_COMPONENT_FACTORY( kdevcmakeimporter,
                            CMakeSupportFactory( "kdevcmakeimporter" ) )

CMakeImporter::CMakeImporter( QObject* parent,
                              const QStringList& )
    : KDevBuildManager( CMakeSupportFactory::instance(), parent ), m_rootItem(0L)
{
    m_project = qobject_cast<KDevProject*>( parent );
    Q_ASSERT( m_project );
/*    CMakeSettings* settings = CMakeSettings::self();

    //what do the settings say about our generator?
    QString generator = settings->generator();
    if ( generator.contains( "Unix" ) ) //use make
        m_builder = new KDevMakeBuilder()*/
}

CMakeImporter::~CMakeImporter()
{
    //delete m_rootItem;
}

KDevProject* CMakeImporter::project() const
{
    return m_project;
}

KUrl CMakeImporter::buildDirectory() const
{
     return project()->folder();
}

QList<KDevProjectFolderItem*> CMakeImporter::parse( KDevProjectFolderItem* dom )
{
    Q_UNUSED( dom );
    return QList<KDevProjectFolderItem*>();
}

KDevProjectItem* CMakeImporter::import( KDevProjectModel* model,
                                           const KUrl& fileName )
{
    QString projectPath = m_project->folder().path();
    kDebug( 9025 ) << k_funcinfo << "project path is " << projectPath << endl;
    QString buildDir = CMakeSettings::self()->buildFolder();
    kDebug( 9025 ) << k_funcinfo << "build dir is " << qPrintable( buildDir ) << endl;
    m_rootItem = new KDevProjectFolderItem( fileName, 0 );
    return m_rootItem;
}

KUrl CMakeImporter::findMakefile( KDevProjectFolderItem* dom ) const
{
    Q_UNUSED( dom );
    return KUrl();
}

KUrl::List CMakeImporter::findMakefiles( KDevProjectFolderItem* dom ) const
{
    Q_UNUSED( dom );
    return KUrl::List();
}

QList<KDevProjectTargetItem*> CMakeImporter::targets() const
{
    return QList<KDevProjectTargetItem*>();
}

KUrl::List CMakeImporter::includeDirectories() const
{
    return m_includeDirList;
}

// kate: indent-mode cstyle; space-indent on; indent-width 4; replace-tabs on;


