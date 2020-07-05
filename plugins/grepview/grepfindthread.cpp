#include "grepfindthread.h"
#include "debug.h"

#include <QDir>
#include <QSet>

#include <project/projectmodel.h>
#include <interfaces/iproject.h>
#include <interfaces/iprojectcontroller.h>
#include <interfaces/icore.h>

#include <serialization/indexedstring.h>

#include <util/wildcardhelpers.h>

#include <algorithm>
#include <utility>

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

namespace {
class FileFinder
{
public:
    explicit FileFinder(const QStringList& include, const QStringList& exclude,
                        const std::atomic<bool>& abort)
        : m_include{include}
        , m_exclude{exclude}
        , m_abort{abort}
    {}

    void getProjectFiles(const QUrl& dir, int depth, QList<QUrl>& results);
    void findFiles(const QDir& dir, int depth, QList<QUrl>& results);

private:
    bool shouldAbort() const { return m_abort.load(std::memory_order_relaxed); }

    const QStringList& m_include;
    const QStringList& m_exclude;
    const std::atomic<bool>& m_abort;
};

void FileFinder::getProjectFiles(const QUrl& dir, int depth, QList<QUrl>& results)
{
    ///@todo This is not thread-safe!
    KDevelop::IProject *project = KDevelop::ICore::self()->projectController()->findProjectForUrl( dir );
    if(!project)
        return;

    const QSet<IndexedString> fileSet = project->fileSet();
    for (const IndexedString& item : fileSet) {
        if (shouldAbort()) {
            break;
        }
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
        if (QDir::match(m_include, url.fileName()) && !WildcardHelpers::match(m_exclude, url.toLocalFile())) {
            results.push_back(std::move(url));
        }
    }
}

void FileFinder::findFiles(const QDir& dir, int depth, QList<QUrl>& results)
{
    QFileInfoList infos = dir.entryInfoList(m_include, QDir::NoDotAndDotDot|QDir::Files|QDir::Readable|QDir::Hidden);

    if(!QFileInfo(dir.path()).isDir())
        infos << QFileInfo(dir.path());

    for (const QFileInfo& currFile : qAsConst(infos)) {
        QString currName = currFile.canonicalFilePath();
        if (!WildcardHelpers::match(m_exclude, currName)) {
            results.push_back(QUrl::fromLocalFile(currName));
        }
    }
    if(depth != 0)
    {
        constexpr QDir::Filters dirFilter = QDir::NoDotAndDotDot|QDir::AllDirs|QDir::Readable|QDir::NoSymLinks|QDir::Hidden;
        const auto dirs = dir.entryInfoList(QStringList(), dirFilter);
        for (const QFileInfo& currDir : dirs) {
            if (shouldAbort()) {
                break;
            }
            QString canonical = currDir.canonicalFilePath();
            if (!canonical.startsWith(dir.canonicalPath()))
                continue;

            if ( depth > 0 ) {
                depth--;
            }

            findFiles(canonical, depth, results);
        }
    }
}

} // namespace

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
    m_tryAbort.store(true, std::memory_order_relaxed);
}

bool GrepFindFilesThread::triesToAbort() const
{
    return m_tryAbort.load(std::memory_order_relaxed);
}

void GrepFindFilesThread::run()
{
    QStringList include = GrepFindFilesThread::parseInclude(m_patString);
    QStringList exclude = GrepFindFilesThread::parseExclude(m_exclString);

    qCDebug(PLUGIN_GREPVIEW) << "running with start dir" << m_startDirs;

    FileFinder finder(include, exclude, m_tryAbort);
    for (const QUrl& directory : m_startDirs) {
        if (m_project) {
            finder.getProjectFiles(directory, m_depth, m_files);
        } else {
            finder.findFiles(directory.toLocalFile(), m_depth, m_files);
        }
    }
}

QList<QUrl> GrepFindFilesThread::takeFiles()
{
    Q_ASSERT(isFinished());

    QList<QUrl> tmpList;
    m_files.swap(tmpList);

    std::sort(tmpList.begin(), tmpList.end());
    tmpList.erase(std::unique(tmpList.begin(), tmpList.end()), tmpList.end());
    return tmpList;
}

QStringList GrepFindFilesThread::parseExclude(const QString& excl)
{
    QStringList exclude;
    // Split around commas or spaces
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
    const auto excludesList = excl.splitRef(QRegExp(QStringLiteral(",|\\s")), Qt::SkipEmptyParts);
#else
    const auto excludesList = excl.split(QRegExp(QStringLiteral(",|\\s")), QString::SkipEmptyParts);
#endif
    exclude.reserve(excludesList.size());
    for (const auto& sub : excludesList) {
        exclude << QStringLiteral("*%1*").arg(sub);
    }
    return exclude;
}

QStringList GrepFindFilesThread::parseInclude(const QString& inc)
{
    // Split around commas or spaces
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
    return inc.split(QRegExp(QStringLiteral(",|\\s")), Qt::SkipEmptyParts);
#else
    return inc.split(QRegExp(QStringLiteral(",|\\s")), QString::SkipEmptyParts);
#endif
}
