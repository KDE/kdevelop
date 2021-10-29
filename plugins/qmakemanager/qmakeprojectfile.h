/*
    SPDX-FileCopyrightText: 2006 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef QMAKEPROJECTFILE_H
#define QMAKEPROJECTFILE_H

#include "qmakefile.h"

class QMakeMkSpecs;

template <typename T> class QList;

class QMakeMkSpecs;
class QMakeCache;

namespace KDevelop {
class IProject;
}

class QMakeProjectFile : public QMakeFile
{
public:
    using DefinePair = QPair<QString, QString>;
    static const QStringList FileVariables;

    explicit QMakeProjectFile( const QString& projectfile );
    ~QMakeProjectFile() override;

    bool read() override;

    QStringList subProjects() const;
    bool hasSubProject(const QString& file) const;

    QStringList files() const;
    QStringList filesForTarget( const QString& ) const;
    QStringList includeDirectories() const;
    QStringList frameworkDirectories() const;
    QStringList extraArguments() const;

    QStringList targets() const;

    QString getTemplate() const;

    void setMkSpecs( QMakeMkSpecs* mkspecs );
    void setOwnMkSpecs(bool own);
    QMakeMkSpecs* mkSpecs() const;
    void setQMakeCache( QMakeCache* cache );
    QMakeCache* qmakeCache() const;
    QStringList resolveVariable(const QString& variable, VariableInfo::VariableType type) const override;
    QList< DefinePair > defines() const;

    /// current pwd, e.g. absoluteDir even for included files
    virtual QString pwd() const;
    /// path to build dir for the current .pro file
    virtual QString outPwd() const;
    /// path to dir of current .pro file
    virtual QString proFilePwd() const;
    /// path to current .pro file
    virtual QString proFile() const;

private:
    void addPathsForVariable(const QString& variable, QStringList* list, const QString& base = {}) const;

    QMakeMkSpecs* m_mkspecs;
    QMakeCache* m_cache;
    static QHash<QString, QHash<QString, QString> > m_qmakeQueryCache;
    QString m_qtIncludeDir;
    QString m_qtVersion;
    // On OS X, QT_INSTALL_LIBS is typically a framework directory and should thus be added to the framework search path
    QString m_qtLibDir;
    bool m_ownMkSpecs = false;
};

#endif

