/*
    SPDX-FileCopyrightText: 2007 Andreas Pakulat <pakulat@rostock.zgdv.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef QMAKEMKSPECS_H
#define QMAKEMKSPECS_H

#include <QHash>
#include <QString>
#include "qmakefile.h"

class QMakeMkSpecs : public QMakeFile
{
public:
    QMakeMkSpecs( const QString& basicmkspec, QHash<QString, QString>  variables );

    QString qmakeInternalVariable( const QString& ) const;
    bool isQMakeInternalVariable( const QString& ) const;

private:
    QHash<QString, QString> m_qmakeInternalVariables;
};

#endif

