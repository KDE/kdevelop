/****************************************************************************
**
** Copyright (C) 2021 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of Qt Creator.
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 as published by the Free Software
** Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
****************************************************************************/

#include "filepath.h"

#include "algorithm.h"
#include "environment.h"
#include "fileutils.h"
#include "hostosinfo.h"
#include "qtcassert.h"

#include <QtGlobal>
#include <QDateTime>
#include <QDebug>
#include <QDirIterator>
#include <QFileInfo>
#include <QRegularExpression>
#include <QStorageInfo>
#include <QUrl>

#ifdef Q_OS_WIN
#ifdef QTCREATOR_PCH_H
#define CALLBACK WINAPI
#endif
#include <qt_windows.h>
#include <shlobj.h>
#endif

#ifdef Q_OS_OSX
#include "fileutils_mac.h"
#endif

QT_BEGIN_NAMESPACE
QDebug operator<<(QDebug dbg, const Utils::FilePath &c)
{
    return dbg << c.toString();
}

QT_END_NAMESPACE

namespace Utils {

static DeviceFileHooks s_deviceHooks;

/*! \class Utils::FileUtils

  \brief The FileUtils class contains file and directory related convenience
  functions.

*/

static bool removeRecursivelyLocal(const FilePath &filePath, QString *error)
{
    QTC_ASSERT(!filePath.needsDevice(), return false);
    QFileInfo fileInfo = filePath.toFileInfo();
    if (!fileInfo.exists() && !fileInfo.isSymLink())
        return true;
    QFile::setPermissions(filePath.toString(), fileInfo.permissions() | QFile::WriteUser);
    if (fileInfo.isDir()) {
        QDir dir(filePath.toString());
        dir.setPath(dir.canonicalPath());
        if (dir.isRoot()) {
            if (error) {
                *error = QCoreApplication::translate("Utils::FileUtils",
                    "Refusing to remove root directory.");
            }
            return false;
        }
        if (dir.path() == QDir::home().canonicalPath()) {
            if (error) {
                *error = QCoreApplication::translate("Utils::FileUtils",
                    "Refusing to remove your home directory.");
            }
            return false;
        }

        const QStringList fileNames = dir.entryList(
                    QDir::Files | QDir::Hidden | QDir::System | QDir::Dirs | QDir::NoDotAndDotDot);
        for (const QString &fileName : fileNames) {
            if (!removeRecursivelyLocal(filePath / fileName, error))
                return false;
        }
        if (!QDir::root().rmdir(dir.path())) {
            if (error) {
                *error = QCoreApplication::translate("Utils::FileUtils", "Failed to remove directory \"%1\".")
                        .arg(filePath.toUserOutput());
            }
            return false;
        }
    } else {
        if (!QFile::remove(filePath.toString())) {
            if (error) {
                *error = QCoreApplication::translate("Utils::FileUtils", "Failed to remove file \"%1\".")
                        .arg(filePath.toUserOutput());
            }
            return false;
        }
    }
    return true;
}

// Cleans part after optional :// scheme separator, similar to QDir::cleanPath()
//  - directory separators normalized (that is, platform-native
//    separators converted to "/") and redundant ones removed, and "."s and ".."s
//    resolved (as far as possible).
//   Symbolic links are kept. This function does not return the
//    canonical path, but rather the simplest version of the input.
//    For example, "./local" becomes "local", "local/../bin" becomes
//    "bin" and "/local/usr/../bin" becomes "/local/bin".

// FIXME: This should not use the host-platform dependent QDir::cleanPath()

static QString doCleanPath(const QString &input)
{
    const int pos = input.indexOf("://");
    if (pos == -1)
        return QDir::cleanPath(input);
    return input.left(pos + 3) + QDir::cleanPath(input.right(pos + 3));
}

/*!
  Copies the directory specified by \a srcFilePath recursively to \a tgtFilePath. \a tgtFilePath will contain
  the target directory, which will be created. Example usage:

  \code
    QString error;
    bool ok = Utils::FileUtils::copyRecursively("/foo/bar", "/foo/baz", &error);
    if (!ok)
      qDebug() << error;
  \endcode

  This will copy the contents of /foo/bar into to the baz directory under /foo, which will be created in the process.

  \note The \a error parameter is optional.

  Returns whether the operation succeeded.
*/

bool FileUtils::copyRecursively(const FilePath &srcFilePath, const FilePath &tgtFilePath, QString *error)
{
    return copyRecursively(
        srcFilePath, tgtFilePath, error, [](const FilePath &src, const FilePath &dest, QString *error) {
            if (!src.copyFile(dest)) {
                if (error) {
                    *error = QCoreApplication::translate("Utils::FileUtils",
                                                         "Could not copy file \"%1\" to \"%2\".")
                                 .arg(src.toUserOutput(), dest.toUserOutput());
                }
                return false;
            }
            return true;
        });
}

/*!
  Copies a file specified by \a srcFilePath to \a tgtFilePath only if \a srcFilePath is different
  (file contents and last modification time).

  Returns whether the operation succeeded.
*/

bool FileUtils::copyIfDifferent(const FilePath &srcFilePath, const FilePath &tgtFilePath)
{
    QTC_ASSERT(srcFilePath.exists(), return false);
    QTC_ASSERT(srcFilePath.scheme() == tgtFilePath.scheme(), return false);
    QTC_ASSERT(srcFilePath.host() == tgtFilePath.host(), return false);

    if (tgtFilePath.exists()) {
        const QDateTime srcModified = srcFilePath.lastModified();
        const QDateTime tgtModified = tgtFilePath.lastModified();
        if (srcModified == tgtModified) {
            const QByteArray srcContents = srcFilePath.fileContents();
            const QByteArray tgtContents = srcFilePath.fileContents();
            if (srcContents == tgtContents)
                return true;
        }
        tgtFilePath.removeFile();
    }

    return srcFilePath.copyFile(tgtFilePath);
}

/*!
  If this is a directory, the function will recursively check all files and return
  true if one of them is newer than \a timeStamp. If this is a single file, true will
  be returned if the file is newer than \a timeStamp.

  Returns whether at least one file in \a filePath has a newer date than
  \a timeStamp.
*/
bool FilePath::isNewerThan(const QDateTime &timeStamp) const
{
    if (!exists() || lastModified() >= timeStamp)
        return true;
    if (isDir()) {
        const FilePaths dirContents = dirEntries(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);
        for (const FilePath &entry : dirContents) {
            if (entry.isNewerThan(timeStamp))
                return true;
        }
    }
    return false;
}

Qt::CaseSensitivity FilePath::caseSensitivity() const
{
    if (m_scheme.isEmpty())
        return HostOsInfo::fileNameCaseSensitivity();

    // FIXME: This could or possibly should the target device's file name case sensitivity
    // into account by diverting to IDevice. However, as this is expensive and we are
    // in time-critical path here, we go with "good enough" for now:
    // The first approximation is "Anything unusual is not case sensitive"
    return Qt::CaseSensitive;
}

/*!
  Recursively resolves symlinks if this is a symlink.
  To resolve symlinks anywhere in the path, see canonicalPath.
  Unlike QFileInfo::canonicalFilePath(), this function will still return the expected deepest
  target file even if the symlink is dangling.

  \note Maximum recursion depth == 16.

  Returns the symlink target file path.
*/
FilePath FilePath::resolveSymlinks() const
{
    FilePath current = *this;
    int links = 16;
    while (links--) {
        const FilePath target = current.symLinkTarget();
        if (target.isEmpty())
            return current;
        current = target;
    }
    return current;
}

/*!
  Recursively resolves possibly present symlinks in this file name.
  Unlike QFileInfo::canonicalFilePath(), this function will not return an empty
  string if path doesn't exist.

  Returns the canonical path.
*/
FilePath FilePath::canonicalPath() const
{
    if (needsDevice()) {
        // FIXME: Not a full solution, but it stays on the right device.
        return *this;
    }
    const QString result = toFileInfo().canonicalFilePath();
    if (result.isEmpty())
        return *this;
    return FilePath::fromString(result);
}

FilePath FilePath::operator/(const QString &str) const
{
    return pathAppended(str);
}

void FilePath::clear()
{
    m_data.clear();
    m_host.clear();
    m_scheme.clear();
}

bool FilePath::isEmpty() const
{
    return m_data.isEmpty();
}

/*!
  Like QDir::toNativeSeparators(), but use prefix '~' instead of $HOME on unix systems when an
  absolute path is given.

  Returns the possibly shortened path with native separators.
*/
QString FilePath::shortNativePath() const
{
    if (HostOsInfo::isAnyUnixHost()) {
        const FilePath home = FileUtils::homePath();
        if (isChildOf(home)) {
            return QLatin1Char('~') + QDir::separator()
                + QDir::toNativeSeparators(relativeChildPath(home).toString());
        }
    }
    return toUserOutput();
}

QString FileUtils::fileSystemFriendlyName(const QString &name)
{
    QString result = name;
    result.replace(QRegularExpression(QLatin1String("\\W")), QLatin1String("_"));
    result.replace(QRegularExpression(QLatin1String("_+")), QLatin1String("_")); // compact _
    result.remove(QRegularExpression(QLatin1String("^_*"))); // remove leading _
    result.remove(QRegularExpression(QLatin1String("_+$"))); // remove trailing _
    if (result.isEmpty())
        result = QLatin1String("unknown");
    return result;
}

int FileUtils::indexOfQmakeUnfriendly(const QString &name, int startpos)
{
    static const QRegularExpression checkRegExp(QLatin1String("[^a-zA-Z0-9_.-]"));
    return checkRegExp.match(name, startpos).capturedStart();
}

QString FileUtils::qmakeFriendlyName(const QString &name)
{
    QString result = name;

    // Remove characters that might trip up a build system (especially qmake):
    int pos = indexOfQmakeUnfriendly(result);
    while (pos >= 0) {
        result[pos] = QLatin1Char('_');
        pos = indexOfQmakeUnfriendly(result, pos);
    }
    return fileSystemFriendlyName(result);
}

bool FileUtils::makeWritable(const FilePath &path)
{
    return path.setPermissions(path.permissions() | QFile::WriteUser);
}

// makes sure that capitalization of directories is canonical on Windows and OS X.
// This mimics the logic in QDeclarative_isFileCaseCorrect
QString FileUtils::normalizedPathName(const QString &name)
{
#ifdef Q_OS_WIN
    const QString nativeSeparatorName(QDir::toNativeSeparators(name));
    const auto nameC = reinterpret_cast<LPCTSTR>(nativeSeparatorName.utf16()); // MinGW
    PIDLIST_ABSOLUTE file;
    HRESULT hr = SHParseDisplayName(nameC, NULL, &file, 0, NULL);
    if (FAILED(hr))
        return name;
    TCHAR buffer[MAX_PATH];
    const bool success = SHGetPathFromIDList(file, buffer);
    ILFree(file);
    return success ? QDir::fromNativeSeparators(QString::fromUtf16(reinterpret_cast<const ushort *>(buffer)))
                   : name;
#elif defined(Q_OS_OSX)
    return Internal::normalizePathName(name);
#else // do not try to handle case-insensitive file systems on Linux
    return name;
#endif
}

static bool isRelativePathHelper(const QString &path, OsType osType)
{
    if (path.startsWith('/'))
        return false;
    if (osType == OsType::OsTypeWindows) {
        if (path.startsWith('\\'))
            return false;
        // Unlike QFileInfo, this won't accept a relative path with a drive letter.
        // Such paths result in a royal mess anyway ...
        if (path.length() >= 3 && path.at(1) == ':' && path.at(0).isLetter()
                && (path.at(2) == '/' || path.at(2) == '\\'))
            return false;
    }
    return true;
}

bool FileUtils::isRelativePath(const QString &path)
{
    return isRelativePathHelper(path, HostOsInfo::hostOs());
}

bool FilePath::isRelativePath() const
{
    return isRelativePathHelper(m_data, osType());
}

FilePath FilePath::resolvePath(const FilePath &tail) const
{
    if (!isRelativePathHelper(tail.m_data, osType()))
        return tail;
    return pathAppended(tail.m_data);
}

FilePath FilePath::resolvePath(const QString &tail) const
{
    if (!FileUtils::isRelativePath(tail))
        return FilePath::fromString(doCleanPath(tail));
    FilePath result = *this;
    result.setPath(doCleanPath(m_data + '/' + tail));
    return result;
}

FilePath FilePath::cleanPath() const
{
    FilePath result = *this;
    result.setPath(doCleanPath(result.path()));
    return result;
}

FilePath FileUtils::commonPath(const FilePath &oldCommonPath, const FilePath &filePath)
{
    FilePath newCommonPath = oldCommonPath;
    while (!newCommonPath.isEmpty() && !filePath.isChildOf(newCommonPath))
        newCommonPath = newCommonPath.parentDir();
    return newCommonPath.canonicalPath();
}

FilePath FileUtils::homePath()
{
    return FilePath::fromString(doCleanPath(QDir::homePath()));
}


/*! \class Utils::FilePath

    \brief The FilePath class is a light-weight convenience class for filenames.

    On windows filenames are compared case insensitively.
*/

FilePath::FilePath()
{
}

/// Constructs a FilePath from \a info
FilePath FilePath::fromFileInfo(const QFileInfo &info)
{
    return FilePath::fromString(info.absoluteFilePath());
}

/// \returns a QFileInfo
QFileInfo FilePath::toFileInfo() const
{
    QTC_ASSERT(!needsDevice(), return QFileInfo());
    return QFileInfo(cleanPath().path());
}

FilePath FilePath::fromUrl(const QUrl &url)
{
    FilePath fn;
    fn.m_scheme = url.scheme();
    fn.m_host = url.host();
    fn.m_data = url.path();
    return fn;
}

static QString hostEncoded(QString host)
{
    host.replace('%', "%25");
    host.replace('/', "%2f");
    return host;
}

/// \returns a QString for passing on to QString based APIs
QString FilePath::toString() const
{
    if (m_scheme.isEmpty())
        return m_data;
    if (m_data.startsWith('/'))
        return m_scheme + "://" + hostEncoded(m_host) + m_data;
    return m_scheme + "://" + hostEncoded(m_host) + "/./" + m_data;
}

QUrl FilePath::toUrl() const
{
    QUrl url;
    url.setScheme(m_scheme);
    url.setHost(m_host);
    url.setPath(m_data);
    return url;
}

void FileUtils::setDeviceFileHooks(const DeviceFileHooks &hooks)
{
    s_deviceHooks = hooks;
}

/// \returns a QString to display to the user, including the device prefix
/// Converts the separators to the native format of the system
/// this path belongs to.
QString FilePath::toUserOutput() const
{
    FilePath tmp = *this;
    if (osType() == OsTypeWindows)
        tmp.m_data.replace('/', '\\');
    return tmp.toString();
}

/// \returns a QString to pass to target system native commands, without the device prefix.
/// Converts the separators to the native format of the system
/// this path belongs to.
QString FilePath::nativePath() const
{
    QString data = m_data;
    if (osType() == OsTypeWindows)
        data.replace('/', '\\');
    return data;
}

QString FilePath::fileName() const
{
    const QChar slash = QLatin1Char('/');
    return m_data.mid(m_data.lastIndexOf(slash) + 1);
}

QString FilePath::fileNameWithPathComponents(int pathComponents) const
{
    if (pathComponents < 0)
        return m_data;
    const QChar slash = QLatin1Char('/');
    int i = m_data.lastIndexOf(slash);
    if (pathComponents == 0 || i == -1)
        return m_data.mid(i + 1);
    int component = i + 1;
    // skip adjacent slashes
    while (i > 0 && m_data.at(--i) == slash)
        ;
    while (i >= 0 && --pathComponents >= 0) {
        i = m_data.lastIndexOf(slash, i);
        component = i + 1;
        while (i > 0 && m_data.at(--i) == slash)
            ;
    }

    if (i > 0 && m_data.lastIndexOf(slash, i) != -1)
        return m_data.mid(component);

    // If there are no more slashes before the found one, return the entire string
    return displayName();
}

/// \returns the base name of the file without the path.
///
/// The base name consists of all characters in the file up to
/// (but not including) the first '.' character.

QString FilePath::baseName() const
{
    const QString &name = fileName();
    return name.left(name.indexOf('.'));
}

/// \returns the complete base name of the file without the path.
///
/// The complete base name consists of all characters in the file up to
/// (but not including) the last '.' character. In case of ".ui.qml"
/// it will be treated as one suffix.

QString FilePath::completeBaseName() const
{
    const QString &name = fileName();
    if (name.endsWith(".ui.qml"))
        return name.left(name.length() - QString(".ui.qml").length());
    return name.left(name.lastIndexOf('.'));
}

/// \returns the suffix (extension) of the file.
///
/// The suffix consists of all characters in the file after
/// (but not including) the last '.'. In case of ".ui.qml" it will
/// be treated as one suffix.

QString FilePath::suffix() const
{
    const QString &name = fileName();
    if (name.endsWith(".ui.qml"))
        return "ui.qml";
    const int index = name.lastIndexOf('.');
    if (index >= 0)
        return name.mid(index + 1);
    return {};
}

/// \returns the complete suffix (extension) of the file.
///
/// The complete suffix consists of all characters in the file after
/// (but not including) the first '.'.

QString FilePath::completeSuffix() const
{
    const QString &name = fileName();
    const int index = name.indexOf('.');
    if (index >= 0)
        return name.mid(index + 1);
    return {};
}

void FilePath::setScheme(const QString &scheme)
{
    QTC_CHECK(!scheme.contains('/'));
    m_scheme = scheme;
}

void FilePath::setHost(const QString &host)
{
    m_host = host;
}


/// \returns a bool indicating whether a file with this
/// FilePath exists.
bool FilePath::exists() const
{
    if (needsDevice()) {
        QTC_ASSERT(s_deviceHooks.exists, return false);
        return s_deviceHooks.exists(*this);
    }
    return !isEmpty() && QFileInfo::exists(m_data);
}

/// \returns a bool indicating whether a path is writable.
bool FilePath::isWritableDir() const
{
    if (needsDevice()) {
        QTC_ASSERT(s_deviceHooks.isWritableDir, return false);
        return s_deviceHooks.isWritableDir(*this);
    }
    const QFileInfo fi{m_data};
    return exists() && fi.isDir() && fi.isWritable();
}

bool FilePath::isWritableFile() const
{
    if (needsDevice()) {
        QTC_ASSERT(s_deviceHooks.isWritableFile, return false);
        return s_deviceHooks.isWritableFile(*this);
    }
    const QFileInfo fi{m_data};
    return fi.isWritable() && !fi.isDir();
}

bool FilePath::ensureWritableDir() const
{
    if (needsDevice()) {
        QTC_ASSERT(s_deviceHooks.ensureWritableDir, return false);
        return s_deviceHooks.ensureWritableDir(*this);
    }
    const QFileInfo fi{m_data};
    if (fi.isDir() && fi.isWritable())
        return true;
    return QDir().mkpath(m_data);
}

bool FilePath::ensureExistingFile() const
{
    if (needsDevice()) {
        QTC_ASSERT(s_deviceHooks.ensureExistingFile, return false);
        return s_deviceHooks.ensureExistingFile(*this);
    }
    QFile f(m_data);
    if (f.exists())
        return true;
    f.open(QFile::WriteOnly);
    f.close();
    return f.exists();
}

bool FilePath::isExecutableFile() const
{
    if (needsDevice()) {
        QTC_ASSERT(s_deviceHooks.isExecutableFile, return false);
        return s_deviceHooks.isExecutableFile(*this);
    }
    const QFileInfo fi{m_data};
    return fi.isExecutable() && !fi.isDir();
}

bool FilePath::isReadableFile() const
{
    if (needsDevice()) {
        QTC_ASSERT(s_deviceHooks.isReadableFile, return false);
        return s_deviceHooks.isReadableFile(*this);
    }
    const QFileInfo fi{m_data};
    return fi.isReadable() && !fi.isDir();
}

bool FilePath::isReadableDir() const
{
    if (needsDevice()) {
        QTC_ASSERT(s_deviceHooks.isReadableDir, return false);
        return s_deviceHooks.isReadableDir(*this);
    }
    const QFileInfo fi{m_data};
    return fi.isReadable() && fi.isDir();
}

bool FilePath::isFile() const
{
    if (needsDevice()) {
        QTC_ASSERT(s_deviceHooks.isFile, return false);
        return s_deviceHooks.isFile(*this);
    }
    const QFileInfo fi{m_data};
    return fi.isFile();
}

bool FilePath::isDir() const
{
    if (needsDevice()) {
        QTC_ASSERT(s_deviceHooks.isDir, return false);
        return s_deviceHooks.isDir(*this);
    }
    const QFileInfo fi{m_data};
    return fi.isDir();
}

bool FilePath::createDir() const
{
    if (needsDevice()) {
        QTC_ASSERT(s_deviceHooks.createDir, return false);
        return s_deviceHooks.createDir(*this);
    }
    QDir dir(m_data);
    return dir.mkpath(dir.absolutePath());
}

FilePaths FilePath::dirEntries(const FileFilter &filter, QDir::SortFlags sort) const
{
    FilePaths result;

    if (needsDevice()) {
        QTC_ASSERT(s_deviceHooks.iterateDirectory, return {});
        const auto callBack = [&result](const FilePath &path) { result.append(path); return true; };
        s_deviceHooks.iterateDirectory(*this, callBack, filter);
    } else {
        QDirIterator dit(m_data, filter.nameFilters, filter.fileFilters, filter.iteratorFlags);
        while (dit.hasNext())
            result.append(FilePath::fromString(dit.next()));
    }

    // FIXME: Not all flags supported here.

    const QDir::SortFlags sortBy = (sort & QDir::SortByMask);
    if (sortBy == QDir::Name) {
        Utils::sort(result);
    } else if (sortBy == QDir::Time) {
        Utils::sort(result, [](const FilePath &path1, const FilePath &path2) {
            return path1.lastModified() < path2.lastModified();
        });
    }

    if (sort & QDir::Reversed)
        std::reverse(result.begin(), result.end());

    return result;
}

FilePaths FilePath::dirEntries(QDir::Filters filters) const
{
    return dirEntries(FileFilter({}, filters));
}

// This runs \a callBack on each directory entry matching all \a filters and
// either of the specified \a nameFilters.
// An empty \nameFilters list matches every name.

void FilePath::iterateDirectory(const std::function<bool(const FilePath &item)> &callBack,
                                const FileFilter &filter) const
{
    if (needsDevice()) {
        QTC_ASSERT(s_deviceHooks.iterateDirectory, return);
        s_deviceHooks.iterateDirectory(*this, callBack, filter);
        return;
    }

    QDirIterator it(m_data, filter.nameFilters, filter.fileFilters, filter.iteratorFlags);
    while (it.hasNext()) {
        if (!callBack(FilePath::fromString(it.next())))
            return;
    }
}

void FilePath::iterateDirectories(const FilePaths &dirs,
                                  const std::function<bool(const FilePath &)> &callBack,
                                  const FileFilter &filter)
{
    for (const FilePath &dir : dirs)
        dir.iterateDirectory(callBack, filter);
}

QByteArray FilePath::fileContents(qint64 maxSize, qint64 offset) const
{
    if (needsDevice()) {
        QTC_ASSERT(s_deviceHooks.fileContents, return {});
        return s_deviceHooks.fileContents(*this, maxSize, offset);
    }

    const QString path = toString();
    QFile f(path);
    if (!f.exists())
        return {};

    if (!f.open(QFile::ReadOnly))
        return {};

    if (offset != 0)
        f.seek(offset);

    if (maxSize != -1)
        return f.read(maxSize);

    return f.readAll();
}

void FilePath::asyncFileContents(const Continuation<const QByteArray &> &cont,
                                 qint64 maxSize, qint64 offset) const
{
    if (needsDevice()) {
        QTC_ASSERT(s_deviceHooks.asyncFileContents, return);
        return s_deviceHooks.asyncFileContents(cont, *this, maxSize, offset);
    }

    cont(fileContents(maxSize, offset));
}

bool FilePath::writeFileContents(const QByteArray &data) const
{
    if (needsDevice()) {
        QTC_ASSERT(s_deviceHooks.writeFileContents, return {});
        return s_deviceHooks.writeFileContents(*this, data);
    }

    QFile file(path());
    QTC_ASSERT(file.open(QFile::WriteOnly | QFile::Truncate), return false);
    qint64 res = file.write(data);
    return res == data.size();
}

void FilePath::asyncWriteFileContents(const Continuation<bool> &cont, const QByteArray &data) const
{
    if (needsDevice()) {
        QTC_ASSERT(s_deviceHooks.asyncWriteFileContents, return);
        s_deviceHooks.asyncWriteFileContents(cont, *this, data);
        return;
    }

    cont(writeFileContents(data));
}

bool FilePath::needsDevice() const
{
    return !m_scheme.isEmpty();
}

/// \returns an empty FilePath if this is not a symbolic linl
FilePath FilePath::symLinkTarget() const
{
    if (needsDevice()) {
        QTC_ASSERT(s_deviceHooks.symLinkTarget, return {});
        return s_deviceHooks.symLinkTarget(*this);
    }
    const QFileInfo info(m_data);
    if (!info.isSymLink())
        return {};
    return FilePath::fromString(info.symLinkTarget());
}

QString FilePath::mapToDevicePath() const
{
    if (needsDevice()) {
        QTC_ASSERT(s_deviceHooks.mapToDevicePath, return {});
        return s_deviceHooks.mapToDevicePath(*this);
    }
    return m_data;
}

FilePath FilePath::withExecutableSuffix() const
{
    FilePath res = *this;
    res.setPath(OsSpecificAspects::withExecutableSuffix(osType(), m_data));
    return res;
}

/// Find the parent directory of a given directory.

/// Returns an empty FilePath if the current directory is already
/// a root level directory.

/// \returns \a FilePath with the last segment removed.
FilePath FilePath::parentDir() const
{
    const QString basePath = path();
    if (basePath.isEmpty())
        return FilePath();

    const QDir base(basePath);
    if (base.isRoot())
        return FilePath();

    const QString path = basePath + QLatin1String("/..");
    const QString parent = doCleanPath(path);
    QTC_ASSERT(parent != path, return FilePath());

    FilePath result = *this;
    result.setPath(parent);
    return result;
}

FilePath FilePath::absolutePath() const
{
    if (isAbsolutePath())
        return parentDir();
    QTC_ASSERT(!needsDevice(), return *this);
    FilePath result = *this;
    result.m_data = QFileInfo(m_data).absolutePath();
    return result;
}

FilePath FilePath::absoluteFilePath() const
{
    if (isAbsolutePath())
        return *this;
    QTC_ASSERT(!needsDevice(), return *this);
    FilePath result = *this;
    result.m_data = QFileInfo(m_data).absoluteFilePath();
    return result;
}

FilePath FilePath::normalizedPathName() const
{
    FilePath result = *this;
    if (!needsDevice()) // FIXME: Assumes no remote Windows and Mac for now.
        result.m_data = FileUtils::normalizedPathName(result.m_data);
    return result;
}

QString FilePath::displayName(const QString &args) const
{
    QString deviceName;
    if (needsDevice()) {
        QTC_ASSERT(s_deviceHooks.deviceDisplayName, return m_data);
        deviceName = s_deviceHooks.deviceDisplayName(*this);
    }

    if (args.isEmpty()) {
        if (deviceName.isEmpty())
            return m_data;

        return QCoreApplication::translate("Utils::FileUtils", "%1 on %2", "File on device")
                .arg(m_data, deviceName);
    }

    if (deviceName.isEmpty())
        return m_data + ' ' + args;

    return QCoreApplication::translate("Utils::FileUtils", "%1 %2 on %3", "File and args on device")
            .arg(m_data, args, deviceName);
}

/*!
   Constructs a FilePath from \a filepath

   \a filepath is not checked for validity. It can be given in the following forms:

   \list
   \li  /some/absolute/local/path
   \li  some/relative/path
   \li  scheme://host/absolute/path
   \li  scheme://host/./relative/path    \note the ./ is verbatim part of the path
   \endlist

   Some decoding happens when parsing the \a filepath
   A sequence %25 present in the host part is replaced by % in the host name,
   a sequence %2f present in the host part is replaced by / in the host name.

   The path part might consist of several parts separated by /, independent
   of the platform or file system.

   To create FilePath objects from strings possibly containing backslashes as
   path separator, use \c fromUserInput.

   \sa toString, fromUserInput
 */
FilePath FilePath::fromString(const QString &filepath)
{
    FilePath fn;
    fn.setFromString(filepath);
    return fn;
}

void FilePath::setFromString(const QString &filename)
{
    if (filename.startsWith('/')) {
        m_data = filename;  // fast track: absolute local paths
    } else {
        int pos1 = filename.indexOf("://");
        if (pos1 >= 0) {
            m_scheme = filename.left(pos1);
            pos1 += 3;
            int pos2 = filename.indexOf('/', pos1);
            if (pos2 == -1) {
                m_data = filename.mid(pos1);
            } else {
                m_host = filename.mid(pos1, pos2 - pos1);
                m_host.replace("%2f", "/");
                m_host.replace("%25", "%");
                m_data = filename.mid(pos2);
            }
            if (m_data.startsWith("/./"))
                m_data = m_data.mid(3);
        } else {
            m_data = filename; // treat everything else as local, too.
        }
    }
}

/// Constructs a FilePath from \a filePath. The \a defaultExtension is appended
/// to \a filename if that does not have an extension already.
/// \a filePath is not checked for validity.
FilePath FilePath::fromStringWithExtension(const QString &filepath, const QString &defaultExtension)
{
    if (filepath.isEmpty() || defaultExtension.isEmpty())
        return FilePath::fromString(filepath);

    FilePath rc = FilePath::fromString(filepath);
    // Add extension unless user specified something else
    const QChar dot = QLatin1Char('.');
    if (!rc.fileName().contains(dot)) {
        if (!defaultExtension.startsWith(dot))
            rc = rc.stringAppended(dot);
        rc = rc.stringAppended(defaultExtension);
    }
    return rc;
}

/// Constructs a FilePath from \a filePath
/// \a filePath is only passed through QDir::fromNativeSeparators
FilePath FilePath::fromUserInput(const QString &filePath)
{
    QString clean = doCleanPath(filePath);
    if (clean.startsWith(QLatin1String("~/")))
        return FileUtils::homePath().pathAppended(clean.mid(2));
    return FilePath::fromString(clean);
}

/// Constructs a FilePath from \a filePath, which is encoded as UTF-8.
/// \a filePath is not checked for validity.
FilePath FilePath::fromUtf8(const char *filename, int filenameSize)
{
    return FilePath::fromString(QString::fromUtf8(filename, filenameSize));
}

FilePath FilePath::fromVariant(const QVariant &variant)
{
    if (variant.type() == QVariant::Url)
        return FilePath::fromUrl(variant.toUrl());
    return FilePath::fromString(variant.toString());
}

QVariant FilePath::toVariant() const
{
    return toString();
}

QDir FilePath::toDir() const
{
    return QDir(m_data);
}

bool FilePath::operator==(const FilePath &other) const
{
    return QString::compare(m_data, other.m_data, caseSensitivity()) == 0
        && m_host == other.m_host
        && m_scheme == other.m_scheme;
}

bool FilePath::operator!=(const FilePath &other) const
{
    return !(*this == other);
}

bool FilePath::operator<(const FilePath &other) const
{
    const int cmp = QString::compare(m_data, other.m_data, caseSensitivity());
    if (cmp != 0)
        return cmp < 0;
    if (m_host != other.m_host)
        return m_host < other.m_host;
    return m_scheme < other.m_scheme;
}

bool FilePath::operator<=(const FilePath &other) const
{
    return !(other < *this);
}

bool FilePath::operator>(const FilePath &other) const
{
    return other < *this;
}

bool FilePath::operator>=(const FilePath &other) const
{
    return !(*this < other);
}

FilePath FilePath::operator+(const QString &s) const
{
    FilePath res = *this;
    res.m_data += s;
    return res;
}

/// \returns whether FilePath is a child of \a s
bool FilePath::isChildOf(const FilePath &s) const
{
    if (s.isEmpty())
        return false;
    if (!m_data.startsWith(s.m_data, caseSensitivity()))
        return false;
    if (m_data.size() <= s.m_data.size())
        return false;
    // s is root, '/' was already tested in startsWith
    if (s.m_data.endsWith(QLatin1Char('/')))
        return true;
    // s is a directory, next character should be '/' (/tmpdir is NOT a child of /tmp)
    return m_data.at(s.m_data.size()) == QLatin1Char('/');
}

/// \returns whether FilePath startsWith \a s
bool FilePath::startsWith(const QString &s) const
{
    return m_data.startsWith(s, caseSensitivity());
}

/// \returns whether FilePath endsWith \a s
bool FilePath::endsWith(const QString &s) const
{
    return m_data.endsWith(s, caseSensitivity());
}

/// \returns whether FilePath starts with a drive letter
/// \note defaults to \c false if it is a non-Windows host or represents a path on device
bool FilePath::startsWithDriveLetter() const
{
    if (needsDevice() || !HostOsInfo::isWindowsHost())
        return false;
    return m_data.length() >= 2 && m_data.at(0).isLetter() && m_data.at(1) == ':';
}

/// \returns the relativeChildPath of FilePath to parent if FilePath is a child of parent
/// \note returns a empty FilePath if FilePath is not a child of parent
/// That is, this never returns a path starting with "../"
FilePath FilePath::relativeChildPath(const FilePath &parent) const
{
    FilePath res;
    if (isChildOf(parent))
        res.m_data = m_data.mid(parent.m_data.size() + 1, -1);
    return res;
}

/// \returns the relativePath of FilePath to given \a anchor.
/// Both, FilePath and anchor may be files or directories.
/// Example usage:
///
/// \code
///     FilePath filePath("/foo/b/ar/file.txt");
///     FilePath relativePath = filePath.relativePath("/foo/c");
///     qDebug() << relativePath
/// \endcode
///
/// The debug output will be "../b/ar/file.txt".
///
FilePath FilePath::relativePath(const FilePath &anchor) const
{
    QTC_ASSERT(!needsDevice(), return *this);
    const QFileInfo fileInfo(m_data);
    QString absolutePath;
    QString filename;
    if (fileInfo.isFile()) {
        absolutePath = fileInfo.absolutePath();
        filename = fileInfo.fileName();
    } else if (fileInfo.isDir()) {
        absolutePath = fileInfo.absoluteFilePath();
    } else {
        return {};
    }
    const QFileInfo anchorInfo(anchor.m_data);
    QString absoluteAnchorPath;
    if (anchorInfo.isFile())
        absoluteAnchorPath = anchorInfo.absolutePath();
    else if (anchorInfo.isDir())
        absoluteAnchorPath = anchorInfo.absoluteFilePath();
    else
        return {};
    QString relativeFilePath = calcRelativePath(absolutePath, absoluteAnchorPath);
    if (!filename.isEmpty()) {
        if (relativeFilePath == ".")
            relativeFilePath.clear();
        if (!relativeFilePath.isEmpty())
            relativeFilePath += '/';
        relativeFilePath += filename;
    }
    return FilePath::fromString(relativeFilePath);
}

/// \returns the relativePath of \a absolutePath to given \a absoluteAnchorPath.
/// Both paths must be an absolute path to a directory. Example usage:
///
/// \code
///     qDebug() << FilePath::calcRelativePath("/foo/b/ar", "/foo/c");
/// \endcode
///
/// The debug output will be "../b/ar".
///
/// \see FilePath::relativePath
///
QString FilePath::calcRelativePath(const QString &absolutePath, const QString &absoluteAnchorPath)
{
    if (absolutePath.isEmpty() || absoluteAnchorPath.isEmpty())
        return QString();
    // TODO using split() instead of parsing the strings by char index is slow
    // and needs more memory (but the easiest implementation for now)
    const QStringList splits1 = absolutePath.split('/');
    const QStringList splits2 = absoluteAnchorPath.split('/');
    int i = 0;
    while (i < splits1.count() && i < splits2.count() && splits1.at(i) == splits2.at(i))
        ++i;
    QString relativePath;
    int j = i;
    bool addslash = false;
    while (j < splits2.count()) {
        if (!splits2.at(j).isEmpty()) {
            if (addslash)
                relativePath += '/';
            relativePath += "..";
            addslash = true;
        }
        ++j;
    }
    while (i < splits1.count()) {
        if (!splits1.at(i).isEmpty()) {
            if (addslash)
                relativePath += '/';
            relativePath += splits1.at(i);
            addslash = true;
        }
        ++i;
    }
    if (relativePath.isEmpty())
        return QString(".");
    return relativePath;
}

/*!
    Returns a path corresponding to the current object on the
    same device as \a deviceTemplate.

    Example usage:
    \code
        localDir = FilePath("/tmp/workingdir");
        executable = FilePath::fromUrl("docker://123/bin/ls")
        realDir = localDir.onDevice(executable)
        assert(realDir == FilePath::fromUrl("docker://123/tmp/workingdir"))
    \endcode
*/
FilePath FilePath::onDevice(const FilePath &deviceTemplate) const
{
    const bool sameDevice = m_scheme == deviceTemplate.m_scheme && m_host == deviceTemplate.m_host;
    if (sameDevice)
        return *this;
    // TODO: converting paths between different non local devices is still unsupported
    QTC_CHECK(!needsDevice());
    FilePath res;
    res.m_scheme = deviceTemplate.m_scheme;
    res.m_host = deviceTemplate.m_host;
    res.m_data = m_data;
    res.m_data = res.mapToDevicePath();
    return res;
}

/*!
    Returns a FilePath with local path \a newPath on the same device
    as the current object.

    Example usage:
    \code
        devicePath = FilePath("docker://123/tmp");
        newPath = devicePath.withNewPath("/bin/ls");
        assert(realDir == FilePath::fromUrl("docker://123/bin/ls"))
    \endcode
*/
FilePath FilePath::withNewPath(const QString &newPath) const
{
    FilePath res;
    res.m_data = newPath;
    res.m_host = m_host;
    res.m_scheme = m_scheme;
    return res;
}

/*!
    Searched a binary corresponding to this object in the PATH of
    the device implied by this object's scheme and host.

    Example usage:
    \code
        binary = FilePath::fromUrl("docker://123/./make);
        fullPath = binary.searchInDirectories(binary.deviceEnvironment().path());
        assert(fullPath == FilePath::fromUrl("docker://123/usr/bin/make"))
    \endcode
*/
FilePath FilePath::searchInDirectories(const FilePaths &dirs) const
{
    if (needsDevice()) {
        QTC_ASSERT(s_deviceHooks.searchInPath, return {});
        return s_deviceHooks.searchInPath(*this, dirs);
    }
    return Environment::systemEnvironment().searchInDirectories(path(), dirs);
}

FilePath FilePath::searchInPath(const FilePaths &additionalDirs, PathAmending amending) const
{
    FilePaths directories = deviceEnvironment().path();
    if (!additionalDirs.isEmpty()) {
        if (amending == AppendToPath)
            directories.append(additionalDirs);
        else
            directories = additionalDirs + directories;
    }
    return searchInDirectories(directories);
}

Environment FilePath::deviceEnvironment() const
{
    if (needsDevice()) {
        QTC_ASSERT(s_deviceHooks.environment, return {});
        return s_deviceHooks.environment(*this);
    }
    return Environment::systemEnvironment();
}

QString FilePath::formatFilePaths(const QList<FilePath> &files, const QString &separator)
{
    const QStringList nativeFiles = transform(files, &FilePath::toUserOutput);
    return nativeFiles.join(separator);
}

void FilePath::removeDuplicates(QList<FilePath> &files)
{
    // FIXME: Improve.
    QStringList list = transform<QStringList>(files, &FilePath::toString);
    list.removeDuplicates();
    files = transform(list, &FilePath::fromString);
}

void FilePath::sort(QList<FilePath> &files)
{
    // FIXME: Improve.
    QStringList list = transform<QStringList>(files, &FilePath::toString);
    list.sort();
    files = transform(list, &FilePath::fromString);
}

FilePath FilePath::pathAppended(const QString &path) const
{
    FilePath fn = *this;
    if (path.isEmpty())
        return fn;

    if (fn.m_data.isEmpty()) {
        fn.m_data = path;
        return fn;
    }

    if (fn.m_data.endsWith('/')) {
        if (path.startsWith('/'))
            fn.m_data.append(path.mid(1));
        else
            fn.m_data.append(path);
    } else {
        if (path.startsWith('/'))
            fn.m_data.append(path);
        else
            fn.m_data.append('/').append(path);
    }

    return fn;
}

FilePath FilePath::stringAppended(const QString &str) const
{
    FilePath fn = *this;
    fn.m_data.append(str);
    return fn;
}

QHashValueType FilePath::hash(uint seed) const
{
    if (HostOsInfo::fileNameCaseSensitivity() == Qt::CaseInsensitive)
        return qHash(m_data.toCaseFolded(), seed);
    return qHash(m_data, seed);
}

QDateTime FilePath::lastModified() const
{
    if (needsDevice()) {
        QTC_ASSERT(s_deviceHooks.lastModified, return {});
        return s_deviceHooks.lastModified(*this);
    }
    return toFileInfo().lastModified();
}

QFile::Permissions FilePath::permissions() const
{
    if (needsDevice()) {
        QTC_ASSERT(s_deviceHooks.permissions, return {});
        return s_deviceHooks.permissions(*this);
    }
    return toFileInfo().permissions();
}

bool FilePath::setPermissions(QFile::Permissions permissions) const
{
    if (needsDevice()) {
        QTC_ASSERT(s_deviceHooks.setPermissions, return false);
        return s_deviceHooks.setPermissions(*this, permissions);
    }
    return QFile(m_data).setPermissions(permissions);
}

OsType FilePath::osType() const
{
    if (needsDevice()) {
        QTC_ASSERT(s_deviceHooks.osType, return {});
        return s_deviceHooks.osType(*this);
    }
    return HostOsInfo::hostOs();
}

bool FilePath::removeFile() const
{
    if (needsDevice()) {
        QTC_ASSERT(s_deviceHooks.removeFile, return false);
        return s_deviceHooks.removeFile(*this);
    }
    return QFile::remove(path());
}

/*!
  Removes the directory this filePath refers too and its subdirectories recursively.

  \note The \a error parameter is optional.

  Returns whether the operation succeeded.
*/
bool FilePath::removeRecursively(QString *error) const
{
    if (needsDevice()) {
        QTC_ASSERT(s_deviceHooks.removeRecursively, return false);
        return s_deviceHooks.removeRecursively(*this);
    }
    return removeRecursivelyLocal(*this, error);
}

bool FilePath::copyFile(const FilePath &target) const
{
    if (host() != target.host()) {
        // FIXME: This does not scale.
        const QByteArray ba = fileContents();
        return target.writeFileContents(ba);
    }
    if (needsDevice()) {
        QTC_ASSERT(s_deviceHooks.copyFile, return false);
        return s_deviceHooks.copyFile(*this, target);
    }
    return QFile::copy(path(), target.path());
}

void FilePath::asyncCopyFile(const std::function<void(bool)> &cont, const FilePath &target) const
{
    if (host() != target.host()) {
        asyncFileContents([cont, target](const QByteArray &ba) {
            target.asyncWriteFileContents(cont, ba);
        });
    } else if (needsDevice()) {
        s_deviceHooks.asyncCopyFile(cont, *this, target);
    } else {
        cont(copyFile(target));
    }
}

bool FilePath::renameFile(const FilePath &target) const
{
    if (needsDevice()) {
        QTC_ASSERT(s_deviceHooks.renameFile, return false);
        return s_deviceHooks.renameFile(*this, target);
    }
    return QFile::rename(path(), target.path());
}

qint64 FilePath::fileSize() const
{
    if (needsDevice()) {
        QTC_ASSERT(s_deviceHooks.fileSize, return false);
        return s_deviceHooks.fileSize(*this);
    }
    return QFileInfo(m_data).size();
}

qint64 FilePath::bytesAvailable() const
{
    if (needsDevice()) {
        QTC_ASSERT(s_deviceHooks.bytesAvailable, return false);
        return s_deviceHooks.bytesAvailable(*this);
    }
    return QStorageInfo(m_data).bytesAvailable();
}


QTextStream &operator<<(QTextStream &s, const FilePath &fn)
{
    return s << fn.toString();
}

FileFilter::FileFilter(const QStringList &nameFilters,
                               const QDir::Filters fileFilters,
                               const QDirIterator::IteratorFlags flags)
    : nameFilters(nameFilters),
      fileFilters(fileFilters),
      iteratorFlags(flags)
{
}

} // namespace Utils

std::hash<Utils::FilePath>::result_type
    std::hash<Utils::FilePath>::operator()(const std::hash<Utils::FilePath>::argument_type &fn) const
{
    if (fn.caseSensitivity() == Qt::CaseInsensitive)
        return hash<string>()(fn.toString().toCaseFolded().toStdString());
    return hash<string>()(fn.toString().toStdString());
}
