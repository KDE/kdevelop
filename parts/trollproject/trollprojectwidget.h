/***************************************************************************
 *   Copyright (C) 2001 by Bernd Gehrmann                                  *
 *   bernd@kdevelop.org                                                    *
 *   Copyright (C) 2002 by Jakob Simon-Gaarde                              *
 *   jakob@jsg.dk                                                          *
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
#include <qtoolbutton.h>
#include <kiconloader.h>
#include <klocale.h>
#include <qbutton.h>
#include "filebuffer.h"
#include "projectconfigurationdlg.h"
#include "filepropertydlg.h"

class TrollProjectPart;
class KListView;
class SubprojectItem;
class GroupItem;
class FileItem;
class FileBuffer;


/**
 * Structure that holds a projects configuration
 */
struct ProjectConfiguration
{
  QMakeTemplate   m_template;
  QMakeBuildMode  m_buildMode;
  QMakeWarnings   m_warnings;
  int             m_requirements;
};


/**
 * Base class for all items appearing in ProjectOverview and ProjectDetails.
 */
class ProjectItem : public QListViewItem
{
public:
    enum Type { Subproject, Group, File };

    ProjectItem(Type type, QListView *parent, const QString &text);
    ProjectItem(Type type, ProjectItem *parent, const QString &text);

    QString scopeString;
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
    SubprojectItem(QListView *parent, const QString &text,const QString &scopeString);
    SubprojectItem(SubprojectItem *parent, const QString &text, const QString &scopeString);
    ~SubprojectItem();

    QString subdir;
    QString path;
    QList<GroupItem> groups;
    QList<SubprojectItem> scopes;

    QStringList subdirs;
    QStringList sources;
    QStringList headers;
    QStringList forms;

    ProjectConfiguration configuration;

    FileBuffer m_FileBuffer;
    FileBuffer *m_RootBuffer;
    bool isScope;

private:
    void init();
};


class GroupItem : public ProjectItem
{
public:
    enum GroupType {Sources, Headers, Forms };

    GroupItem(QListView *lv, GroupType type, const QString &text,const QString &scopeString);

    QList<FileItem> files;
    GroupType groupType;
    SubprojectItem *owner;
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

    GroupItem *createGroupItem(GroupItem::GroupType groupType, const QString &name, const QString &scopeName);
    FileItem *createFileItem(const QString &name);

    void updateProjectFile(QListViewItem *item);
    void updateProjectConfiguration(SubprojectItem *item);
    void addFileToCurrentSubProject(GroupItem *titem,QString &filename);
    void emitAddedFile(const QString &name);
    void emitRemovedFile(const QString &name);

private slots:
    void slotOverviewSelectionChanged(QListViewItem *item);
    void slotOverviewContextMenu(KListView *, QListViewItem *item, const QPoint &p);
    void slotDetailsExecuted(QListViewItem *item);
    void slotDetailsContextMenu(KListView *, QListViewItem *item, const QPoint &p);
    void slotBuildProject();
    void slotRebuildProject();
    void slotConfigureProject();
    void slotRunProject();

private:
    void cleanDetailView(SubprojectItem *item);
    void buildProjectDetailTree(SubprojectItem *item, KListView *listviewControl);
    void removeFile(SubprojectItem *spitem, FileItem *fitem);
    void parseScope(SubprojectItem *item,QString scopeString, FileBuffer *buffer);
    void parse(SubprojectItem *item);

    QVBox     *overviewContainer;
    KListView *overview;
    KListView *details;
    QHBox     *projectTools;
    QToolButton *buildButton;
    QToolButton *rebuildButton;
    QToolButton *runButton;
    QToolButton *projectconfButton;
    SubprojectItem *m_shownSubproject;
    TrollProjectPart *m_part;
};

#endif
