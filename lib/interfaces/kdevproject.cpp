#include "kdevproject.h"


KDevProject::KDevProject( const QString& pluginName, const QString& icon, QObject *parent, const char *name)
    : KDevPlugin( pluginName, icon, parent, name)
{
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

KDevProject::Options KDevProject::options( )
{
    return (KDevProject::Options)0;
}

#include "kdevproject.moc"
