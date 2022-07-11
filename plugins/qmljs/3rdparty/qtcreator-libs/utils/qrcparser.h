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

#pragma once

#include "utils_global.h"
#include "utils/filepath.h"

#include <QSharedPointer>
#include <QString>
#include <QStringList>

QT_BEGIN_NAMESPACE
class QLocale;
template <typename K, typename V>
class QMap;
QT_END_NAMESPACE

namespace Utils {

namespace Internal {
class QrcParserPrivate;
class QrcCachePrivate;
}

class QTCREATOR_UTILS_EXPORT QrcParser
{
public:
    struct MatchResult
    {
        int matchDepth = {};
        QStringList reversedPaths;
        QList<Utils::FilePath> sourceFiles;
    };

    typedef QSharedPointer<QrcParser> Ptr;
    typedef QSharedPointer<const QrcParser> ConstPtr;
    ~QrcParser();
    bool parseFile(const QString &path, const QString &contents);
    QString firstFileAtPath(const QString &path, const QLocale &locale) const;
    void collectFilesAtPath(const QString &path, QStringList *res, const QLocale *locale = nullptr) const;
    MatchResult longestReverseMatches(const QString &) const;
    bool hasDirAtPath(const QString &path, const QLocale *locale = nullptr) const;
    void collectFilesInPath(const QString &path, QMap<QString, QStringList> *res, bool addDirs = false,
                            const QLocale *locale = nullptr) const;
    void collectResourceFilesForSourceFile(const QString &sourceFile, QStringList *results,
                                           const QLocale *locale = nullptr) const;

    QStringList errorMessages() const;
    QStringList languages() const;
    bool isValid() const;

    static Ptr parseQrcFile(const QString &path, const QString &contents);
    static QString normalizedQrcFilePath(const QString &path);
    static QString normalizedQrcDirectoryPath(const QString &path);
    static QString qrcDirectoryPathForQrcFilePath(const QString &file);
private:
    QrcParser();
    QrcParser(const QrcParser &);
    Internal::QrcParserPrivate *d;
};

class QTCREATOR_UTILS_EXPORT QrcCache
{
public:
    QrcCache();
    ~QrcCache();
    QrcParser::ConstPtr addPath(const QString &path, const QString &contents);
    void removePath(const QString &path);
    QrcParser::ConstPtr updatePath(const QString &path, const QString &contents);
    QrcParser::ConstPtr parsedPath(const QString &path);
    void clear();
private:
    Internal::QrcCachePrivate *d;
};
}
