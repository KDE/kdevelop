/*
    SPDX-FileCopyrightText: 2007 Dukju Ahn <dukjuahn@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "custommakemodelitems.h"

#include <interfaces/iproject.h>

CustomMakeTargetItem::CustomMakeTargetItem( KDevelop::IProject *project, const QString &name, ProjectBaseItem*parent )
    : KDevelop::ProjectTargetItem( project, name, parent )
{
}

QList<QUrl> CustomMakeTargetItem::includeDirectories() const
{
    return m_includeDirs;
}

QHash<QString, QString> CustomMakeTargetItem::environment() const
{
    return m_envs;
}

QVector<QPair<QString, QString>> CustomMakeTargetItem::defines() const
{
    return m_defines;
}
