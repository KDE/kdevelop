#include "kdevproject.h"
#include <urlutil.h>
#include <qfileinfo.h>

KDevProject::KDevProject( const QString& pluginName, const QString& icon, QObject *parent, const char *name)
    : KDevPlugin( pluginName, icon, parent, name)
{
    connect( this, SIGNAL(addedFilesToProject(const QStringList& )), this, SLOT(slotBuildFileMap()) );
    connect( this, SIGNAL(removedFilesFromProject(const QStringList& )), this, SLOT(slotBuildFileMap()) );
    connect( this, SIGNAL(changedFilesInProject(const QStringList& )), this, SLOT(slotBuildFileMap()) );
}

KDevProject::~KDevProject()
{
}

void KDevProject::changedFile( const QString & fileName )
{
    QStringList fileList;
    fileList.append ( fileName );

    emit changedFilesInProject( fileList );

}

void KDevProject::changedFiles( const QStringList & fileList )
{
    emit changedFilesInProject( fileList );
}

KDevProject::Options KDevProject::options() const
{
    return (KDevProject::Options)0;
}

bool KDevProject::isProjectFile( const QString & absFileName )
{
    return m_absToRel.contains( absFileName );
}

QString KDevProject::relativeProjectFile( const QString & absFileName )
{
    if( isProjectFile(absFileName) )
	return m_absToRel[ absFileName ];
    return QString::null;
}

void KDevProject::slotBuildFileMap( )
{
    m_absToRel.clear();
    const QStringList fileList = allFiles();
    for( QStringList::ConstIterator it=fileList.begin(); it!=fileList.end(); ++it )
    {
	QFileInfo fileInfo( projectDirectory() + "/" + *it );
	m_absToRel[ URLUtil::canonicalPath(fileInfo.absFilePath()) ] = *it;
    }
}

void KDevProject::openProject( const QString & /*dirName*/, const QString & /*projectName*/ )
{
    slotBuildFileMap();
}

#include "kdevproject.moc"
