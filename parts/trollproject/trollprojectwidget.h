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
#include "newwidgetdlg.h"
#include "domutil.h"

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
  QMakeLibMode    m_lib;
  int             m_requirements;
  QString         m_destdir;
  QString         m_target;
  QString         m_subdirName;
  QStringList     m_includepath;
  QString         m_libraryversion;
  QStringList     m_librarypath;
  QStringList     m_defines;
  QStringList     m_cxxflags_debug;
  QStringList     m_cxxflags_release;
  QStringList     m_lflags_debug;
  QStringList     m_lflags_release;
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
    QString relpath;
    QString path;
    QString pro_file;
    QPtrList<GroupItem> groups;
    QPtrList<SubprojectItem> scopes;

    // Containers of included an excluded files
    QStringList subdirs;
    QStringList subdirs_exclude;
    QStringList sources;
    QStringList sources_exclude;
    QStringList headers;
    QStringList headers_exclude;
    QStringList forms;
    QStringList forms_exclude;

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
    enum GroupType {NoType, Sources, Headers, Forms };

    static GroupType groupTypeForExtension(const QString &ext);

    GroupItem(QListView *lv, GroupType type, const QString &text,const QString &scopeString);

    QPtrList<FileItem> files;
    GroupType groupType;
    SubprojectItem *owner;
};


// Not sure if this complexity is really necessary...
class FileItem : public ProjectItem
{
public:
    FileItem(QListView *lv, const QString &text,bool exclude=false);

    QString uiFileLink;
    QString name;
    bool excluded;
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
    /**
     * The directory of the currently active subproject.
     */
    QString getCurrentTarget();

    QString getCurrentDestDir();

    QString getCurrentOutputFilename();

    GroupItem *createGroupItem(GroupItem::GroupType groupType, const QString &name, const QString &scopeName);
    FileItem *createFileItem(const QString &name);

    SubprojectItem* getScope(SubprojectItem *baseItem,const QString &scopeString); // baseItem must be ansister
    void updateProjectFile(QListViewItem *item);
    void updateProjectConfiguration(SubprojectItem *item);
    void addFileToCurrentSubProject(GroupItem *titem,const QString &filename);
    void addFileToCurrentSubProject(GroupItem::GroupType gtype,const QString &filename);
    void addFile(const QString &fileName);
    void emitAddedFile(const QString &name);
    void emitRemovedFile(const QString &name);

private slots:
    void slotOverviewSelectionChanged(QListViewItem *item);
    void slotOverviewContextMenu(KListView *, QListViewItem *item, const QPoint &p);
    void slotDetailsSelectionChanged(QListViewItem*);
    void slotDetailsExecuted(QListViewItem *item);
    void slotDetailsContextMenu(KListView *, QListViewItem *item, const QPoint &p);
    void slotBuildProject();
    void slotRebuildProject();
    void slotConfigureProject();
    void slotRunProject();
    void slotAddFiles();
    void slotNewFile();
    void slotRemoveFile();
    void slotConfigureFile();
    void slotAddSubdir(SubprojectItem *spitem=0);
    void slotCreateScope(SubprojectItem *spitem=0);

private:
    void cleanDetailView(SubprojectItem *item);
    void buildProjectDetailTree(SubprojectItem *item, KListView *listviewControl);
    void removeFile(SubprojectItem *spitem, FileItem *fitem);
    void setupContext();
    void parseScope(SubprojectItem *item,QString scopeString, FileBuffer *buffer);
    void parse(SubprojectItem *item);
    QString getUiFileLink(const QString &path, const QString& filename);
    QString getHeader();

    QVBox     *overviewContainer;
    KListView *overview;
    QHBox     *projectTools;
    QToolButton *addSubdirButton;
    QToolButton *createScopeButton;
    QToolButton *buildButton;
    QToolButton *rebuildButton;
    QToolButton *runButton;
    QToolButton *projectconfButton;

    QVBox     *detailContainer;
    KListView *details;
    QHBox     *fileTools;
    QToolButton *addfilesButton;
    QToolButton *newfileButton;
    QToolButton *removefileButton;
    QToolButton *configurefileButton;

    DomUtil::PairList m_subclasslist;
    SubprojectItem *m_shownSubproject;
    TrollProjectPart *m_part;
};

#endif
