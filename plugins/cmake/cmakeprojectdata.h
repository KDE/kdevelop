/* KDevelop CMake Support
 *
 * Copyright 2013-2017 Aleix Pol <aleixpol@kde.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#ifndef CMAKEPROJECTDATA_H
#define CMAKEPROJECTDATA_H

#include <QSharedPointer>
#include <QStringList>
#include <QHash>
#include <util/path.h>
#include <QDebug>

#include <cmakecommonexport.h>

class CMakeServer;

/**
 * Represents any file in a cmake project that has been added
 * to the project.
 *
 * Contains the required information to compile it properly
 */
struct KDEVCMAKECOMMON_EXPORT CMakeFile
{
    KDevelop::Path::List includes;
    KDevelop::Path::List frameworkDirectories;
    QString compileFlags;
    QString language;
    QHash<QString, QString> defines;

    void addDefine(const QString& define);

    bool isEmpty() const
    {
        return includes.isEmpty() && frameworkDirectories.isEmpty()
            && compileFlags.isEmpty() && defines.isEmpty();
    }
};
Q_DECLARE_TYPEINFO(CMakeFile, Q_MOVABLE_TYPE);

inline QDebug &operator<<(QDebug debug, const CMakeFile& file)
{
    debug << "CMakeFile(-I" << file.includes << ", -F" << file.frameworkDirectories << ", -D" << file.defines << ", " << file.language << ")";
    return debug.maybeSpace();
}

struct KDEVCMAKECOMMON_EXPORT CMakeFilesCompilationData
{
    QHash<KDevelop::Path, CMakeFile> files;
    bool isValid = false;
};

struct KDEVCMAKECOMMON_EXPORT CMakeTarget
{
    Q_GADGET
public:
    enum Type { Library, Executable, Custom };
    Q_ENUM(Type)

    static Type typeToEnum(const QString& target);

    Type type;
    QString name;
    KDevelop::Path::List artifacts;
    KDevelop::Path::List sources;
    // see https://cmake.org/cmake/help/latest/prop_tgt/FOLDER.html
    QString folder;
};
Q_DECLARE_TYPEINFO(CMakeTarget, Q_MOVABLE_TYPE);

inline QDebug &operator<<(QDebug debug, const CMakeTarget& target) {
    debug << target.type << ':' << target.name; return debug.maybeSpace();
}

inline bool operator==(const CMakeTarget& lhs, const CMakeTarget& rhs)
{
    return lhs.type == rhs.type
        && lhs.name == rhs.name
        && lhs.artifacts == rhs.artifacts;
}

struct KDEVCMAKECOMMON_EXPORT CMakeTest
{
    QString name;
    QString executable;
    QStringList arguments;
    QHash<QString, QString> properties;
};
Q_DECLARE_TYPEINFO(CMakeTest, Q_MOVABLE_TYPE);

struct KDEVCMAKECOMMON_EXPORT CMakeProjectData
{
    CMakeFilesCompilationData compilationData;
    QHash<KDevelop::Path, QVector<CMakeTarget>> targets;
    QVector<CMakeTest> testSuites;
};

#endif
