/*
    SPDX-FileCopyrightText: 2007 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "qmakecache.h"

#include "qmakemkspecs.h"

QMakeCache::QMakeCache(const QString& cachefile)
    : QMakeFile(cachefile)
    , m_mkspecs(nullptr)
{
}

void QMakeCache::setMkSpecs(QMakeMkSpecs* specs)
{
    m_mkspecs = specs;
}

bool QMakeCache::read()
{
    foreach (const QString& var, m_mkspecs->variables()) {
        m_variableValues[var] = m_mkspecs->variableValues(var);
    }
    return QMakeFile::read();
}
