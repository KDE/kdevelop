/*
    SPDX-FileCopyrightText: 2015 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef URLINFO_H
#define URLINFO_H

#include <util/texteditorhelpers.h>
#include <KTextEditor/Cursor>

#include <QDataStream>
#include <QDir>
#include <QString>
#include <QUrl>

/**
 * Represents a file to be opened, consisting of its URL and the cursor to jump to.
 */
class UrlInfo
{
public:
    /**
     * Parses a file path argument and determines its line number and column and full path
     * @param path path passed on e.g. command line to parse into an URL
     */
    explicit UrlInfo(QString path = QString())
        : cursor(KTextEditor::Cursor::invalid())
    {
        /**
         * first try: just check if the path is an existing file
         */
        if (QFile::exists(path)) {
            /**
             * create absolute file path, we will e.g. pass this over dbus to other processes
             * and then we are done, no cursor can be detected here!
             */
            url = QUrl::fromLocalFile(QDir::current().absoluteFilePath(path));
            return;
        }

        /**
         * ok, the path as is, is no existing file, now, cut away :xx:yy stuff as cursor
         * this will make test:50 to test with line 50
         */
        int pathLength;
        cursor = KDevelop::KTextEditorHelpers::extractCursor(path, &pathLength);
        if (cursor.isValid()) {
            /**
             * cut away the line/column specification from the path
             */
            path.truncate(pathLength);
        }

        /**
         * construct url:
         *   - make relative paths absolute using the current working directory
         *   - prefer local file, if in doubt!
         */
        url = QUrl::fromUserInput(path, QDir::currentPath(), QUrl::AssumeLocalFile);

        /**
         * in some cases, this will fail, e.g. if you have line/column specs like test.c:10:1
         * => fallback: assume a local file and just convert it to an url
         */
        if (!url.isValid()) {
            /**
             * create absolute file path, we will e.g. pass this over dbus to other processes
             */
            url = QUrl::fromLocalFile(QDir::current().absoluteFilePath(path));
        }
    }

    bool isDirectory() const
    {
        return url.isLocalFile() && QFileInfo(url.toLocalFile()).isDir() && !cursor.isValid();
    }

    /**
     * url computed out of the passed path
     */
    QUrl url;

    /**
     * initial cursor position, if any found inside the path as line/column specification at the end
     */
    KTextEditor::Cursor cursor;
};

Q_DECLARE_TYPEINFO(UrlInfo, Q_MOVABLE_TYPE);

QDataStream& operator<<(QDataStream& stream, const UrlInfo& info)
{
    stream << info.url;
    stream << info.cursor.line();
    stream << info.cursor.column();
    return stream;
}

QDataStream& operator>>(QDataStream& stream, UrlInfo& info)
{
    stream >> info.url;
    int line, column;
    stream >> line;
    stream >> column;
    info.cursor.setLine(line);
    info.cursor.setColumn(column);
    return stream;
}

#endif // URLINFO_H
