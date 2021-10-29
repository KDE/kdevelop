/*
    SPDX-FileCopyrightText: 2007 Andreas Pakulat <pakulat@rostock.zgdv.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "qmakemkspecs.h"

QMakeMkSpecs::QMakeMkSpecs(const QString& basicmkspecs, QHash<QString, QString> variables)
    : QMakeFile(basicmkspecs)
    , m_qmakeInternalVariables(std::move(variables))
{
}

QString QMakeMkSpecs::qmakeInternalVariable(const QString& var) const
{
    return m_qmakeInternalVariables.value(var, QString());
}

bool QMakeMkSpecs::isQMakeInternalVariable(const QString& var) const
{
    return m_qmakeInternalVariables.contains(var);
}
