/***************************************************************************
 *   Copyright (C) 2001 by Bernd Gehrmann                                  *
 *   bernd@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _TROLLPROJECTWIDGET_H_
#define _TROLLPROJECTWIDGET_H_

#include <qdict.h>
#include <qlistview.h>
#include <qmap.h>
#include <qstrlist.h>
#include <qvbox.h>
#include <kiconloader.h>
#include <klocale.h>
#include "filebuffer.h"

class TrollProjectPart;
class KListView;
class SubprojectItem;
class GroupItem;
class FileItem;
class FileBuffer;

/**
 * Base class for all items appearing in ProjectOverview and ProjectDetails.
 */
class ProjectItem : public QListViewItem
{
public:
    enum Type { Subproject, Group, File };

    ProjectItem(Type type, QListView *parent, const QString &text);
    ProjectItem(Type type, ProjectItem *parent, const QString &text);

    Type type()
        { return typ; }

private:
    Type typ;
    void init();
};


/**
 * Stores the content of one .pro file
 */
class SubprojectItem : public ProjectItem
{
public:
    SubprojectItem(QListView *parent, const QString &text);
    SubprojectItem(SubprojectItem *parent, const QString &text);

    QString subdir;
    QString path;
    QList<GroupItem> groups;

    QStringList sources;
    QStringList headers;
    QStringList interfaces;

    FileBuffer m_FileBuffer;

private:
    void init();
};


class GroupItem : public ProjectItem
{
public:
    enum GroupType { Sources, Headers, Interfaces };

    GroupItem(QListView *lv, GroupType type, const QString &text);

    QList<FileItem> files;
    GroupType groupType;
};


// Not sure if this complexity is really necessary...
class FileItem : public ProjectItem
{
public:
    FileItem(QListView *lv, const QString &text);

    QString name;
};


class TrollProjectWidget : public QVBox
{
    Q_OBJECT

public:
    TrollProjectWidget(TrollProjectPart *part);
    ~TrollProjectWidget();

    void openProject(const QString &dirName);
    void closeProject();

    /**
     * A list of the (relative) names of all subprojects (== subdirectories).
     */
    QStringList allSubprojects();
    /**
     * A list of the (relative) names of all libraries.
     */
    QStringList allLibraries();
    /**
     * A list of all files that belong to the project.
     **/
    QStringList allFiles();
    /**
     * The top level directory of the project.
     **/
    QString projectDirectory();
    /**
     * The directory of the currently active subproject.
     */
    QString subprojectDirectory();

    GroupItem *createGroupItem(GroupItem::GroupType groupType, const QString &name);
    FileItem *createFileItem(const QString &name);

    void emitAddedFile(const QString &name);
    void emitRemovedFile(const QString &name);

private slots:
    void slotOverviewSelectionChanged(QListViewItem *item);
    void slotOverviewContextMenu(KListView *, QListViewItem *item, const QPoint &p);
    void slotDetailsExecuted(QListViewItem *item);
    void slotDetailsContextMenu(KListView *, QListViewItem *item, const QPoint &p);

private:
    void removeFile(SubprojectItem *spitem, FileItem *fitem);
    void parse(SubprojectItem *item);

    KListView *overview;
    KListView *details;
    SubprojectItem *m_shownSubproject;
    TrollProjectPart *m_part;
};

#endif
