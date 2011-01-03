#ifndef GREPFINDTHREAD_H
#define GREPFINDTHREAD_H

#include <QThread>
#include <KUrl>

class GrepFindFilesThread : public QThread
{
    Q_OBJECT
public:
    /**
     * @brief Constructor
     * @param[in] parent Parent
     * @param[in] startDir Root directory of the search
     * @param[in] recursive Whether the search should be recursive
     * @param[in] patterns Space-separated list of wildcard patterns to search for
     * @param[in] exclusions Space-separated list of wildcard patterns to exclude. Matches the whole path.
     * @param[in] onlyProject Whether the search should only consider project files.
     */
    GrepFindFilesThread(QObject *parent, const KUrl &startDir, bool recursive,
                    const QString &patterns, const QString &exclusions,
                    bool onlyProject);
    /**
     * @brief Returns the list of found files
     * @return List of found files
     */
    KUrl::List files() const;
    /**
     * @brief Sets the internal m_tryAbort flag to @c true
     * @note It is not guaranteed that the thread stops its work immediately.
     * Check this via QThread::isRunning() or QThread::isFinished().
     */
    void tryAbort();
    /**
     * @brief Check if the internal m_tryAbort flag is set
     * @return Whether the thread is going to abort its work.
     */
    bool triesToAbort() const;
    
    /**
     * @brief Parses include string to a list suitable for QDir::match
     */
    static QStringList parseInclude(QString inc);
    
    /**
     * @brief Parses exclude string to a list suitable for QDir::match
     */
    static QStringList parseExclude(QString excl);
    
protected:
    void run();
private:
    KUrl m_directory;
    QString m_patString;
    QString m_exclString;
    bool m_recursive;
    bool m_project;
    KUrl::List m_files;
    volatile bool m_tryAbort;
    // creating with no parameters would be bad
    GrepFindFilesThread();
};

#endif