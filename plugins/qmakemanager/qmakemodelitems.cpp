/*
    SPDX-FileCopyrightText: 2006 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "qmakemodelitems.h"

#include <QList>

#include "qmakeprojectfile.h"

QMakeFolderItem::QMakeFolderItem(KDevelop::IProject* project, const KDevelop::Path& path,
                                 KDevelop::ProjectBaseItem* parent)
    : KDevelop::ProjectBuildFolderItem(project, path, parent)
{
}

QList<QMakeProjectFile*> QMakeFolderItem::projectFiles() const
{
    return m_projectFiles;
}

void QMakeFolderItem::addProjectFile(QMakeProjectFile* file)
{
    m_projectFiles << file;
}

QMakeFolderItem::~QMakeFolderItem()
{
    qDeleteAll(m_projectFiles);
}

QMakeTargetItem::QMakeTargetItem(QMakeProjectFile* pro, KDevelop::IProject* project, const QString& s,
                                 KDevelop::ProjectBaseItem* parent)
    : KDevelop::ProjectExecutableTargetItem(project, s, parent)
    , m_pro(pro)
{
}

QMakeTargetItem::~QMakeTargetItem()
{
}

QUrl QMakeTargetItem::builtUrl() const
{
    return QUrl::fromLocalFile(m_pro->outPwd().append(QLatin1Char('/') + text()));
}

QUrl QMakeTargetItem::installedUrl() const
{
    return QUrl();
}
