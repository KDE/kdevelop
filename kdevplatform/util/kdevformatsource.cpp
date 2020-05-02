/*
    Copyright 2016 Anton Anikin <anton.anikin@htower.ru>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#include "kdevformatfile.h"

#include <QFileInfo>

int main(int argc, char** argv)
{
    using namespace KDevelop;

    if (argc == 1) {
        qStdOut() << "Usage: " << argv[0] << " ORIGFILE [TMPFILE]\n\n";
        qStdOut() << "Where ORIGFILE represents the original location of the formatted contents,\n";
        qStdOut() << "and TMPFILE is used as the actual, potentially different,\n";
        qStdOut() << "contents of the file.\n";
        return EXIT_FAILURE;
    }

    QFileInfo origFileInfo(QString::fromLocal8Bit(argv[1]));
    if (!origFileInfo.exists()) {
        qStdOut() << "orig file \"" << origFileInfo.absoluteFilePath() << "\" does not exist\n";
        return EXIT_FAILURE;
    }

    QString origFilePath = origFileInfo.canonicalFilePath();
    QString tempFilePath;

    if (argc > 2)
        tempFilePath = QFileInfo(QString::fromLocal8Bit(argv[2])).canonicalFilePath();
    else {
        tempFilePath = origFilePath;
        qStdOut() << "no temp file given, formatting the original file\n";
    }

    KDevFormatFile formatFile(origFilePath, tempFilePath);

    if (!formatFile.find())
        return EXIT_FAILURE;

    if (!formatFile.read())
        return EXIT_FAILURE;

    if (!formatFile.apply())
        return EXIT_FAILURE;

    return EXIT_SUCCESS;
}
