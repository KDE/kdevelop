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
// #include <QFileSystemWatcher>
#include "custommaketreesynchronizer.h"
#include "iproject.h"
#include "custommakemanager.h"

CustomMakeTargetItem::CustomMakeTargetItem( KDevelop::IProject *project, const QString &name, QStandardItem *parent )
    : KDevelop::ProjectTargetItem( project, name, parent )
{
}

KUrl::List CustomMakeTargetItem::includeDirectories() const
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

///////////////////////////////////////////////////////////////

CustomMakeFolderItem::CustomMakeFolderItem( CustomMakeManager* manager, KDevelop::IProject* project, const KUrl& url, QStandardItem *parent )
    : KDevelop::ProjectFolderItem( project, url, parent )
{
    m_watcher = new CustomMakeTreeSynchronizer( manager );
}

CustomMakeFolderItem::~CustomMakeFolderItem()
{
    delete m_watcher;
}

CustomMakeTreeSynchronizer* CustomMakeFolderItem::fsWatcher()
{
    return m_watcher;
}

