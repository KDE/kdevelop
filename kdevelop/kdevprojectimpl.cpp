
#include "kdevprojectimpl.h"
#include "kdevprojectimpl.moc"
#include "ckdevelop.h"
#include "cproject.h"

#include <kurl.h>
#include <kdebug.h>

KDevProjectImpl::KDevProjectImpl( QObject *parent, const char *name )
    : KDevProject( parent, name )
{
}

KDevProjectImpl::~KDevProjectImpl()
{

}

void KDevProjectImpl::openProject(const QString &dirName)
{
    kdDebug() << "KDevProjectImpl::openProject()" << endl;
    kdDebug() << "--> dirName = " << dirName << endl;
    CKDevelop::getInstance()->slotOpenProject( KURL(dirName) );
}

void KDevProjectImpl::closeProject()
{
    CKDevelop::getInstance()->slotProjectClose();
}

QString KDevProjectImpl::mainProgram()
{
    CProject* prj = CKDevelop::getInstance()->getProject();
    return prj->getExecutable();
}

QString KDevProjectImpl::projectDirectory()
{
    CProject* prj = CKDevelop::getInstance()->getProject();
    return prj->getProjectDir();
}

QStringList KDevProjectImpl::allSourceFiles()
{
    QStringList l;
#if 0
    CProject* prj = CKDevelop::getInstance()->getProject();
    if( prj ){
        for( uint i=0; i<prj->getSources().count(); ++i ){
            l << prj->getSources().at( i );
        }
        for( uint i=0; i<prj->getHeaders().count(); ++i ){
            l << prj->getHeaders().at( i );
        }
    }
#endif
    return l;
}
