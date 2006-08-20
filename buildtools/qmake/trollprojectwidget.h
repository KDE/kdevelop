/***************************************************************************
 *   Copyright (C) 2001 by Bernd Gehrmann                                  *
 *   bernd@kdevelop.org                                                    *
 *   Copyright (C) 2002 by Jakob Simon-Gaarde                              *
 *   jakob@jsg.dk                                                          *
 *   Copyright (C) 2002-2003 by Alexander Dymo                             *
 *   cloudtemple@mksat.net                                                 *
 *   Copyright (C) 2003 by Thomas Hasart                                   *
 *   thasart@gmx.de                                                        *
 *   Copyright (C) 2006 by Andreas Pakulat                                 *
 *   apaku@gmx.de                                                          *
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
#include <kdeversion.h>
#include <qbutton.h>
#include <qfileinfo.h>
#include <qptrlist.h>
#include "filebuffer.h"
#include "projectconfigurationdlg.h"
#include "choosesubprojectdlg.h"
#include "filepropertydlg.h"
#include "newwidgetdlg.h"
#include "domutil.h"

class TrollProjectPart;
class KListView;
class SubqmakeprojectItem;
class GroupItem;
class FileItem;
class FileBuffer;


/**
 * Structure that holds a subprojects configuration
 */
struct ProjectConfiguration
{
  QMakeTemplate   m_template;
  QMakeBuildMode  m_buildMode;
  QMakeWarnings   m_warnings;
  int             m_qt4libs;
  int             m_requirements;
  QString         m_destdir;
  QString         m_target;
  QString         m_subdirName;
  QStringList     m_includepath;
  QString         m_libraryversion;
  QStringList     m_librarypath;
  QString         m_objectpath;
  QString         m_uipath;
  QString         m_mocpath;
  QString         m_makefile;
  QStringList     m_defines;
  QStringList     m_cxxflags_debug;
  QStringList     m_cxxflags_release;
  QStringList     m_lflags_debug;
  QStringList     m_lflags_release;
  QStringList     m_libadd;
  QStringList     m_incadd;

  QString         idl_compiler;
  QString         idl_options;
  QStringList     m_prjdeps;
  QStringList     m_incdeps;
  bool            m_inheritconfig;
  bool            m_target_install;
  QString         m_target_install_path;

  QMap<QString,QString> m_variables;
  QStringList    m_removed_variables;
  QStringList    m_projectInclude;

};


/**
 * Base class for all items appearing in ProjectOverview and ProjectDetails.
 */
class qProjectItem : public QListViewItem
{
public:
    enum Type { Subproject, Group, File };

    qProjectItem(Type type, QListView *parent, const QString &text);
    qProjectItem(Type type, qProjectItem *parent, const QString &text);

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
class SubqmakeprojectItem : public qProjectItem
{
public:
    SubqmakeprojectItem(QListView *parent, const QString &text,const QString &scopeString);
    SubqmakeprojectItem(SubqmakeprojectItem *parent, const QString &text, const QString &scopeString);
    ~SubqmakeprojectItem();

    QString subdir;
    QString relpath;
    QString path;
    QString pro_file;
    QPtrList<GroupItem> groups;
    QPtrList<SubqmakeprojectItem> scopes;

    // Containers of included an excluded files
    QStringList subdirs;
    QStringList subdirs_exclude;
    QStringList sources;
    QStringList sources_exclude;
    QStringList headers;
    QStringList headers_exclude;
    QStringList forms;
    QStringList forms_exclude;
    QStringList resources;
    QStringList resources_exclude;

    QStringList distfiles;
    QStringList distfiles_exclude;
    QStringList images;
    QStringList images_exclude;
    QStringList lexsources;
    QStringList lexsources_exclude;
    QStringList yaccsources;
    QStringList yaccsources_exclude;
    QStringList translations;
    QStringList translations_exclude;
    QStringList idls;
    QStringList idls_exclude;





    ProjectConfiguration configuration;

    FileBuffer m_FileBuffer;
    FileBuffer *m_RootBuffer;
    bool isScope;
    QString getRelativPath();
    QString getDownDirs();
    QString getLibAddPath(QString downDirs);
    QString getLibAddObject(QString downDirs);
    QString getSharedLibAddObject(QString downDirs);
    QString getApplicationObject(QString downDirs);
    QString getIncAddPath(QString downDirs);

private:
    void init();
};

class ProjectConfigurationDlg;

class InsideCheckListItem:public QCheckListItem
{
public:
  InsideCheckListItem(QListView *parent,SubqmakeprojectItem *item, ProjectConfigurationDlg *config):
    QCheckListItem(parent, item->relpath.right(item->relpath.length()-1), QCheckListItem::CheckBox)
  {
    prjItem=item;
    m_config = config;
  }
  InsideCheckListItem(QListView *parent,QListViewItem *after,SubqmakeprojectItem *item, ProjectConfigurationDlg *config):
      QCheckListItem(parent,
#if KDE_VERSION > 305
             after,
#endif
             item->relpath.right(item->relpath.length()-1),QCheckListItem::CheckBox)
  {
    prjItem=item;
    m_config = config;
  }
  SubqmakeprojectItem *prjItem;
  ProjectConfigurationDlg *m_config;

protected:
    virtual void stateChange ( bool state );
};



class GroupItem : public qProjectItem
{
public:
    enum GroupType {NoType, Sources, Headers, Forms,Distfiles,Images,Resources,Lexsources,Yaccsources,Translations,IDLs, InstallRoot, InstallObject, MaxTypeEnum };

    static GroupType groupTypeForExtension(const QString &ext);
    static void groupTypeMeanings(GroupItem::GroupType type, QString& title, QString& ext);

    GroupItem(QListView *lv, GroupType type, const QString &text,const QString &scopeString);

    // qmake INSTALLS support
    QString             install_objectname;
    QString             install_path;
    QPtrList<GroupItem> installs;
    QPtrList<FileItem>  files;

    QStringList         str_files;
    QStringList         str_files_exclude;
    // end qmake INSTALLS support
    GroupType           groupType;
    SubqmakeprojectItem*     owner;

protected:
    void paintCell(QPainter* p, const QColorGroup& cg, int column, int width, int align);
};


// Not sure if this complexity is really necessary...
class FileItem : public qProjectItem
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

    SubqmakeprojectItem* getScope(SubqmakeprojectItem *baseItem,const QString &scopeString); // baseItem must be ansister
    void updateProjectFile(QListViewItem *item);
    void updateProjectConfiguration(SubqmakeprojectItem *item);
    void updateInstallObjects(SubqmakeprojectItem* item, FileBuffer *subBuffer);
    void addFileToCurrentSubProject(GroupItem *titem,const QString &filename);
    void addFileToCurrentSubProject(GroupItem::GroupType gtype,const QString &filename);
    void addFiles( QStringList &files, bool noPathTruncate = false);
    void emitAddedFile(const QString &name);
    void emitRemovedFile(const QString &name);

    enum TrollProjectView { SubprojectView, DetailsView };
    void setLastFocusedView(TrollProjectView view);

public slots:
    void slotBuildTarget();
    void slotRebuildTarget();
    void slotCleanTarget();
    void slotExecuteTarget();

    void slotBuildProject();
    void slotRebuildProject();
    void slotCleanProject();
    void slotExecuteProject();

    void slotBuildFile();

    void slotConfigureProject();
    void slotAddFiles();
    void slotNewFile();
    void slotRemoveFile();

protected:
    virtual void focusInEvent(QFocusEvent *e);

private slots:
    void slotOverviewSelectionChanged(QListViewItem *item);
    void slotOverviewContextMenu(KListView *, QListViewItem *item, const QPoint &p);
    void slotDetailsSelectionChanged(QListViewItem*);
    void slotDetailsExecuted(QListViewItem *item);
    void slotDetailsContextMenu(KListView *, QListViewItem *item, const QPoint &p);
    void slotConfigureFile();
    void slotAddSubdir(SubqmakeprojectItem *spitem=0);
    void slotRemoveSubproject(SubqmakeprojectItem *spitem=0);
    void slotCreateScope(SubqmakeprojectItem *spitem=0);
    void slotRemoveScope(SubqmakeprojectItem *spitem=0);

private:
    void cleanDetailView(SubqmakeprojectItem *item);
    void buildProjectDetailTree(SubqmakeprojectItem *item, KListView *listviewControl);
    void removeFile(SubqmakeprojectItem *spitem, FileItem *fitem);
    void setupContext();
    void parseScope(SubqmakeprojectItem *item,QString scopeString, FileBuffer *buffer);
    void parse(SubqmakeprojectItem *item);
    GroupItem* getInstallRoot(SubqmakeprojectItem *item);
    GroupItem* getInstallObject(SubqmakeprojectItem *item,const QString& objectname);
    QString getUiFileLink(const QString &path, const QString& filename);
    QString getHeader();
    QString constructMakeCommandLine(const QString makeFileName = QString::null);
    void startMakeCommand(const QString &dir, const QString &target);
    void createMakefileIfMissing(const QString &dir, SubqmakeprojectItem *item);

    void runQMakeRecursive( SubqmakeprojectItem* proj);

    /*fileName: full base file name like QFileInfo::baseName ( true )*/
    QPtrList<SubqmakeprojectItem> findSubprojectForFile( QFileInfo fi );
    void findSubprojectForFile( QPtrList<SubqmakeprojectItem> &list, SubqmakeprojectItem * item, QString absFilePath );
//    QString makeEnvironment();

    SubqmakeprojectItem *findSubprojectForScope(SubqmakeprojectItem *scope);

    QVBox     *overviewContainer;
    KListView *overview;
    QHBox     *projectTools;
    QToolButton *addSubdirButton;
    QToolButton *createScopeButton;

    QToolButton *buildProjectButton;
    QToolButton *rebuildProjectButton;
    QToolButton *executeProjectButton;

    QToolButton *buildTargetButton;
    QToolButton *rebuildTargetButton;
    QToolButton *executeTargetButton;

    QToolButton *buildFileButton;
    QToolButton *projectconfButton;

    QVBox     *detailContainer;
    KListView *details;
    QHBox     *fileTools;
    QToolButton *addfilesButton;
    QToolButton *newfileButton;
    QToolButton *removefileButton;
    QToolButton *configurefileButton;

    DomUtil::PairList m_subclasslist;
    SubqmakeprojectItem *m_shownSubproject;
    SubqmakeprojectItem *m_rootSubproject;
    TrollProjectPart *m_part;

    TrollProjectView m_lastFocusedView;

    friend class ChooseSubprojectDlg;
};

#endif
