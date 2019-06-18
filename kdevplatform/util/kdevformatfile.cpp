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

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QProcess>

namespace KDevelop {

static const QString formatFileName = QStringLiteral("format_sources");

KDevFormatFile::KDevFormatFile(const QString& origFilePath, const QString& tempFilePath)
    : m_origFilePath(origFilePath)
    , m_tempFilePath(tempFilePath)
{
}

bool KDevFormatFile::find()
{
    QDir srcDir(QFileInfo(m_origFilePath).canonicalPath());

    while (!srcDir.isRoot()) {
        if (srcDir.exists(formatFileName)) {
            QDir::setCurrent(srcDir.canonicalPath());

            qStdOut() << "found \""
                      << QFileInfo(srcDir.canonicalPath() + QDir::separator() + formatFileName).canonicalFilePath()
                      << "\"\n";
            return true;
        }

        srcDir.cdUp();
    }

    return false;
}

bool KDevFormatFile::read()
{
    static const QChar delimiter = QLatin1Char(':');

    QFile formatFile(formatFileName);
    if (!formatFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qStdOut() << "unable to open \"" << formatFileName << "\"\n";
        return false;
    }

    int lineNumber = 0;
    QString line;
    QStringList wildcards;
    QString command;

    while (!formatFile.atEnd()) {
        ++lineNumber;

        line =  QString::fromUtf8(formatFile.readLine().trimmed());
        if (line.isEmpty() || line.startsWith(QLatin1Char('#')))
            continue;

        if (line.indexOf(delimiter) < 0) {
            // We found the simple syntax without wildcards, and only with the command

            wildcards.clear();
            m_formatLines.append({wildcards, line});
        } else {
            // We found the correct syntax with "wildcards : command"

            wildcards = line.section(delimiter, 0, 0).split(QLatin1Char(' '), QString::SkipEmptyParts);
            command = line.section(delimiter, 1).trimmed();

            if (wildcards.isEmpty()) {
                qStdOut() << formatFileName << ":" << lineNumber
                          << ": error: empty wildcard, skip the line\n";
                continue;
            }
            m_formatLines.append({wildcards, command});
        }
    }

    if (m_formatLines.isEmpty()) {
        qStdOut() << formatFileName << ": error: no commands are found\n";
        return false;
    }

    return true;
}

bool KDevFormatFile::apply()
{
    for (const KDevFormatLine& formatLine : qAsConst(m_formatLines)) {
        if (formatLine.wildcards.isEmpty()) {
            qStdOut() << "matched \"" << m_origFilePath << "\" without wildcard";
            return executeCommand(formatLine.command);
        }

        for (const QString& wildcard : formatLine.wildcards) {
            if (QDir::match(QDir::current().canonicalPath() + QDir::separator() + wildcard.trimmed(), m_origFilePath)) {
                qStdOut() << "matched \"" << m_origFilePath << "\" with wildcard \"" << wildcard << '\"';
                return executeCommand(formatLine.command);
            }
        }
    }

    qStdOut() << formatFileName << ": error: no commands applicable to \"" << m_origFilePath << "\"\n";
    return false;
}

bool KDevFormatFile::executeCommand(QString command)
{
    qStdOut() << ", using command \"" << command << "\"\n";

    command.replace(QStringLiteral("$ORIGFILE"), m_origFilePath);
    command.replace(QStringLiteral("$TMPFILE"), m_tempFilePath);

#ifdef Q_OS_WIN
    int execResult = QProcess::execute(QStringLiteral("cmd"), {QStringLiteral("/c"), command});
#else
    int execResult = QProcess::execute(QStringLiteral("sh"), {QStringLiteral("-c"), command});
#endif

    if (execResult == -2) {
        qStdOut() << "command \"" << command << "\" failed to start\n";
        return false;
    }

    if (execResult == -1) {
        qStdOut() << "command \"" << command << "\" crashed\n";
        return false;
    }

    return true;
}

}
