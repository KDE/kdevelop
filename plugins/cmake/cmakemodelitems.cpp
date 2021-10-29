/*
    SPDX-FileCopyrightText: 2006 Matt Rogers <mattr@kde.org>
    SPDX-FileCopyrightText: 2007 Aleix Pol <aleixpol@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "cmakemodelitems.h"
#include "cmakeutils.h"

CMakeTargetItem::CMakeTargetItem(KDevelop::ProjectBaseItem* parent, const QString& name, const KDevelop::Path &builtUrl)
    : KDevelop::ProjectExecutableTargetItem(parent->project(), name, parent)
    , m_builtUrl(builtUrl)
{}

QUrl CMakeTargetItem::builtUrl() const
{
    if (!m_builtUrl.isEmpty())
        return m_builtUrl.toUrl();

    const KDevelop::Path buildDir = CMake::currentBuildDir(project());
    if (buildDir.isEmpty())
        return QUrl();

    QString p = project()->path().relativePath(parent()->path());
    return KDevelop::Path(KDevelop::Path(buildDir, p), text()).toUrl();
}

QUrl CMakeTargetItem::installedUrl() const
{
    return QUrl();
}
