#include "grepfindthread.h"
#include "debug.h"

#include <QDir>
#include <QSet>

#include <project/projectmodel.h>
#include <interfaces/iproject.h>
#include <interfaces/iprojectcontroller.h>
#include <interfaces/icore.h>

#include <serialization/indexedstring.h>


using KDevelop::IndexedString;

/**
 * @return Return true in case @p url is in @p dir within a maximum depth of @p maxDepth
 */
static bool isInDirectory(const QUrl& url, const QUrl& dir, int maxDepth)
{
    QUrl folderUrl = url.adjusted(QUrl::RemoveFilename);

    int currentLevel = maxDepth;
    while(currentLevel > 0) {
        folderUrl = folderUrl.adjusted(QUrl::RemoveFilename | QUrl::StripTrailingSlash);
        if ( folderUrl == dir.adjusted(QUrl::StripTrailingSlash) ) {
            return true;
        }
        currentLevel--;
    }
    return false;
}

// the abort parameter must be volatile so that it
// is evaluated every time - optimization might prevent that

static QList<QUrl> thread_getProjectFiles(const QUrl& dir, int depth, const QStringList& include,
                                         const QStringList& exlude, volatile bool &abort)
{
    ///@todo This is not thread-safe!
    KDevelop::IProject *project = KDevelop::ICore::self()->projectController()->findProjectForUrl( dir );
    QList<QUrl> res;
    if(!project)
        return res;

    const QSet<IndexedString> fileSet = project->fileSet();
    for (const IndexedString& item : fileSet) {
        if(abort)
            break;
        QUrl url = item.toUrl();
        if( url != dir )
        {
            if ( depth == 0 ) {
                if ( url.adjusted(QUrl::RemoveFilename | QUrl::StripTrailingSlash) != dir.adjusted(QUrl::StripTrailingSlash) ) {
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

static QList<QUrl> thread_findFiles(const QDir& dir, int depth, const QStringList& include,
                                   const QStringList& exclude, volatile bool &abort)
{
    QFileInfoList infos = dir.entryInfoList(include, QDir::NoDotAndDotDot|QDir::Files|QDir::Readable);

    if(!QFileInfo(dir.path()).isDir())
        infos << QFileInfo(dir.path());

    QList<QUrl> dirFiles;
    for (const QFileInfo& currFile : qAsConst(infos)) {
        QString currName = currFile.canonicalFilePath();
        if(!QDir::match(exclude, currName))
            dirFiles << QUrl::fromLocalFile(currName);
    }
    if(depth != 0)
    {
        static const QDir::Filters dirFilter = QDir::NoDotAndDotDot|QDir::AllDirs|QDir::Readable|QDir::NoSymLinks;
        const auto dirs = dir.entryInfoList(QStringList(), dirFilter);
        for (const QFileInfo& currDir : dirs) {
            if(abort)
                break;
            QString canonical = currDir.canonicalFilePath();
            if (!canonical.startsWith(dir.canonicalPath()))
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
                                         const QList<QUrl>& startDirs,
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

    qCDebug(PLUGIN_GREPVIEW) << "running with start dir" << m_startDirs;

    for (const QUrl& directory : qAsConst(m_startDirs)) {
        if(m_project)
            m_files += thread_getProjectFiles(directory, m_depth, include, exclude, m_tryAbort);
        else
        {
            m_files += thread_findFiles(directory.toLocalFile(), m_depth, include, exclude, m_tryAbort);
        }
    }
}

QList<QUrl> GrepFindFilesThread::files() const {
    auto tmpList = QList<QUrl>::fromSet(m_files.toSet());
    std::sort(tmpList.begin(), tmpList.end());
    return tmpList;
}

QStringList GrepFindFilesThread::parseExclude(const QString& excl)
{
    QStringList exclude;
    // Split around commas or spaces
    const auto excludesList = excl.split(QRegExp(QStringLiteral(",|\\s")), QString::SkipEmptyParts);
    exclude.reserve(excludesList.size());
    for (const auto& sub : excludesList) {
        exclude << QStringLiteral("*%1*").arg(sub);
    }
    return exclude;
}

QStringList GrepFindFilesThread::parseInclude(const QString& inc)
{
    // Split around commas or spaces
    return inc.split(QRegExp(QStringLiteral(",|\\s")), QString::SkipEmptyParts);
}
