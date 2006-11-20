/* KDevelop QMake Support
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

#include "qmakeimporter.h"

#include <QList>
#include <QVector>
#include <QDomDocument>


#include <kurl.h>
#include <kio/job.h>

#include "kdevcore.h"
#include "kdevproject.h"
#include "kgenericfactory.h"
#include "kdevprojectmodel.h"

typedef KGenericFactory<QMakeImporter> QMakeSupportFactory ;
K_EXPORT_COMPONENT_FACTORY( kdevqmakeimporter,
                            QMakeSupportFactory( "kdevqmakeimporter" ) )

QMakeImporter::QMakeImporter( QObject* parent,
                              const QStringList& )
    : KDevBuildManager( QMakeSupportFactory::instance(), parent ), m_rootItem(0L)
{
    m_project = 0;
/*    QMakeSettings* settings = QMakeSettings::self();

    //what do the settings say about our generator?
    QString generator = settings->generator();
    if ( generator.contains( "Unix" ) ) //use make
        m_builder = new KDevMakeBuilder()*/
}

QMakeImporter::~QMakeImporter()
{
    //delete m_rootItem;
}

KDevProject* QMakeImporter::project() const
{
    return m_project;
}

KUrl QMakeImporter::buildDirectory() const
{
     return project()->folder();
}

QList<KDevProjectFolderItem*> QMakeImporter::parse( KDevProjectFolderItem* item )
{
    QList<KDevProjectFolderItem*> folderList;
/*
    QMakeFolderItem* folder = dynamic_cast<QMakeFolderItem*>( item );
    if ( !folder )
        return folderList;

    FolderInfo fi = folder->folderInfo();
    for ( QStringList::iterator it = fi.includes.begin();
          it != fi.includes.end(); ++it )
    {
        KUrl urlCandidate = KUrl( ( *it ) );
        if ( m_includeDirList.indexOf( urlCandidate ) == -1 )
            m_includeDirList.append( urlCandidate );
    }

    foreach ( FolderInfo sfi, fi.subFolders )
        folderList.append( new QMakeFolderItem( sfi, folder ) );

    foreach ( TargetInfo ti, fi.targets )
    {
        QMakeTargetItem* targetItem = new QMakeTargetItem( ti, folder );
        foreach( QString sFile, ti.sources )
        {
            KUrl sourceFile = folder->url();
            sourceFile.adjustPath( KUrl::AddTrailingSlash );
            sourceFile.addPath( sFile );
            new KDevProjectFileItem( sourceFile, targetItem );
        }
    }

*/
    return folderList;
}

KDevProjectItem* QMakeImporter::import( KDevProjectModel* model,
                                           const KUrl& fileName )
{
/*    QString buildDir = QMakeSettings::self()->buildFolder();
    kDebug( 9025 ) << k_funcinfo << "build dir is " << qPrintable( buildDir ) << endl;
    KUrl qmakeInfoFile(buildDir);
    qmakeInfoFile.adjustPath( KUrl::AddTrailingSlash );
    qmakeInfoFile.addPath("qmakeinfo.xml");
    kDebug(9025) << k_funcinfo << "file is " << qmakeInfoFile.path() << endl;
    if ( !qmakeInfoFile.isLocalFile() )
    {
        //FIXME turn this into a real warning
        kWarning(9025) << "not a local file. QMake support doesn't handle remote projects" << endl;
    }
    else
    {
        m_projectInfo = m_xmlParser.parse( qmakeInfoFile );
        FolderInfo rootFolder = m_projectInfo.rootFolder;
        m_rootItem = new QMakeFolderItem( rootFolder, 0 );
        m_rootItem->setText( KDevCore::activeProject()->name() );
    }*/
    return m_rootItem;
}

KUrl QMakeImporter::findMakefile( KDevProjectFolderItem* dom ) const
{
    Q_UNUSED( dom );
    return KUrl();
}

KUrl::List QMakeImporter::findMakefiles( KDevProjectFolderItem* dom ) const
{
    Q_UNUSED( dom );
    return KUrl::List();
}

QList<KDevProjectTargetItem*> QMakeImporter::targets() const
{
    return QList<KDevProjectTargetItem*>();
}

KUrl::List QMakeImporter::includeDirectories() const
{
    return m_includeDirList;
}


// kate: indent-mode cstyle; space-indent on; indent-width 4; replace-tabs on;


