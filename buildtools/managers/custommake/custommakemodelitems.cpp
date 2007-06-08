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

///////////////////////////////////////////////////////////////

CustomMakeProjectItem::CustomMakeProjectItem( KDevelop::IProject* project, const QString &name, QStandardItem *parent )
    : KDevelop::ProjectItem( project, name, parent )
{
    KDevelop::IProjectFileManager *fileMan = project->fileManager();
    CustomMakeManager *manager = dynamic_cast<CustomMakeManager*>(fileMan);
    m_watcher = new CustomMakeTreeSynchronizer( manager );
}

CustomMakeProjectItem::~CustomMakeProjectItem()
{
    delete m_watcher;
}

CustomMakeTreeSynchronizer* CustomMakeProjectItem::fsWatcher()
{
    return m_watcher;
}

//kate: space-indent on; indent-width 4; replace-tabs on; auto-insert-doxygen on; indent-mode cstyle;
