/*
    SPDX-FileCopyrightText: 2007 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef QMAKEINCLUDEFILE_H
#define QMAKEINCLUDEFILE_H

#include "qmakeprojectfile.h"

class QMakeIncludeFile : public QMakeProjectFile
{
public:
    explicit QMakeIncludeFile( const QString& incfile, QMakeFile* parent,
                               const VariableMap& variables );

    QString pwd() const override;
    QString outPwd() const override;
    QString proFile() const override;
    QString proFilePwd() const override;

private:
    QMakeFile* m_parent;
};

#endif

