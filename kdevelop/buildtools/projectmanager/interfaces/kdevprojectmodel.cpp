/* This file is part of KDevelop
    Copyright (C) 2004 Roberto Raggi <roberto@kdevelop.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/
#include "kdevprojectmodel.h"
#include <kdebug.h>
#include <qfileinfo.h>

QString ProjectModelItem::shortDescription() const
{
    return name();
}

// ---------------------------------------------------------------------------
ProjectFolderList ProjectFolderModel::folderList() const
{
    return m_foldersList;
}

ProjectFolderDom ProjectFolderModel::folderByName(const QString &name) const
{
    return m_folders.contains(name) ? m_folders[name] : ProjectFolderDom();
}

void ProjectFolderModel::addFolder(ProjectFolderDom dom)
{
    m_folders.insert(dom->name(), dom);
    m_foldersList.append(dom);
}

void ProjectFolderModel::removeFolder(ProjectFolderDom dom)
{
    m_folders.remove(dom->name());
    m_foldersList.remove(dom);
}

ProjectFileList ProjectFolderModel::fileList() const
{
    return m_filesList;
}

ProjectFileDom ProjectFolderModel::fileByName(const QString &name) const
{
    return m_files.contains(name) ? m_files[name] : ProjectFileDom();
}

void ProjectFolderModel::addFile(ProjectFileDom dom)
{
    m_files.insert(dom->name(), dom);
    m_filesList.append(dom);
}

void ProjectFolderModel::removeFile(ProjectFileDom dom)
{
    m_files.remove(dom->name());
    m_filesList.remove(dom);
}

ProjectTargetList ProjectFolderModel::targetList() const
{
    return m_targetsList;
}

ProjectTargetDom ProjectFolderModel::targetByName(const QString &name) const
{
    return m_targets.contains(name) ? m_targets[name] : ProjectTargetDom();
}

void ProjectFolderModel::addTarget(ProjectTargetDom dom)
{
    m_targets.insert(dom->name(), dom);
    m_targetsList.append(dom);
}

void ProjectFolderModel::removeTarget(ProjectTargetDom dom)
{
    m_targets.remove(dom->name());
    m_targetsList.remove(dom);
}

// ---------------------------------------------------------------------------
ProjectModel::ProjectModel()
{
}

ProjectModel::~ProjectModel()
{
}

void ProjectModel::wipeout()
{
    m_items.clear();
}

ProjectItemDom ProjectModel::itemByName(const QString &name) const
{
    return m_items.contains(name) ? m_items[name] : ProjectItemDom();
}

void ProjectModel::addItem(ProjectItemDom dom)
{
    if (!dom->toTarget())
        m_items.insert(dom->name(), dom);

    if (ProjectFolderDom folder = dom->toFolder()) {
        ProjectFolderList folder_list = folder->folderList();
        for (ProjectFolderList::Iterator it=folder_list.begin(); it!=folder_list.end(); ++it)
            addItem((*it)->toItem());

        ProjectTargetList target_list = folder->targetList();
        for (ProjectTargetList::Iterator it=target_list.begin(); it!=target_list.end(); ++it)
            addItem((*it)->toItem());

        ProjectFileList file_list = folder->fileList();
        for (ProjectFileList::Iterator it=file_list.begin(); it!=file_list.end(); ++it)
            addItem((*it)->toItem());
    } else if (ProjectTargetDom target = dom->toTarget()) {
        ProjectFileList file_list = target->fileList();
        for (ProjectFileList::Iterator it=file_list.begin(); it!=file_list.end(); ++it)
            addItem((*it)->toItem());
    }
}

void ProjectModel::removeItem(ProjectItemDom dom)
{
    if (!dom->toTarget())
        m_items.remove(dom->name());

    if (ProjectFolderDom folder = dom->toFolder()) {
        ProjectFolderList folder_list = folder->folderList();
        for (ProjectFolderList::Iterator it=folder_list.begin(); it!=folder_list.end(); ++it)
            removeItem((*it)->toItem());

        ProjectTargetList target_list = folder->targetList();
        for (ProjectTargetList::Iterator it=target_list.begin(); it!=target_list.end(); ++it)
            removeItem((*it)->toItem());

        ProjectFileList file_list = folder->fileList();
        for (ProjectFileList::Iterator it=file_list.begin(); it!=file_list.end(); ++it)
            removeItem((*it)->toItem());
    } else if (ProjectTargetDom target = dom->toTarget()) {
        ProjectFileList file_list = target->fileList();
        for (ProjectFileList::Iterator it=file_list.begin(); it!=file_list.end(); ++it)
            removeItem((*it)->toItem());
    }
}

// ---------------------------------------------------------------------------

ProjectFileList ProjectTargetModel::fileList() const
{
    return m_filesList;
}

ProjectFileDom ProjectTargetModel::fileByName(const QString &name) const
{
    return m_files.contains(name) ? m_files[name] : ProjectFileDom();
}

void ProjectTargetModel::addFile(ProjectFileDom dom)
{
    m_files.insert(dom->name(), dom);
    m_filesList.append(dom);
}

void ProjectTargetModel::removeFile(ProjectFileDom dom)
{
    m_files.remove(dom->name());
    m_filesList.append(dom);
}

QString ProjectFileModel::shortDescription() const
{
    QString f = QFileInfo(name()).fileName();
    return f.isEmpty() ? name() : f;
}

QString ProjectFolderModel::shortDescription() const
{
    QString f = QFileInfo(name()).fileName();
    return f.isEmpty() ? name() : f;
}

QString ProjectWorkspaceModel::shortDescription() const
{
    return name();
}

