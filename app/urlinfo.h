/* This file is part of the KDE project
   Copyright (C) 2015 Milian Wolff <mail@milianw.de>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) version 3, or any
   later version accepted by the membership of KDE e.V. (or its
   successor approved by the membership of KDE e.V.), which shall
   act as a proxy defined in Section 6 of version 3 of the license.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library.  If not, see <http://www.gnu.org/licenses/>.
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
