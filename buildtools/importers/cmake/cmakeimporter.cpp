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

#include "cmTarget.h"
#include "cmMakefile.h"
#include "cmSourceFile.h"
#include "cmLocalGenerator.h"
#include "cmGlobalGenerator.h"

#include "config.h"
#include "cmaketargetitem.h"


K_EXPORT_COMPONENT_FACTORY( kdevcmakeimporter,
                            KGenericFactory<CMakeImporter>( "kdevcmakeimporter" ) )

void updateProgress( const char* text, float percent )
{
    kWarning() << text << endl;
}

CMakeImporter::CMakeImporter( QObject* parent,
                              const QStringList& )
: KDevBuildManager( parent ), m_rootItem(0L)
{
    m_project = qobject_cast<KDevProject*>( parent );
    Q_ASSERT( m_project );
    QString appPath = QCoreApplication::applicationFilePath();
    m_cmakeEngine.SetCMakeCommand( appPath.toLocal8Bit().data() );

    CMakeSettings* settings = CMakeSettings::self();

    //what do the settings say about our generator?
    QString generator = settings->generator();
    /*if ( generator.contains( "Unix" ) ) //use make
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

QList<KDevProjectFolderItem*> CMakeImporter::parse( KDevProjectFolderItem* dom )
{
    Q_UNUSED( dom );
    return QList<KDevProjectFolderItem*>();
}

KDevProjectItem* CMakeImporter::import( KDevProjectModel* model,
                                           const KUrl& fileName )
{
    QString projectPath = m_project->projectDirectory().path();
    kDebug( 9025 ) << k_funcinfo << "project path is " << projectPath << endl;
    QString buildDir = CMakeSettings::self()->buildFolder();
    kDebug( 9025 ) << k_funcinfo << "build dir is " << qPrintable( buildDir ) << endl;
    
    m_cmakeEngine.SetHomeDirectory( projectPath.toLocal8Bit().data() );
    m_cmakeEngine.SetHomeOutputDirectory( qPrintable( buildDir ) );
    m_cmakeEngine.SetStartDirectory( projectPath.toLocal8Bit().data() );
    m_cmakeEngine.SetStartOutputDirectory( qPrintable( buildDir ) );


    Q_UNUSED( model );
    m_rootItem = new KDevProjectFolderItem( fileName, 0 );

    if ( m_cmakeEngine.LoadCache() < 0 ) 
    {
        kWarning() << "Error loading the cmake cache." << endl;
        return m_rootItem;
    }

    m_cmakeEngine.PreLoadCMakeFiles();
    //add setting cache variables
    m_cmakeEngine.SetStartDirectory(m_cmakeEngine.GetHomeDirectory());
    m_cmakeEngine.SetStartOutputDirectory(m_cmakeEngine.GetHomeOutputDirectory());
    int ret = m_cmakeEngine.Configure();
    cmGlobalGenerator* rootGenerator = m_cmakeEngine.GetGlobalGenerator();
    cmLocalGenerator* localGenerator = rootGenerator->FindLocalGenerator( m_cmakeEngine.GetStartDirectory() );
    std::vector<cmLocalGenerator*> generatorVector = localGenerator->GetChildren();
    std::vector<cmLocalGenerator*>::iterator git;
    for ( git = generatorVector.begin(); git != generatorVector.end(); ++git )
    {
        KUrl url( ( *git )->GetMakefile()->GetStartDirectory() );
        KDevProjectFolderItem* item = new KDevProjectFolderItem( url, 0 );
        createProjectItems( ( *git ), item );
        m_rootItem->add( item );
    }
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

void CMakeImporter::createProjectItems( cmLocalGenerator* generator, KDevProjectItem* rootItem )
{
    cmMakefile* makefile = generator->GetMakefile();
    cmTargets targets = makefile->GetTargets();
    QString folderName = makefile->GetStartDirectory();

    cmTargets::iterator it = targets.begin(), itEnd = targets.end();
    for ( ; it != itEnd; ++it )
    {
        cmTarget target = ( *it ).second;
        CMakeTargetItem* targetItem = new CMakeTargetItem( target, rootItem );
        rootItem->add( targetItem );
        std::vector<std::string> sourceLists = target.GetSourceLists();
        std::vector<std::string>::iterator sit, sitEnd = sourceLists.end();
        for ( sit = sourceLists.begin(); sit != sitEnd; ++sit )
        {
            QString sourceName = QLatin1String( ( *sit ).c_str() );
            cmSourceFile* sf = makefile->GetSourceFileWithOutput( (*sit).c_str() );
            if ( sf )
            {
                sourceName = QLatin1String( sf->GetSourceName().c_str() );
                sourceName += QLatin1String( sf->GetSourceExtension().c_str() );
            }

            QString fullPath = folderName + QDir::separator() + sourceName;
            targetItem->add( new KDevProjectFileItem( KUrl( fullPath ) ) );
        }
    }

    std::vector<cmLocalGenerator*> generatorVector = generator->GetChildren();
    std::vector<cmLocalGenerator*>::iterator git;
    for ( git = generatorVector.begin(); git != generatorVector.end(); ++git )
    {
        KUrl url( ( *git )->GetMakefile()->GetStartDirectory() );
        KDevProjectBuildFolderItem* item = new KDevProjectBuildFolderItem( url, 0 );
        std::vector<std::string> includeList = makefile->GetIncludeDirectories();
        std::vector<std::string>::iterator it, itEnd = includeList.end();
        for ( it = includeList.begin(); it != itEnd; ++it )
        {
            KUrl urlCandidate = KUrl( QLatin1String( ( *it ).c_str() ) );
            if ( m_includeDirList.indexOf( urlCandidate ) == -1 )
                m_includeDirList.append( urlCandidate );
        }
        createProjectItems( ( *git ), item );
        rootItem->add( item );
    }
}

KUrl::List CMakeImporter::includeDirectories() const
{
    return m_includeDirList;
}

// kate: indent-mode cstyle; space-indent on; indent-width 4; replace-tabs on;


