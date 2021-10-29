/*
    SPDX-FileCopyrightText: 2016 Anton Anikin <anton.anikin@htower.ru>

    SPDX-License-Identifier: GPL-2.0-or-later
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
