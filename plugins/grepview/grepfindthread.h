#ifndef KDEVPLATFORM_PLUGIN_GREPFINDTHREAD_H
#define KDEVPLATFORM_PLUGIN_GREPFINDTHREAD_H

#include <QScopedPointer>
#include <QThread>
#include <QUrl>

class GrepFindFilesThreadPrivate;

class GrepFindFilesThread : public QThread
{
    Q_OBJECT
public:
    /**
     * @brief Constructor
     * @param[in] parent Parent
     * @param[in] startDirs Root directories or files of the search
     * @param[in] depth Depth for the search. Possible values are -1 (recursive), 0 (no recursion), or integers
     *                  from 1 on indicating the level of subfolders allowed in the recursion.
     * @param[in] patterns Space-separated list of wildcard patterns to search for
     * @param[in] exclusions Space-separated list of wildcard patterns to exclude. Matches the whole path.
     * @param[in] onlyProject Whether the search should only consider project files.
     */
    GrepFindFilesThread(QObject *parent, const QList<QUrl> &startDirs, int depth,
                    const QString &patterns, const QString &exclusions,
                    bool onlyProject);

    ~GrepFindFilesThread() override;

    /**
     * @note This function may be called only after run() returns, e.g. in a slot
     *       connected to QThread::finished().
     * @return The list of found files when called for the first time;
     *         an empty list on subsequent calls.
     */
    QList<QUrl> takeFiles();
    /**
     * @brief Sets the internal m_tryAbort flag to @c true
     * @note It is not guaranteed that the thread stops its work immediately.
     * Check this via QThread::isRunning() or QThread::isFinished().
     */
    void tryAbort();

    /**
     * @brief Parses include string to a list suitable for QDir::match
     */
    static QStringList parseInclude(const QString& inc);
    
    /**
     * @brief Parses exclude string to a list suitable for QDir::match
     */
    static QStringList parseExclude(const QString& excl);
    
protected:
    void run() override;

private:
    const QScopedPointer<class GrepFindFilesThreadPrivate> d_ptr;
    Q_DECLARE_PRIVATE(GrepFindFilesThread)
};

#endif
