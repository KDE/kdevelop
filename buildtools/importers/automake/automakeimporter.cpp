/* KDevelop Automake Support
 *
 * Copyright (C)  2005  Matt Rogers <mattr@kde.org>
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

#include "automakeimporter.h"

#include <QList>

#include <kgenericfactory.h>
#include "kdevproject.h"
#include "kdevprojectmodel.h"

#include "automakeprojectmodel.h"
#include "makefileinterface.h"


K_EXPORT_COMPONENT_FACTORY( libkdevautomakeimporter,
                            KGenericFactory<AutoMakeImporter>( "kdevautomakeimporter" ) )

AutoMakeImporter::AutoMakeImporter( QObject* parent, const char* name,
                                    const QStringList& )
: KDevProjectEditor( parent ), m_rootItem(0L)
{
    setObjectName( QString::fromUtf8( name ) );
    m_project = qobject_cast<KDevProject*>( parent );
    Q_ASSERT( m_project );
    m_interface = new MakefileInterface( this );
}

AutoMakeImporter::~AutoMakeImporter()
{
	//delete m_rootItem;
}

KDevProject* AutoMakeImporter::project() const
{
    return m_project;
}

KDevProjectEditor* AutoMakeImporter::editor() const
{
    return const_cast<AutoMakeImporter*>( this );
}

QList<KDevProjectFolderItem*> AutoMakeImporter::parse( KDevProjectFolderItem* dom )
{
    Q_UNUSED( dom );
    return QList<KDevProjectFolderItem*>();
}

KDevProjectItem* AutoMakeImporter::import( KDevProjectModel* model,
                                           const QString& fileName )
{
    Q_UNUSED( model );
    m_rootItem = new AutoMakeDirItem( fileName, 0 );
    m_interface->setProjectRoot( fileName );
    bool parsedCorrectly = m_interface->parse( fileName );

    if ( parsedCorrectly )
    {
        QStringList topLevelSubdirs = m_interface->topSubDirs();
        foreach ( QString dir, topLevelSubdirs )
        {
            QString fullPath = m_interface->projectRoot();
            fullPath = fullPath + QDir::separator() + dir;
            createProjectItems( fullPath, m_rootItem );
        }
    }
    return m_rootItem;

}

QString AutoMakeImporter::findMakefile( KDevProjectFolderItem* dom ) const
{
    Q_UNUSED( dom );
    return QString();
}

QStringList AutoMakeImporter::findMakefiles( KDevProjectFolderItem* dom ) const
{
    Q_UNUSED( dom );
    return QStringList();
}

void AutoMakeImporter::createProjectItems( const QDir& folder, KDevProjectItem* rootItem )
{
    //first look for the subdirs
    //recursively descend into any other subdirs. when finished look for targets.
    //for each target, add the files for the target to the target
    KDevProjectFolderItem *folderItem = new KDevProjectFolderItem( folder, rootItem );
    rootItem->add( folderItem );

    QStringList subdirs = m_interface->subdirsFor( folder );

    if ( !subdirs.isEmpty() )
    {
        foreach( QString dir, subdirs )
        {
            QString fullPath = folder.absolutePath();
            fullPath = fullPath + QDir::separator() + dir;
            createProjectItems( fullPath, folderItem );
        }
    }

    QList<TargetInfo> targets = m_interface->targetsForFolder( folder );
    foreach( TargetInfo target, targets )
    {
        AutoMakeTargetItem* targetItem = new AutoMakeTargetItem( target, folderItem );
        folderItem->add( targetItem );
        QList<QFileInfo> targetFiles = m_interface->filesForTarget( target );
        foreach( QFileInfo fi, targetFiles )
        {
            //kDebug() << k_funcinfo << fi.absoluteFilePath() << endl;
            targetItem->add( new KDevProjectFileItem( fi, targetItem ) );
        }
    }
}

#include "automakeimporter.h"
// kate: indent-mode cstyle; space-indent on; indent-width 4; replace-tabs on;


