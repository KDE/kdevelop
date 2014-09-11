/* KDevelop Custom Makefile Support
 *
 * Copyright 2007 Dukju Ahn <dukjuahn@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
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

QList<QPair<QString, QString> > CustomMakeTargetItem::defines() const
{
    return m_defines;
}
