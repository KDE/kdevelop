/*
    SPDX-FileCopyrightText: 2017 Aleix Pol <aleixpol@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "cmakeprojectdata.h"
#include "cmakeutils.h"

#include <QTextStream>

void CMakeFile::addDefine(const QString& define)
{
    if (define.isEmpty())
        return;
    const int eqIdx = define.indexOf(QLatin1Char('='));
    if (eqIdx < 0) {
        defines[define] = QString();
    } else {
        defines[define.left(eqIdx)] = define.mid(eqIdx + 1);
    }
}

void CMakeFilesCompilationData::rebuildFileForFolderMapping()
{
    fileForFolder.clear();
    // iterate over files and add all direct folders
    for (auto it = files.constBegin(), end = files.constEnd(); it != end; ++it) {
        const auto file = it.key();
        const auto folder = file.parent();
        if (fileForFolder.contains(folder))
            continue;
        fileForFolder.insert(folder, it.key());
    }
    // now also add the parents of these folders
    const auto copy = fileForFolder;
    for (auto it = copy.begin(), end = copy.end(); it != end; ++it) {
        auto folder = it.key();
        while (folder.hasParent()) {
            folder = folder.parent();
            if (fileForFolder.contains(folder)) {
                break;
            }
            fileForFolder.insert(folder, it.key());
        }
    }
}

CMakeTarget::Type CMakeTarget::typeToEnum(const QString& value)
{
    static const QHash<QString, CMakeTarget::Type> s_types = {
        {QStringLiteral("EXECUTABLE"), CMakeTarget::Executable},
        {QStringLiteral("STATIC_LIBRARY"), CMakeTarget::Library},
        {QStringLiteral("MODULE_LIBRARY"), CMakeTarget::Library},
        {QStringLiteral("SHARED_LIBRARY"), CMakeTarget::Library},
        {QStringLiteral("OBJECT_LIBRARY"), CMakeTarget::Library},
        {QStringLiteral("INTERFACE_LIBRARY"), CMakeTarget::Library}
    };
    return s_types.value(value, CMakeTarget::Custom);
}

QDebug operator<<(QDebug debug, PrintLastModified p)
{
    const QDebugStateSaver saver(debug);
    debug.noquote().nospace();
    if (p.whatWasModified) {
        // Align the printed timestamps to facilitate comparison.
        debug << "last modified " << qSetFieldWidth(21) << p.whatWasModified << qSetFieldWidth(0) << ": ";
    }
    debug << p.lastModified.toString(Qt::ISODateWithMs);
    return debug;
}
