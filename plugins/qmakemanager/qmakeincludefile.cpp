/*
    SPDX-FileCopyrightText: 2007 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "qmakeincludefile.h"
#include "qmakemkspecs.h"
#include <QString>

QMakeIncludeFile::QMakeIncludeFile(const QString& incfile, QMakeFile* parent, const VariableMap& variables)
    : QMakeProjectFile(incfile)
    , m_parent(parent)
{
    m_variableValues = variables;

    setProject(parent->project());

    auto* pro = dynamic_cast<QMakeProjectFile*>(parent);
    if (pro) {
        setMkSpecs(pro->mkSpecs());
        setQMakeCache(pro->qmakeCache());
    } else {
        auto* specs = dynamic_cast<QMakeMkSpecs*>(parent);
        setMkSpecs(specs);
    }
}

QString QMakeIncludeFile::pwd() const
{
    return absoluteDir();
}

QString QMakeIncludeFile::outPwd() const
{
    auto* pro = dynamic_cast<QMakeProjectFile*>(m_parent);
    if (pro) {
        return pro->outPwd();
    } else {
        return absoluteDir();
    }
}

QString QMakeIncludeFile::proFile() const
{
    return m_parent->absoluteFile();
}

QString QMakeIncludeFile::proFilePwd() const
{
    return m_parent->absoluteDir();
}
