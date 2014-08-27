#include "grepfindthread.h"

#include <KUrl>
#include <QDir>
#include <QSet>

#include <project/projectmodel.h>
#include <interfaces/iproject.h>
#include <interfaces/iprojectcontroller.h>
#include <interfaces/icore.h>

#include <serialization/indexedstring.h>
#include <kdebug.h>

using KDevelop::IndexedString;

/**
 * @return Return true in case @p url is in @p dir within a maximum depth of @p maxDepth
 */
static bool isInDirectory(const KUrl& url, const KUrl& dir, int maxDepth)
{
    KUrl folderUrl = url.upUrl();

    int currentLevel = maxDepth;
    while(currentLevel > 0) {
        folderUrl = folderUrl.upUrl();
        if ( folderUrl.equals(dir, KUrl::CompareWithoutTrailingSlash) ) {
            return true;
        }
        currentLevel--;
    }
    return false;
}

// the abort parameter must be volatile so that it
// is evaluated every time - optimization might prevent that

static KUrl::List thread_getProjectFiles(const KUrl dir, int depth, const QStringList include,
                                         const QStringList exlude, volatile bool &abort)
{
    ///@todo This is not thread-safe!
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
        if( !url.equals(dir) )
        {
            if ( depth == 0 ) {
                if ( !url.upUrl().equals(dir, KUrl::CompareWithoutTrailingSlash) ) {
                    continue;
                }
            } else if ( !dir.isParentOf(url) ) {
                continue;
            } else if ( depth > 0 ) {
                // To ensure the current file is within the defined depth limit, navigate up the tree for as many levels
                // as the depth value, trying to find "dir", which is the project folder. If after all the loops there
                // is no match, it means the current file is deeper down the project tree than the limit depth, and so
                // it must be skipped.
                if(!isInDirectory(url, dir, depth))
                    continue;
            }
        }
        if( QDir::match(include, url.fileName()) && !QDir::match(exlude, url.toLocalFile()) )
            res << url;
    }

    return res;
}

static KUrl::List thread_findFiles(const QDir& dir, int depth, const QStringList& include,
                                   const QStringList& exclude, volatile bool &abort)
{
    QFileInfoList infos = dir.entryInfoList(include, QDir::NoDotAndDotDot|QDir::Files|QDir::Readable);
    
    if(!QFileInfo(dir.path()).isDir())
        infos << QFileInfo(dir.path());
    
    KUrl::List dirFiles;
    foreach(const QFileInfo &currFile, infos)
    {
        QString currName = currFile.canonicalFilePath();
        if(!QDir::match(exclude, currName))
            dirFiles << currName;
    }
    if(depth != 0)
    {
        static const QDir::Filters dirFilter = QDir::NoDotAndDotDot|QDir::AllDirs|QDir::Readable|QDir::NoSymLinks;
        foreach(const QFileInfo &currDir, dir.entryInfoList(QStringList(), dirFilter))
        {
            if(abort)
                break;
            QString canonical = currDir.canonicalFilePath();
            if(!KUrl(dir.canonicalPath()).isParentOf(canonical))
                continue;

            if ( depth > 0 ) {
                depth--;
            }

            dirFiles << thread_findFiles(canonical, depth, include, exclude, abort);
        }
    }
    return dirFiles;
}

GrepFindFilesThread::GrepFindFilesThread(QObject* parent,
                                         const QList<KUrl>& startDirs,
                                         int depth, const QString& pats,
                                         const QString& excl,
                                         bool onlyProject)
: QThread(parent)
, m_startDirs(startDirs)
, m_patString(pats)
, m_exclString(excl)
, m_depth(depth)
, m_project(onlyProject)
, m_tryAbort(false)
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

    kDebug() << "running with start dir" << m_startDirs;
    
    foreach(KUrl directory, m_startDirs)
    {
        if(m_project)
            m_files += thread_getProjectFiles(directory, m_depth, include, exclude, m_tryAbort);
        else
        {
            m_files += thread_findFiles(directory.toLocalFile(), m_depth, include, exclude, m_tryAbort);
        }
    }

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
