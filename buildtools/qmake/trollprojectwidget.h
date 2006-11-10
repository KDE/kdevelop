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
#include "choosesubprojectdlg.h"
#include "newwidgetdlg.h"
#include "domutil.h"
#include "qmakescopeitem.h"

class TrollProjectPart;
class KListView;
class ProjectConfigurationDlg;

class TrollProjectWidget : public QVBox
{
    Q_OBJECT

public:
    TrollProjectWidget( TrollProjectPart *part );
    ~TrollProjectWidget();

    void openProject( const QString &dirName );
    void closeProject();

    /**
     * A list of the (relative) names of all subprojects (== subdirectories).
     */
    //QStringList allSubprojects();
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

    void addFileToCurrentSubProject( GroupItem *titem, const QString &filename );
    void addFileToCurrentSubProject( GroupItem::GroupType gtype, const QString &filename );
    void addFiles( QStringList &files, bool relativeToProjectRoot = true );
    void emitAddedFile( const QString &name );
    void emitRemovedFile( const QString &name );

    QString getUiFileLink( const QString &path, const QString& filename );
    bool isTMakeProject();

    enum TrollProjectView { SubprojectView, DetailsView };
    void setLastFocusedView( TrollProjectView view );
    enum SaveType { AlwaysSave, NeverSave, Ask };

public slots:
    void slotBuildTarget();
    void slotRebuildTarget();
    void slotCleanTarget();
    void slotExecuteTarget();

    void slotBuildProject();
    void slotRebuildProject();
    void slotCleanProject();
    void slotExecuteProject();

    void slotBuildOpenFile();
    void slotBuildSelectedFile();

    void slotConfigureProject();
    void slotAddFiles();
    void slotNewFile();
    void slotRemoveFile();

protected:
    virtual void focusInEvent( QFocusEvent *e );

private slots:
    void slotOverviewSelectionChanged( QListViewItem *item );
    void slotOverviewContextMenu( KListView *, QListViewItem *item, const QPoint &p );
    void slotDetailsSelectionChanged( QListViewItem* );
    void slotDetailsExecuted( QListViewItem *item );
    void slotDetailsContextMenu( KListView *, QListViewItem *item, const QPoint &p );
    void slotExcludeFileFromScopeButton();
    void slotAddSubproject( QMakeScopeItem *spitem = 0 );
    void slotRemoveSubproject( QMakeScopeItem *spitem = 0 );
    void slotCreateScope( QMakeScopeItem *spitem = 0 );
    void slotRemoveScope( QMakeScopeItem *spitem = 0 );
    void slotDisableSubproject( QMakeScopeItem* spitem = 0 );
    void slotProjectDirty( const QString& );

    void createQMakeScopeItems();

private:
    void cleanDetailView( QMakeScopeItem *item );
    void buildProjectDetailTree( QMakeScopeItem *item, KListView *listviewControl );
    void removeFile( QMakeScopeItem *spitem, FileItem *fitem );
    void setupContext();
    //     void parseScope(QMakeScopeItem *item,QString scopeString, Scope *scope);
    GroupItem* getInstallRoot( QMakeScopeItem *item );
    GroupItem* getInstallObject( QMakeScopeItem *item, const QString& objectname );
    QString constructMakeCommandLine( Scope* s = 0 );
    void startMakeCommand( const QString &dir, const QString &target );
    void createMakefileIfMissing( const QString &dir, QMakeScopeItem *item );

    void runQMakeRecursive( QMakeScopeItem* proj);
    void buildFile( QMakeScopeItem* spitem, FileItem* fitem);

    /*fileName: full base file name like QFileInfo::baseName ( true )*/
    QPtrList<QMakeScopeItem> findSubprojectForFile( QFileInfo fi );
    void findSubprojectForFile( QPtrList<QMakeScopeItem> &list, QMakeScopeItem * item, QString absFilePath );
    //    QString makeEnvironment();

    TrollProjectWidget::SaveType dialogSaveBehaviour() const;

    QMakeScopeItem *findSubprojectForScope( QMakeScopeItem *scope );

    void reloadProjectFromFile( QMakeScopeItem* item );

    QVBox *overviewContainer;
    KListView *overview;
    QHBox *projectTools;
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

    QVBox *detailContainer;
    KListView *details;
    QHBox *fileTools;
    QToolButton *addfilesButton;
    QToolButton *newfileButton;
    QToolButton *removefileButton;
    QToolButton *excludeFileFromScopeButton;

    DomUtil::PairList m_subclasslist;
    QMakeScopeItem *m_shownSubproject;
    QMakeScopeItem *m_rootSubproject;
    Scope* m_rootScope;
    TrollProjectPart *m_part;
    ProjectConfigurationDlg* m_configDlg;

    TrollProjectView m_lastFocusedView;

    friend class ChooseSubprojectDlg;
    friend class ProjectConfigurationDlg;
};

#endif

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
