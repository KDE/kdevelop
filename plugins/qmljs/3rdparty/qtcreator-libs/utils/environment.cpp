/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
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

#include "environment.h"

#include "algorithm.h"
#include "fileutils.h"
#include "qtcassert.h"

#include <QDir>
#include <QProcessEnvironment>
#include <QSet>

namespace Utils {

Q_GLOBAL_STATIC_WITH_ARGS(Environment, staticSystemEnvironment,
                          (QProcessEnvironment::systemEnvironment().toStringList()))

Q_GLOBAL_STATIC(QVector<EnvironmentProvider>, environmentProviders)

NameValueItems Environment::diff(const Environment &other, bool checkAppendPrepend) const
{
    return m_dict.diff(other.m_dict, checkAppendPrepend);
}

int Environment::isValid() const
{
    return m_dict.size() != 0;
}

QProcessEnvironment Environment::toProcessEnvironment() const
{
    QProcessEnvironment result;
    for (auto it = m_dict.m_values.constBegin(); it != m_dict.m_values.constEnd(); ++it) {
        if (it.value().second)
            result.insert(it.key().name, expandedValueForKey(key(it)));
    }
    return result;
}

void Environment::appendOrSetPath(const FilePath &value)
{
    QTC_CHECK(value.osType() == osType());
    if (value.isEmpty())
        return;
    appendOrSet("PATH", value.nativePath(),
                QString(OsSpecificAspects::pathListSeparator(osType())));
}

void Environment::prependOrSetPath(const FilePath &value)
{
    QTC_CHECK(value.osType() == osType());
    if (value.isEmpty())
        return;
    prependOrSet("PATH", value.nativePath(),
                 QString(OsSpecificAspects::pathListSeparator(osType())));
}

void Environment::appendOrSet(const QString &key, const QString &value, const QString &sep)
{
    QTC_ASSERT(!key.contains('='), return );
    const auto it = m_dict.findKey(key);
    if (it == m_dict.m_values.end()) {
        m_dict.m_values.insert(DictKey(key, m_dict.nameCaseSensitivity()), qMakePair(value, true));
    } else {
        // Append unless it is already there
        const QString toAppend = sep + value;
        if (!it.value().first.endsWith(toAppend))
            it.value().first.append(toAppend);
    }
}

void Environment::prependOrSet(const QString &key, const QString &value, const QString &sep)
{
    QTC_ASSERT(!key.contains('='), return );
    const auto it = m_dict.findKey(key);
    if (it == m_dict.m_values.end()) {
        m_dict.m_values.insert(DictKey(key, m_dict.nameCaseSensitivity()), qMakePair(value, true));
    } else {
        // Prepend unless it is already there
        const QString toPrepend = value + sep;
        if (!it.value().first.startsWith(toPrepend))
            it.value().first.prepend(toPrepend);
    }
}

void Environment::prependOrSetLibrarySearchPath(const FilePath &value)
{
    QTC_CHECK(value.osType() == osType());
    switch (osType()) {
    case OsTypeWindows: {
        const QChar sep = ';';
        prependOrSet("PATH", value.nativePath(), QString(sep));
        break;
    }
    case OsTypeMac: {
        const QString sep =  ":";
        const QString nativeValue = value.nativePath();
        prependOrSet("DYLD_LIBRARY_PATH", nativeValue, sep);
        prependOrSet("DYLD_FRAMEWORK_PATH", nativeValue, sep);
        break;
    }
    case OsTypeLinux:
    case OsTypeOtherUnix: {
        const QChar sep = ':';
        prependOrSet("LD_LIBRARY_PATH", value.nativePath(), QString(sep));
        break;
    }
    default:
        break;
    }
}

void Environment::prependOrSetLibrarySearchPaths(const FilePaths &values)
{
    reverseForeach(values, [this](const FilePath &value) {
        prependOrSetLibrarySearchPath(value);
    });
}

Environment Environment::systemEnvironment()
{
    return *staticSystemEnvironment();
}

void Environment::setupEnglishOutput()
{
    m_dict.set("LC_MESSAGES", "en_US.utf8");
    m_dict.set("LANGUAGE", "en_US:en");
}

static FilePath searchInDirectory(const QStringList &execs,
                                  const FilePath &directory,
                                  QSet<FilePath> &alreadyChecked)
{
    const int checkedCount = alreadyChecked.count();
    alreadyChecked.insert(directory);

    if (directory.isEmpty() || alreadyChecked.count() == checkedCount)
        return FilePath();

    const QString dir = directory.toString();

    QFileInfo fi;
    for (const QString &exec : execs) {
        fi.setFile(dir, exec);
        if (fi.isFile() && fi.isExecutable())
            return FilePath::fromString(fi.absoluteFilePath());
    }
    return FilePath();
}

QStringList Environment::appendExeExtensions(const QString &executable) const
{
    QStringList execs(executable);
    const QFileInfo fi(executable);
    if (osType() == OsTypeWindows) {
        // Check all the executable extensions on windows:
        // PATHEXT is only used if the executable has no extension
        if (fi.suffix().isEmpty()) {
            const QStringList extensions = expandedValueForKey("PATHEXT").split(';');

            for (const QString &ext : extensions)
                execs << executable + ext.toLower();
        }
    }
    return execs;
}

bool Environment::isSameExecutable(const QString &exe1, const QString &exe2) const
{
    const QStringList exe1List = appendExeExtensions(exe1);
    const QStringList exe2List = appendExeExtensions(exe2);
    for (const QString &i1 : exe1List) {
        for (const QString &i2 : exe2List) {
            const FilePath f1 = FilePath::fromString(i1);
            const FilePath f2 = FilePath::fromString(i2);
            if (f1 == f2)
                return true;
            if (f1.needsDevice() != f2.needsDevice() || f1.scheme() != f2.scheme())
                return false;
            if (f1.resolveSymlinks() == f2.resolveSymlinks())
                return true;
            if (FileUtils::fileId(f1) == FileUtils::fileId(f2))
                return true;
        }
    }
    return false;
}

QString Environment::expandedValueForKey(const QString &key) const
{
    return expandVariables(m_dict.value(key));
}

static FilePath searchInDirectoriesHelper(const Environment &env,
                                          const QString &executable,
                                          const FilePaths &dirs,
                                          const Environment::PathFilter &func,
                                          bool usePath)
{
    if (executable.isEmpty())
        return FilePath();

    const QString exec = QDir::cleanPath(env.expandVariables(executable));
    const QFileInfo fi(exec);

    const QStringList execs = env.appendExeExtensions(exec);

    if (fi.isAbsolute()) {
        for (const QString &path : execs) {
            QFileInfo pfi = QFileInfo(path);
            if (pfi.isFile() && pfi.isExecutable())
                return FilePath::fromString(path);
        }
        return FilePath::fromString(exec);
    }

    QSet<FilePath> alreadyChecked;
    for (const FilePath &dir : dirs) {
        FilePath tmp = searchInDirectory(execs, dir, alreadyChecked);
        if (!tmp.isEmpty() && (!func || func(tmp)))
            return tmp;
    }

    if (usePath) {
        if (executable.contains('/'))
            return FilePath();

        for (const FilePath &p : env.path()) {
            FilePath tmp = searchInDirectory(execs, p, alreadyChecked);
            if (!tmp.isEmpty() && (!func || func(tmp)))
                return tmp;
        }
    }
    return FilePath();
}

FilePath Environment::searchInDirectories(const QString &executable,
                                          const FilePaths &dirs) const
{
    return searchInDirectoriesHelper(*this, executable, dirs, {}, false);
}

FilePath Environment::searchInPath(const QString &executable,
                                   const FilePaths &additionalDirs,
                                   const PathFilter &func) const
{
    return searchInDirectoriesHelper(*this, executable, additionalDirs, func, true);
}

FilePaths Environment::findAllInPath(const QString &executable,
                                        const FilePaths &additionalDirs,
                                        const Environment::PathFilter &func) const
{
    if (executable.isEmpty())
        return {};

    const QString exec = QDir::cleanPath(expandVariables(executable));
    const QFileInfo fi(exec);

    const QStringList execs = appendExeExtensions(exec);

    if (fi.isAbsolute()) {
        for (const QString &path : execs) {
            QFileInfo pfi = QFileInfo(path);
            if (pfi.isFile() && pfi.isExecutable())
                return {FilePath::fromString(path)};
        }
        return {FilePath::fromString(exec)};
    }

    QSet<FilePath> result;
    QSet<FilePath> alreadyChecked;
    for (const FilePath &dir : additionalDirs) {
        FilePath tmp = searchInDirectory(execs, dir, alreadyChecked);
        if (!tmp.isEmpty() && (!func || func(tmp)))
            result << tmp;
    }

    if (!executable.contains('/')) {
        for (const FilePath &p : path()) {
            FilePath tmp = searchInDirectory(execs, p, alreadyChecked);
            if (!tmp.isEmpty() && (!func || func(tmp)))
                result << tmp;
        }
    }
    return result.values();
}

FilePaths Environment::path() const
{
    return pathListValue("PATH");
}

FilePaths Environment::pathListValue(const QString &varName) const
{
    const QStringList pathComponents = expandedValueForKey(varName).split(
        OsSpecificAspects::pathListSeparator(osType()), Qt::SkipEmptyParts);
    return transform(pathComponents, &FilePath::fromUserInput);
}

void Environment::modifySystemEnvironment(const EnvironmentItems &list)
{
    staticSystemEnvironment->modify(list);
}

void Environment::setSystemEnvironment(const Environment &environment)
{
    *staticSystemEnvironment = environment;
}

/** Expand environment variables in a string.
 *
 * Environment variables are accepted in the following forms:
 * $SOMEVAR, ${SOMEVAR} on Unix and %SOMEVAR% on Windows.
 * No escapes and quoting are supported.
 * If a variable is not found, it is not substituted.
 */
QString Environment::expandVariables(const QString &input) const
{
    QString result = input;

    if (osType() == OsTypeWindows) {
        for (int vStart = -1, i = 0; i < result.length(); ) {
            if (result.at(i++) == '%') {
                if (vStart > 0) {
                    const auto it = m_dict.findKey(result.mid(vStart, i - vStart - 1));
                    if (it != m_dict.m_values.constEnd()) {
                        result.replace(vStart - 1, i - vStart + 1, it->first);
                        i = vStart - 1 + it->first.length();
                        vStart = -1;
                    } else {
                        vStart = i;
                    }
                } else {
                    vStart = i;
                }
            }
        }
    } else {
        enum { BASE, OPTIONALVARIABLEBRACE, VARIABLE, BRACEDVARIABLE } state = BASE;
        int vStart = -1;

        for (int i = 0; i < result.length();) {
            QChar c = result.at(i++);
            if (state == BASE) {
                if (c == '$')
                    state = OPTIONALVARIABLEBRACE;
            } else if (state == OPTIONALVARIABLEBRACE) {
                if (c == '{') {
                    state = BRACEDVARIABLE;
                    vStart = i;
                } else if (c.isLetterOrNumber() || c == '_') {
                    state = VARIABLE;
                    vStart = i - 1;
                } else {
                    state = BASE;
                }
            } else if (state == BRACEDVARIABLE) {
                if (c == '}') {
                    const_iterator it = constFind(result.mid(vStart, i - 1 - vStart));
                    if (it != constEnd()) {
                        result.replace(vStart - 2, i - vStart + 2, it->first);
                        i = vStart - 2 + it->first.length();
                    }
                    state = BASE;
                }
            } else if (state == VARIABLE) {
                if (!c.isLetterOrNumber() && c != '_') {
                    const_iterator it = constFind(result.mid(vStart, i - vStart - 1));
                    if (it != constEnd()) {
                        result.replace(vStart - 1, i - vStart, it->first);
                        i = vStart - 1 + it->first.length();
                    }
                    state = BASE;
                }
            }
        }
        if (state == VARIABLE) {
            const_iterator it = constFind(result.mid(vStart));
            if (it != constEnd())
                result.replace(vStart - 1, result.length() - vStart + 1, it->first);
        }
    }
    return result;
}

FilePath Environment::expandVariables(const FilePath &variables) const
{
    return FilePath::fromString(expandVariables(variables.toString()));
}

QStringList Environment::expandVariables(const QStringList &variables) const
{
    return transform(variables, [this](const QString &i) { return expandVariables(i); });
}

void EnvironmentProvider::addProvider(EnvironmentProvider &&provider)
{
    environmentProviders->append(std::move(provider));
}

const QVector<EnvironmentProvider> EnvironmentProvider::providers()
{
    return *environmentProviders;
}

optional<EnvironmentProvider> EnvironmentProvider::provider(const QByteArray &id)
{
    const int index = indexOf(*environmentProviders, equal(&EnvironmentProvider::id, id));
    if (index >= 0)
        return make_optional(environmentProviders->at(index));
    return nullopt;
}

void EnvironmentChange::addSetValue(const QString &key, const QString &value)
{
    m_changeItems.append({Item::SetValue, QVariant::fromValue(QPair<QString, QString>(key, value))});
}

void EnvironmentChange::addUnsetValue(const QString &key)
{
    m_changeItems.append({Item::UnsetValue, key});
}

void EnvironmentChange::addPrependToPath(const FilePaths &values)
{
    for (int i = values.size(); --i >= 0; ) {
        const FilePath value = values.at(i);
        m_changeItems.append({Item::PrependToPath, value.toVariant()});
    }
}

void EnvironmentChange::addAppendToPath(const FilePaths &values)
{
    for (const FilePath &value : values)
        m_changeItems.append({Item::AppendToPath, value.toVariant()});
}

EnvironmentChange EnvironmentChange::fromFixedEnvironment(const Environment &fixedEnv)
{
    EnvironmentChange change;
    change.m_changeItems.append({Item::SetFixedEnvironment, QVariant::fromValue(fixedEnv)});
    return change;
}

void EnvironmentChange::applyToEnvironment(Environment &env) const
{
    for (const Item &item : m_changeItems) {
        switch (item.type) {
        case Item::SetSystemEnvironment:
            env = Environment::systemEnvironment();
            break;
        case Item::SetFixedEnvironment:
            env = item.data.value<Environment>();
            break;
        case Item::SetValue: {
            auto data = item.data.value<QPair<QString, QString>>();
            env.set(data.first, data.second);
            break;
        }
        case Item::UnsetValue:
            env.unset(item.data.toString());
            break;
        case Item::PrependToPath:
            env.prependOrSetPath(FilePath::fromVariant(item.data));
            break;
        case Item::AppendToPath:
            env.appendOrSetPath(FilePath::fromVariant(item.data));
            break;
        }
    }
}

} // namespace Utils
