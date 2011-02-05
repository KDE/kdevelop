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

static KUrl::List thread_getProjectFiles(const KUrl dir, bool recursive, const QStringList include,
                                         const QStringList exlude, volatile bool &abort)
{
    KDevelop::IProject *project = KDevelop::ICore::self()->projectController()->findProjectForUrl( dir );
    KUrl::List res;
    if(!project)
        return res;

    const QSet<IndexedString> fileSet = project->fileSet();
    foreach( const IndexedString &item, fileSet )
    {
        if(abort)
            break;
        KUrl url = item.toUrl();
        if( recursive && !dir.isParentOf(url) )
            continue;
        if( !recursive && !url.upUrl().equals(dir, KUrl::CompareWithoutTrailingSlash))
            continue;
        if( QDir::match(include, url.fileName()) && !QDir::match(exlude, url.toLocalFile()) )
            res << url;
    }

    return res;
}

static KUrl::List thread_findFiles(const QDir& dir, bool recursive, const QStringList& include,
                                           const QStringList& exclude, volatile bool &abort)
{
    QFileInfoList infos = dir.entryInfoList(include, QDir::NoDotAndDotDot|QDir::Files|QDir::Readable);
    KUrl::List dirFiles;
    foreach(const QFileInfo &currFile, infos)
    {
        QString currName = currFile.canonicalFilePath();
        if(!QDir::match(exclude, currName))
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
            dirFiles << thread_findFiles(canonical, true, include, exclude, abort);
        }
    }
    return dirFiles;
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
    QStringList include = GrepFindFilesThread::parseInclude(m_patString);
    QStringList exclude = GrepFindFilesThread::parseExclude(m_exclString);

    if(m_project)
        m_files = thread_getProjectFiles(m_directory, m_recursive, include, exclude, m_tryAbort);
    else
        m_files = thread_findFiles(m_directory.toLocalFile(), m_recursive, include, exclude, m_tryAbort);

    qSort(m_files);
}

KUrl::List GrepFindFilesThread::files() const {
    return m_files;
}

QStringList GrepFindFilesThread::parseExclude(QString excl)
{
    QStringList exclude;
    // Split around commas or spaces
    foreach(const QString &sub, excl.split(QRegExp(",|\\s"), QString::SkipEmptyParts))
        exclude << QString("*%1*").arg(sub);
    return exclude;
}

QStringList GrepFindFilesThread::parseInclude(QString inc)
{
    // Split around commas or spaces
    return inc.split(QRegExp(",|\\s"), QString::SkipEmptyParts);
}
