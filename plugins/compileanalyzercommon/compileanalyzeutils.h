/*
    SPDX-FileCopyrightText: 2018, 2020 Friedrich W. H. Kossebau <kossebau@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef COMPILEANALYZER_COMPILEANALYZEUTILS_H
#define COMPILEANALYZER_COMPILEANALYZEUTILS_H

// lib
#include <compileanalyzercommonexport.h>

class QUrl;
class QString;
#include <QStringList>

namespace KDevelop
{
class Path;

namespace Utils
{

KDEVCOMPILEANALYZERCOMMON_EXPORT
QString findExecutable(const QString& fallbackExecutablePath);

KDEVCOMPILEANALYZERCOMMON_EXPORT
QStringList filesFromCompilationDatabase(const KDevelop::Path& buildPath,
                                         const QUrl& urlToCheck, bool allFiles,
                                         QString& error);

}

}

#endif
