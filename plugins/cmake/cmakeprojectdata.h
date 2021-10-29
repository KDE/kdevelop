/*
    SPDX-FileCopyrightText: 2013-2017 Aleix Pol <aleixpol@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
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
    /// lookup table to quickly find a file path for a given folder path
    /// this greatly speeds up fallback searching for information on untracked files
    /// based on their folder path
    QHash<KDevelop::Path, KDevelop::Path> fileForFolder;
    void rebuildFileForFolderMapping();
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
    struct CMakeFileFlags
    {
        bool isGenerated = false;
        bool isExternal = false;
        bool isCMake = false;
    };
    QHash<KDevelop::Path, CMakeFileFlags> cmakeFiles;
};

#endif
