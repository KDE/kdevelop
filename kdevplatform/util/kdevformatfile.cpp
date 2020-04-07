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
#include "wildcardhelpers.h"

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
    constexpr QChar delimiter = QLatin1Char(':');

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

        const QChar dirSeparator = QDir::separator();
        for (const QString& wildcard : formatLine.wildcards) {
            const QString pattern = QDir::current().canonicalPath() + dirSeparator + wildcard.trimmed();
            if (WildcardHelpers::matchSinglePattern(pattern, m_origFilePath)) {
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
    if (command.isEmpty()) {
        qStdOut() << ", empty command => nothing to do\n";
        return true;
    }
    qStdOut() << ", using command \"" << command << "\"\n";

    command.replace(QLatin1String("$ORIGFILE"), m_origFilePath);
    command.replace(QLatin1String("$TMPFILE"), m_tempFilePath);

#ifdef Q_OS_WIN
    const QString interpreter = QStringLiteral("cmd");
    const QStringList arguments{QStringLiteral("/c"), command};
#else
    const QString interpreter = QStringLiteral("sh");
    const QStringList arguments{QStringLiteral("-c"), command};
#endif
    const int execResult = QProcess::execute(interpreter, arguments);

    if (execResult != 0) {
        const QString interpreterDescription = QLatin1String("interpreter ") + interpreter;
        if (execResult == -2) {
            qStdOut() << "error: " << interpreterDescription << " failed to start\n";
            return false;
        }
        if (execResult == -1) {
            qStdOut() << "error: " << interpreterDescription << " crashed\n";
            return false;
        }
        qStdOut() << "warning: " << interpreterDescription << " exited with code " << execResult << '\n';
    }

    return true;
}

}
