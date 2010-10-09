#include "grepfindthread.h"

#include <KUrl>
#include <QDir>
#include <QSet>

#include <project/projectmodel.h>
#include <interfaces/iproject.h>
#include <interfaces/iprojectcontroller.h>
#include <interfaces/icore.h>

#include <language/duchain/indexedstring.h>

using KDevelop::IndexedString;

// the abort parameter must be volatile so that it
// is evaluated every time - optimization might prevent that

static KUrl::List thread_getProjectFiles(const KUrl dir, bool recursive, const QString inclString, const QString exclString, volatile bool &abort)
{
    KDevelop::IProject *project = KDevelop::ICore::self()->projectController()->findProjectForUrl( dir );
    KUrl::List res;
    if(!project)
        return res;
    QStringList exclList;
    foreach(const QString &sub, exclString.split(',', QString::SkipEmptyParts))
        exclList << QString("*%1*").arg(sub);
    QSet<IndexedString> fileSet = project->fileSet();
    QStringList inclList = inclString.split(',');
    foreach( const IndexedString &item, fileSet )
    {
        if(abort)
            break;
        KUrl url = item.toUrl();
        if( recursive && !dir.isParentOf(url) )
            continue;
        if( !recursive && !url.upUrl().equals(dir, KUrl::CompareWithoutTrailingSlash))
            continue;
        if( QDir::match(inclList, url.fileName()) && !QDir::match(exclList, url.toLocalFile()) )
            res << url;
    }
    qSort(res);
    return res;
}

static KUrl::List thread_findFilesInternal(const QDir& dir, bool recursive, const QStringList& patList, const QStringList& exclList, volatile bool &abort)
{
    QFileInfoList infos = dir.entryInfoList(patList, QDir::NoDotAndDotDot|QDir::Files|QDir::Readable);
    KUrl::List dirFiles;
    foreach(const QFileInfo &currFile, infos)
    {
        QString currName = currFile.canonicalFilePath();
        if(!QDir::match(exclList, currName))
            dirFiles << currName;
    }
    if(recursive)
    {
        static const QDir::Filters dirFilter = QDir::NoDotAndDotDot|QDir::AllDirs|QDir::Readable|QDir::NoSymLinks;
        foreach(const QFileInfo &currDir, dir.entryInfoList(QStringList(), dirFilter))
        {
            if(abort)
                break;
            QString canonical = currDir.canonicalFilePath();
            if(!KUrl(dir.canonicalPath()).isParentOf(canonical))
                continue;
            dirFiles << thread_findFilesInternal(canonical, true, patList, exclList, abort);
        }
    }
    return dirFiles;
}

static KUrl::List thread_findFiles(const QDir dir, bool recursive, const QString inclString, const QString exclString, volatile bool &abort)
{
    QStringList exclList;
    foreach(const QString &sub, exclString.split(',', QString::SkipEmptyParts))
        exclList << QString("*%1*").arg(sub);
    KUrl::List list = thread_findFilesInternal(
        dir, recursive,
        inclString.split(',', QString::SkipEmptyParts),
        exclList,
        abort);
    qSort(list);
    return list;
}

GrepFindFilesThread::GrepFindFilesThread(QObject* parent, const KUrl& startDir, bool recursive, const QString& pats, const QString& excl, bool onlyProject)
    : QThread(parent), m_directory(startDir), m_patString(pats), m_exclString(excl), m_recursive(recursive), m_project(onlyProject), m_tryAbort(false)
{
    setTerminationEnabled(false);
}

void GrepFindFilesThread::tryAbort()
{
    m_tryAbort = true;
}

bool GrepFindFilesThread::triesToAbort() const
{
    return m_tryAbort;
}

void GrepFindFilesThread::run()
{
    if(m_project)
        m_files = thread_getProjectFiles(m_directory, m_recursive, m_patString, m_exclString, m_tryAbort);
    else
        m_files = thread_findFiles(m_directory.toLocalFile(), m_recursive, m_patString, m_exclString, m_tryAbort);
}

KUrl::List GrepFindFilesThread::files() const {
    return m_files;
}
