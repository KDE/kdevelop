/**********************************************************************
** Copyright (C) 2000 Trolltech AS.  All rights reserved.
**
** This file is part of Qt Designer.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "metadatabase.h"
#include "../interfaces/actioninterface.h" // for GCC 2.7.* compatibility
#include "../interfaces/editorinterface.h"
#include "../interfaces/templatewizardiface.h"
#include "../interfaces/languageinterface.h"
#include "../interfaces/filterinterface.h"
#include "../interfaces/interpreterinterface.h"
#include "../interfaces/preferenceinterface.h"
#include "../interfaces/projectsettingsiface.h"
#include "../interfaces/sourcetemplateiface.h"
#include "sourceeditor.h"

#include <qaction.h>
#include <q3mainwindow.h>
#include <qmap.h>
#include <qpointer.h>
//Added by qt3to4:
#include <Q3ActionGroup>
#include <QCloseEvent>
#include <Q3PtrList>
#include <QEvent>
#include <Q3ValueList>
#include <Q3PopupMenu>
#include <private/qpluginmanager_p.h>
#include <qobject.h>

class QToolBox;
class PropertyEditor;
class QWorkspace;
class QMenuBar;
class FormWindow;
class Q3PopupMenu;
class HierarchyView;
class QCloseEvent;
class Workspace;
class ActionEditor;
class Project;
class OutputWindow;
class QTimer;
class FindDialog;
struct DesignerProject;
class ReplaceDialog;
class GotoLineDialog;
class SourceFile;
class FormFile;
class QAssistantClient;
class DesignerAction;

#if defined(Q_FULL_TEMPLATE_INSTANTIATION)
#include <q3toolbar.h>
#else
class Q3ToolBar;
#endif
class Preferences;

class KDevDesignerPart;

class MainWindow : public Q3MainWindow
{
    Q_OBJECT

public:
    enum LineMode { Error, Step, StackFrame };

    MainWindow( KDevDesignerPart *part, bool asClient, bool single = FALSE, const QString &plgDir = "/designer" );
    ~MainWindow();

    HierarchyView *objectHierarchy() const;
    Workspace *workspace() const;
    PropertyEditor *propertyeditor() const;
    ActionEditor *actioneditor() const;

    void resetTool();
    int currentTool() const;

    FormWindow *formWindow();

    bool unregisterClient( FormWindow *w );
    void editorClosed( SourceEditor *e );
    QWidget *isAFormWindowChild( QObject *o ) const;
    QWidget *isAToolBarChild( QObject *o ) const;

    void insertFormWindow( FormWindow *fw );
    QWorkspace *qWorkspace() const;

    void popupFormWindowMenu( const QPoint &gp, FormWindow *fw );
    void popupWidgetMenu( const QPoint &gp, FormWindow *fw, QWidget *w );

    Q3PopupMenu *setupNormalHierarchyMenu( QWidget *parent );
    Q3PopupMenu *setupTabWidgetHierarchyMenu( QWidget *parent, const char *addSlot, const char *removeSlot );

    FormWindow *openFormWindow( const QString &fn, bool validFileName = TRUE, FormFile *ff = 0 );
    bool isCustomWidgetUsed( MetaDataBase::CustomWidget *w );

    void setGrid( const QPoint &p );
    void setShowGrid( bool b );
    void setSnapGrid( bool b );
    QPoint grid() const { return grd; }
    bool showGrid() const { return sGrid; }
    bool snapGrid() const { return snGrid && sGrid; }

    QString documentationPath() const;

    static MainWindow *self;

    QString templatePath() const { return templPath; }

    void editFunction( const QString &func, bool rereadSource = FALSE );

    bool isPreviewing() const { return previewing; }

    Project *currProject() const { return currentProject; }

    FormWindow *activeForm() const { return lastActiveFormWindow; }

    TemplateWizardInterface* templateWizardInterface( const QString& className );
    QStringList sourceTemplates() const;
    SourceTemplateInterface* sourceTemplateInterface( const QString& templ );
    QUnknownInterface* designerInterface() const { return desInterface; }
    Q3PtrList<DesignerProject> projectList() const;
    QStringList projectNames() const;
    QStringList projectFileNames() const;
    Project *findProject( const QString &projectName ) const;
    void setCurrentProject( Project *pro );
    void setCurrentProjectByFilename( const QString& proFilename );
    OutputWindow *outputWindow() const { return oWindow; }
    void addPreferencesTab( QWidget *tab, const QString &title, QObject *receiver, const char *init_slot, const char *accept_slot );
    void addProjectTab( QWidget *tab, const QString &title, QObject *receiver, const char *init_slot, const char *accept_slot );
    void setModified( bool b, QWidget *window );
    void functionsChanged();
    void updateFunctionList();
    void updateWorkspace();
    void runProjectPrecondition();
    void runProjectPostcondition( QObjectList *l );

    void formNameChanged( FormWindow *fw );

    int currentLayoutDefaultSpacing() const;
    int currentLayoutDefaultMargin() const;

    void saveAllBreakPoints();
    void resetBreakPoints();

    SourceFile *sourceFile();
    void createNewProject( const QString &lang );

    void popupProjectMenu( const QPoint &pos );
    QObject *findRealObject( QObject *o );

    void setSingleProject( Project *pro );
    bool singleProjectMode() const { return singleProject; }

    void showSourceLine( QObject *o, int line, LineMode lm );

    void shuttingDown();
    void showGUIStuff( bool b );
    void setEditorsReadOnly( bool b );
    bool areEditorsReadOnly() const { return editorsReadOnly; }
    void toggleSignalHandlers( bool show );
    bool showSignalHandlers() const { return sSignalHandlers; }
    void writeConfig();

    void openProject( const QString &fn );
    void setPluginDirectory( const QString &pd );
    QString pluginDirectory() const { return pluginDir; }

    QAssistantClient* assistantClient() const { return assistant; }

    void addRecentlyOpenedFile( const QString &fn ) { addRecentlyOpened( fn, recentlyFiles ); }

    void statusMessage(const QString &msg);
    
    //integration
    KDevDesignerPart *part() const { return m_part; }
    
public slots:
    void showProperties( QObject *w );
    void updateProperties( QObject *w );
    void showDialogHelp();
    void showDebugStep( QObject *o, int line );
    void showStackFrame( QObject *o, int line );
    void showErrorMessage( QObject *o, int line, const QString &errorMessage );
    void finishedRun();
    void breakPointsChanged();

signals:
    void currentToolChanged();
    void hasActiveForm( bool );
    void hasActiveWindow( bool );
    void hasActiveWindowOrProject( bool );
    void hasNonDummyProject( bool );
    void formModified( bool );
    void formWindowsChanged();
    void formWindowChanged();
    void projectChanged();
    void editorChanged();

protected:
    bool eventFilter( QObject *o, QEvent *e );
    void closeEvent( QCloseEvent *e );

public slots:
    void fileNew();
    void fileNewDialog();
    void fileNewFile();
    void fileClose();
    void fileQuit();
    void fileCloseProject(); // not visible in menu, called from fileClose
    void fileOpen();
    void fileOpen( const QString &filter, const QString &extension, const QString &filename = "" , bool inProject = TRUE );
    bool fileSave();
    bool fileSaveForm(); // not visible in menu, called from fileSave
    bool fileSaveProject(); // not visible in menu, called from fileSaveProject
    bool fileSaveAs();
    void fileSaveAll();
    void fileCreateTemplate();

public slots:
    void editUndo();
    void editRedo();
    void editCut();
    void editCopy();
    void editPaste();
    void editDelete();
    void editSelectAll();
    void editLower();
    void editRaise();
    void editAdjustSize();
    void editLayoutHorizontal();
    void editLayoutVertical();
    void editLayoutHorizontalSplit();
    void editLayoutVerticalSplit();
    void editLayoutGrid();
    void editLayoutContainerHorizontal();
    void editLayoutContainerVertical();
    void editLayoutContainerGrid();
    void editBreakLayout();
    void editAccels();
    void editFunctions();
    void editConnections();
    SourceEditor *editSource();
    SourceEditor *editSource( SourceFile *f );
    SourceEditor *openSourceEditor();
    SourceEditor *createSourceEditor( QObject *object, Project *project,
				      const QString &lang = QString::null,
				      const QString &func = QString::null,
				      bool rereadSource = FALSE );
    void editFormSettings();
    void editProjectSettings();
    void editPixmapCollection();
    void editDatabaseConnections();
    void editPreferences();

    void projectInsertFile();

    void searchFind();
    void searchIncremetalFindMenu();
    void searchIncremetalFind();
    void searchIncremetalFindNext();
    void searchReplace();
    void searchGotoLine();

    void previewForm();
    void previewForm( const QString& );

    void toolsCustomWidget();
    void toolsConfigure();

    void helpContents();
    void helpManual();
    void helpAbout();
    void helpAboutQt();
    void helpRegister();

private slots:
    void activeWindowChanged( QWidget *w );
    void updateUndoRedo( bool, bool, const QString &, const QString & );
    void updateEditorUndoRedo();

    void toolSelected( QAction* );

    void clipboardChanged();
    void selectionChanged();

    void windowsMenuActivated( int id );
    void setupWindowActions();

    void createNewTemplate();
    void projectSelected( QAction *a );

    void setupRecentlyFilesMenu();
    void setupRecentlyProjectsMenu();
    void recentlyFilesMenuActivated( int id );
    void recentlyProjectsMenuActivated( int id );

    void emitProjectSignals();

    void showStartDialog();

private:
    void setupMDI();
    void setupMenuBar();
    void setupEditActions();
    void setupProjectActions();
    void setupSearchActions();
    void setupToolActions();
    void setupLayoutActions();
    void setupFileActions();
    void setupPreviewActions();
    void setupHelpActions();
    void setupRMBMenus();

    void setupPropertyEditor();
    void setupHierarchyView();
    void setupWorkspace();
    void setupActionEditor();
    void setupOutputWindow();
    void setupToolbox();

    void setupActionManager();
    void setupPluginManagers();

    void enableAll( bool enable );

    QWidget* previewFormInternal( QStyle* style = 0, QPalette* pal = 0 );

    void readConfig();

    void setupRMBProperties( Q3ValueList<uint> &ids, QMap<QString, int> &props, QWidget *w );
    void handleRMBProperties( int id, QMap<QString, int> &props, QWidget *w );
    void setupRMBSpecialCommands( Q3ValueList<uint> &ids, QMap<QString, int> &commands, QWidget *w );
    void handleRMBSpecialCommands( int id, QMap<QString, int> &commands, QWidget *w );
    void setupRMBSpecialCommands( Q3ValueList<uint> &ids, QMap<QString, int> &commands, FormWindow *w );
    void handleRMBSpecialCommands( int id, QMap<QString, int> &commands, FormWindow *w );
    bool openEditor( QWidget *w, FormWindow *fw );
    void rebuildCustomWidgetGUI();
    void rebuildCommonWidgetsToolBoxPage();
    void checkTempFiles();

    void addRecentlyOpened( const QString &fn, QStringList &lst );

    QString whatsThisFrom( const QString &key );

private slots:
    void doFunctionsChanged();
    bool openProjectSettings( Project *pro );

private:
    struct Tab
    {
	QWidget *w;
	QString title;
	QObject *receiver;
	const char *init_slot, *accept_slot;
	Q_DUMMY_COMPARISON_OPERATOR( Tab )
    };

private:
    PropertyEditor *propertyEditor;
    HierarchyView *hierarchyView;
    Workspace *wspace;
    QWidget *lastPressWidget;
    QWorkspace *qworkspace;
    QMenuBar *menubar;
    QPointer<FormWindow> lastActiveFormWindow;
    bool breakLayout, layoutChilds, layoutSelected;
    QPoint grd;
    bool sGrid, snGrid;
    bool restoreConfig;
    bool backPix;
    bool splashScreen;
    QString fileFilter;

    QMap<QAction*, Project*> projects;
    DesignerAction *actionEditUndo, *actionEditRedo, *actionEditCut, *actionEditCopy,
    *actionEditPaste, *actionEditDelete,
    *actionEditAdjustSize,
    *actionEditHLayout, *actionEditVLayout, *actionEditGridLayout,
    *actionEditSplitHorizontal, *actionEditSplitVertical,
    *actionEditSelectAll, *actionEditBreakLayout, *actionEditFunctions, *actionEditConnections,
    *actionEditLower, *actionEditRaise;
    QAction *actionInsertSpacer;
    Q3ActionGroup *actionGroupTools, *actionGroupProjects;
    QAction* actionPointerTool, *actionConnectTool, *actionOrderTool, *actionBuddyTool;
    QAction* actionCurrentTool;
    DesignerAction *actionHelpContents, *actionHelpAbout, *actionHelpAboutQt, *actionHelpWhatsThis;
    DesignerAction *actionHelpManual;
#if defined(QT_NON_COMMERCIAL)
    DesignerAction *actionHelpRegister;
#endif
    DesignerAction *actionToolsCustomWidget, *actionEditPreferences;
    DesignerAction *actionWindowTile, *actionWindowCascade, *actionWindowClose, *actionWindowCloseAll;
    DesignerAction *actionWindowNext, *actionWindowPrevious;
    DesignerAction *actionEditFormSettings, *actionEditAccels;
    DesignerAction *actionEditSource, *actionNewFile, *actionFileSave, *actionFileExit;
    DesignerAction *actionFileClose, *actionFileSaveAs, *actionFileSaveAll;
    DesignerAction *actionSearchFind, *actionSearchIncremetal, *actionSearchReplace, *actionSearchGotoLine;
    DesignerAction *actionProjectAddFile, *actionEditPixmapCollection, *actionEditDatabaseConnections,
        *actionEditProjectSettings, *actionPreview;
    Q3ActionGroup *actionGroupNew;

    Q3PopupMenu *rmbWidgets;
    Q3PopupMenu *rmbFormWindow;
    Q3PopupMenu *customWidgetMenu, *windowMenu, *fileMenu, *recentlyFilesMenu, *recentlyProjectsMenu;
    Q3PopupMenu *toolsMenu, *layoutMenu, *previewMenu;
    Q3ToolBar *customWidgetToolBar, *layoutToolBar, *projectToolBar, *customWidgetToolBar2, *toolsToolBar;
    Q3ToolBar *commonWidgetsToolBar;
    Q3PtrList<Q3ToolBar> widgetToolBars;

    Preferences *prefDia;
    QMap<QString,QString> propertyDocumentation;
    bool client;
    QString templPath;
    ActionEditor *actionEditor;
    Project *currentProject;
    QPluginManager<ActionInterface> *actionPluginManager;
    QPluginManager<EditorInterface> *editorPluginManager;
    QPluginManager<TemplateWizardInterface> *templateWizardPluginManager;
    QPluginManager<InterpreterInterface> *interpreterPluginManager;
    QPluginManager<PreferenceInterface> *preferencePluginManager;
    QPluginManager<ProjectSettingsInterface> *projectSettingsPluginManager;
    QPluginManager<SourceTemplateInterface> *sourceTemplatePluginManager;
    Q3PtrList<SourceEditor> sourceEditors;
    bool previewing;
    QUnknownInterface *desInterface;
    QStringList recentlyFiles;
    QStringList recentlyProjects;
    OutputWindow *oWindow;
    Q3ValueList<Tab> preferenceTabs;
    Q3ValueList<Tab> projectTabs;
    bool databaseAutoEdit;
    QTimer *updateFunctionsTimer;
    QTimer *autoSaveTimer;
    bool autoSaveEnabled;
    int autoSaveInterval;
    QLineEdit *incrementalSearch;
    QPointer<FindDialog> findDialog;
    QPointer<ReplaceDialog> replaceDialog;
    QPointer<GotoLineDialog> gotoLineDialog;
    Project *eProject;
    bool inDebugMode;
    QObjectList debuggingForms;
    QString lastOpenFilter;
    QPointer<QWidget> previewedForm;
    Q3PopupMenu *projectMenu;
    QString menuHelpFile;
    bool singleProject;
    QToolBox *toolBox;
    int toolsMenuId, toolsMenuIndex;
    uint guiStuffVisible : 1;
    uint editorsReadOnly : 1;
    uint savePluginPaths : 1;
    QAssistantClient *assistant;
    bool shStartDialog;
    QString pluginDir;
    bool sSignalHandlers;
    
    KDevDesignerPart *m_part;

public:
    QString lastSaveFilter;
    Q3PtrList<QAction> toolActions;
    Q3PtrList<QAction> commonWidgetsPage;

friend class KDevDesignerPart;
};

class SenderObject : public QObject
{
    Q_OBJECT

public:
    SenderObject( QUnknownInterface *i ) : iface( i ) { iface->addRef(); }
    ~SenderObject() { iface->release(); }

public slots:
    void emitInitSignal() { emit initSignal( iface ); }
    void emitAcceptSignal() { emit acceptSignal( iface ); }

signals:
    void initSignal( QUnknownInterface * );
    void acceptSignal( QUnknownInterface * );

private:
    QUnknownInterface *iface;

};

#endif
