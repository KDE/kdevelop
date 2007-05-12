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
#include <QHash>

CustomMakeTargetItem::CustomMakeTargetItem( KDevelop::IProject *project, const QString &name, QStandardItem *parent )
    : KDevelop::ProjectTargetItem( project, name, parent )
{
}

const KUrl::List& CustomMakeTargetItem::includeDirectories() const
{
    return m_includeDirs;
}

const QHash<QString, QString>& CustomMakeTargetItem::environment() const
{
    return m_envs;
}

const QList<QPair<QString, QString> >& CustomMakeTargetItem::defines() const
{
    return m_defines;
}

//kate: space-indent on; indent-width 4; replace-tabs on; auto-insert-doxygen on; indent-mode cstyle;
