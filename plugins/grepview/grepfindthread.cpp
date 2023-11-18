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

namespace {
QString removeTrailingSlashes(QString dirPath)
{
    while (!dirPath.isEmpty() && std::as_const(dirPath).back() == QLatin1Char{'/'}) {
        dirPath.chop(1);
    }
    return dirPath;
}

/**
 * Is the file at @p filePath in the directory at @p dirPath within a maximum depth of @p maxDepth?
 * @param filePath the path to a file.
 * @param dirPath the path to a file or a directory without a trailing slash
 *                (an empty string is the root directory path).
 * @param maxDepth maximum depth of recursion or -1 if unlimited.
 * @note When @p dirPath points to a file rather than a directory, this function always returns @c false.
 */
bool isInDirectory(const QString& filePath, const QString& dirPath, int maxDepth)
{
    constexpr QLatin1Char slash{'/'};

    Q_ASSERT(!filePath.endsWith(slash)); // the path to a file cannot end with a slash
    Q_ASSERT(!dirPath.endsWith(slash)); // precondition

    // First check whether dirPath is a parent directory of filePath.
    // The parent directory check below is a simplified (thanks to preconditions) version of QUrl::isParentOf().

    if (!filePath.startsWith(dirPath)) {
        return false; // dirPath is not a parent directory of filePath
    }

    const auto dirPathSize = dirPath.size();
    if (filePath.size() == dirPathSize) {
        Q_ASSERT(filePath == dirPath);
        return false; // dirPath points to the same file as filePath
    }
    Q_ASSERT(filePath.size() > dirPathSize);
    if (filePath.at(dirPathSize) != slash) {
        return false; // dirPath is not a parent directory of filePath
    }

    // dirPath *is* a parent directory of filePath. Check whether it is within the maxDepth limit.

    if (maxDepth < 0) {
        return true; // unlimited depth
    }

    int indexOfSlashInUrlPath = 0;
    do {
        indexOfSlashInUrlPath = filePath.lastIndexOf(slash, indexOfSlashInUrlPath - 1);
        Q_ASSERT(indexOfSlashInUrlPath >= dirPathSize); // because dirPath is a parent directory of filePath
        if (indexOfSlashInUrlPath == dirPathSize) {
            return true;
        }
    } while (--maxDepth >= 0);
    return false;
}

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
    // Cannot use dir.adjusted(QUrl::StripTrailingSlash) here, because it does not
    // remove the single slash of the root directory. isInDirectory() requires
    // the empty-string representation of the root directory.
    const auto dirPath = removeTrailingSlashes(dir.path());

    for (const IndexedString& item : projectFileSet) {
        if (shouldAbort()) {
            break;
        }
        QUrl url = item.toUrl();
        // The scheme and authority of url match those of dir, because
        // both belong to a common project (see getProjectFileSets() below).
        auto urlPath = url.path();

        if (urlPath != dirPath && !isInDirectory(urlPath, dirPath, depth)) {
            continue;
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
