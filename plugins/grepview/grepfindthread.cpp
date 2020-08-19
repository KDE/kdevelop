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
#include <atomic>
#include <queue>
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

    void getProjectFiles(const QSet<IndexedString>& projectFileSet,
                         const QUrl& dir, int depth, QList<QUrl>& results);
    void findFiles(const QDir& dir, int depth, QList<QUrl>& results);

private:
    bool shouldAbort() const { return m_abort.load(std::memory_order_relaxed); }

    const QStringList& m_include;
    const QStringList& m_exclude;
    const std::atomic<bool>& m_abort;
};

void FileFinder::getProjectFiles(const QSet<IndexedString>& projectFileSet,
                                 const QUrl& dir, int depth, QList<QUrl>& results)
{
    for (const IndexedString& item : projectFileSet) {
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

using FileSetCollection = std::queue<QSet<IndexedString>>;

FileSetCollection getProjectFileSets(const QList<QUrl>& dirs)
{
    FileSetCollection fileSets;
    for (const QUrl& dir : dirs) {
        const auto* const project = KDevelop::ICore::self()->projectController()->findProjectForUrl(dir);
        // Store an empty file set when project==nullptr because each element
        // of fileSets must correspond to an element of dirs at the same index.
        fileSets.push(project ? project->fileSet() : FileSetCollection::value_type{});
    }
    return fileSets;
}

} // namespace

class GrepFindFilesThreadPrivate
{
public:
    const QList<QUrl> m_startDirs;
    FileSetCollection m_projectFileSets;
    const QString m_patString;
    const QString m_exclString;
    const int m_depth;
    std::atomic<bool> m_tryAbort;
    QList<QUrl> m_files;
};

GrepFindFilesThread::GrepFindFilesThread(QObject* parent,
                                         const QList<QUrl>& startDirs,
                                         int depth, const QString& pats,
                                         const QString& excl,
                                         bool onlyProject)
    : QThread(parent)
    , d_ptr(new GrepFindFilesThreadPrivate{
                startDirs,
                onlyProject ? getProjectFileSets(startDirs) : FileSetCollection{},
                pats, excl, depth, {false}, {}})
{
    setTerminationEnabled(false);
}

GrepFindFilesThread::~GrepFindFilesThread() = default;

void GrepFindFilesThread::tryAbort()
{
    Q_D(GrepFindFilesThread);

    d->m_tryAbort.store(true, std::memory_order_relaxed);
}

void GrepFindFilesThread::run()
{
    Q_D(GrepFindFilesThread);

    const QStringList include = GrepFindFilesThread::parseInclude(d->m_patString);
    const QStringList exclude = GrepFindFilesThread::parseExclude(d->m_exclString);

    qCDebug(PLUGIN_GREPVIEW) << "running with start dir" << d->m_startDirs;

    FileFinder finder(include, exclude, d->m_tryAbort);
    // m_projectFileSets contains a project file set for each element of m_startDirs at a
    // corresponding index if this search is limited to project files; is empty otherwise.
    Q_ASSERT(d->m_projectFileSets.empty() ||
                d->m_projectFileSets.size() == static_cast<std::size_t>(d->m_startDirs.size()));
    for (const QUrl& directory : d->m_startDirs) {
        if (d->m_projectFileSets.empty()) {
            finder.findFiles(directory.toLocalFile(), d->m_depth, d->m_files);
        } else {
            finder.getProjectFiles(d->m_projectFileSets.front(), directory, d->m_depth, d->m_files);
            // Removing the no longer needed file set from the collection as
            // soon as possible may save some memory or prevent a copy on write
            // if the project's file set is changed during the search.
            d->m_projectFileSets.pop();
        }
    }
}

QList<QUrl> GrepFindFilesThread::takeFiles()
{
    Q_D(GrepFindFilesThread);
    Q_ASSERT(isFinished());

    QList<QUrl> tmpList;
    d->m_files.swap(tmpList);

    std::sort(tmpList.begin(), tmpList.end());
    tmpList.erase(std::unique(tmpList.begin(), tmpList.end()), tmpList.end());
    return tmpList;
}

QStringList GrepFindFilesThread::parseExclude(const QString& excl)
{
    QStringList exclude;
    // Split around commas or spaces
    const auto excludesList = excl.splitRef(QRegExp(QStringLiteral(",|\\s")), Qt::SkipEmptyParts);
    exclude.reserve(excludesList.size());
    for (const auto& sub : excludesList) {
        exclude << QStringLiteral("*%1*").arg(sub);
    }
    return exclude;
}

QStringList GrepFindFilesThread::parseInclude(const QString& inc)
{
    // Split around commas or spaces
    return inc.split(QRegExp(QStringLiteral(",|\\s")), Qt::SkipEmptyParts);
}

#include "moc_grepfindthread.cpp"
