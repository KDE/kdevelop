 /**********************************************************************
** Copyright (C) 2000-2001 Trolltech AS.  All rights reserved.
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

#ifndef DESIGNERAPPIFACE_H
#define DESIGNERAPPIFACE_H

#include "../interfaces/designerinterface.h"
#include "project.h"
//Added by qt3to4:
#include <QPixmap>
#include <Q3CString>
#include <Q3PtrList>
#include <Q3ValueList>

class FormWindow;
class MainWindow;
class Project;
class OutputWindow;
class SourceFile;

class DesignerInterfaceImpl : public DesignerInterface
{
public:
    DesignerInterfaceImpl( MainWindow *mw );

    DesignerProject *currentProject() const;
    DesignerFormWindow *currentForm() const;
    DesignerSourceFile *currentSourceFile() const;
    Q3PtrList<DesignerProject> projectList() const;
    void showStatusMessage( const QString &, int ms = 0 ) const;
    DesignerDock *createDock() const;
    DesignerOutputDock *outputDock() const;
    void setModified( bool b, QWidget *window );
    void updateFunctionList();

    void onProjectChange( QObject *receiver, const char *slot );
    void onFormChange( QObject *receiver, const char *slot );

    bool singleProjectMode() const;
    void showError( QWidget *widget, int line, const QString &message );
    void runFinished();
    void showStackFrame( QWidget *w, int line );
    void showDebugStep( QWidget *w, int line );
    void runProjectPrecondition();
    void runProjectPostcondition( QObjectList *l );


    QRESULT queryInterface( const QUuid &uuid, QUnknownInterface** iface );
    Q_REFCOUNT

private:
    MainWindow *mainWindow;

};

class DesignerProjectImpl: public DesignerProject
{
public:
    DesignerProjectImpl( Project *pr );

    Q3PtrList<DesignerFormWindow> formList() const;
    QStringList formNames() const;
    QString formFileName( const QString &form ) const;
    void addForm( DesignerFormWindow * );
    void removeForm( DesignerFormWindow * );
    QString fileName() const;
    void setFileName( const QString & );
    QString projectName() const;
    void setProjectName( const QString & );
    QString databaseFile() const;
    void setDatabaseFile( const QString & );
    void setupDatabases() const;
    Q3PtrList<DesignerDatabase> databaseConnections() const;
    void addDatabase( DesignerDatabase * );
    void removeDatabase( DesignerDatabase * );
    void save() const;
    void setLanguage( const QString & );
    QString language() const;
    void setCustomSetting( const QString &key, const QString &value );
    QString customSetting( const QString &key ) const;
    DesignerPixmapCollection *pixmapCollection() const;
    void breakPoints( QMap<QString, Q3ValueList<uint> > &bps ) const;
    QString breakPointCondition( QObject *o, int line ) const;
    void setBreakPointCondition( QObject *o, int line, const QString &condition );
    void clearAllBreakpoints() const;
    void setIncludePath( const QString &platform, const QString &path );
    void setLibs( const QString &platform, const QString &path );
    void setDefines( const QString &platform, const QString &path );
    void setConfig( const QString &platform, const QString &config );
    void setTemplate( const QString &t );
    QString config( const QString &platform ) const;
    QString libs( const QString &platform ) const;
    QString defines( const QString &platform ) const;
    QString includePath( const QString &platform ) const;
    QString templte() const;
    bool isGenericObject( QObject *o ) const;

private:
    Project *project;

};

#ifndef QT_NO_SQL
class QSqlDatabase;

class DesignerDatabaseImpl: public DesignerDatabase
{
public:
    DesignerDatabaseImpl( DatabaseConnection *d );

    QString name() const;
    void setName( const QString & );
    QString driver() const;
    void setDriver( const QString & );
    QString database() const;
    void setDatabase( const QString & );
    QString userName() const;
    void setUserName( const QString & );
    QString password() const;
    void setPassword( const QString & );
    QString hostName() const;
    void setHostName( const QString & );
    QStringList tables() const;
    void setTables( const QStringList & );
    QMap<QString, QStringList> fields() const;
    void setFields( const QMap<QString, QStringList> & );
    void open( bool suppressDialog = TRUE ) const;
    void close() const;
    QSqlDatabase* connection();
private:
    DatabaseConnection *db;

};
#endif

class DesignerPixmapCollectionImpl : public DesignerPixmapCollection
{
public:
    DesignerPixmapCollectionImpl( PixmapCollection *coll );
    void addPixmap( const QPixmap &p, const QString &name, bool force );
    QPixmap pixmap( const QString &name ) const;

private:
    PixmapCollection *pixCollection;

};

class DesignerFormWindowImpl: public DesignerFormWindow
{
public:
    DesignerFormWindowImpl( FormWindow *fw );

    QString name() const;
    void setName( const QString &n );
    QString fileName() const;
    void setFileName( const QString & );
    void save() const;
    bool isModified() const;
    void insertWidget( QWidget * );
    QWidget *create( const char *className, QWidget *parent, const char *name );
    void removeWidget( QWidget * );
    QWidgetList widgets() const;
    void undo();
    void redo();
    void cut();
    void copy();
    void paste();
    void adjustSize();
    void editConnections();
    void checkAccels();
    void layoutH();
    void layoutV();
    void layoutHSplit();
    void layoutVSplit();
    void layoutG();
    void layoutHContainer( QWidget* w );
    void layoutVContainer( QWidget* w );
    void layoutGContainer( QWidget* w );
    void breakLayout();
    void selectWidget( QWidget * );
    void selectAll();
    void clearSelection();
    bool isWidgetSelected( QWidget * ) const;
    QWidgetList selectedWidgets() const;
    QWidget *currentWidget() const;
    QWidget *form() const;
    void setCurrentWidget( QWidget * );
    Q3PtrList<QAction> actionList() const;
    QAction *createAction( const QString& text, const QIcon& icon, const QString& menuText, int accel,
			   QObject* parent, const char* name = 0, bool toggle = FALSE );
    void addAction( QAction * );
    void removeAction( QAction * );
    void preview() const;
    void addFunction( const Q3CString &function, const QString& specifier, const QString &access,
		      const QString &type, const QString &language, const QString &returnType );
    void addConnection( QObject *sender, const char *signal, QObject *receiver, const char *slot );
    void setProperty( QObject *o, const char *property, const QVariant &value );
    QVariant property( QObject *o, const char *property ) const;
    void setPropertyChanged( QObject *o, const char *property, bool changed );
    bool isPropertyChanged( QObject *o, const char *property ) const;
    void setColumnFields( QObject *o, const QMap<QString, QString> & );
    QStringList implementationIncludes() const;
    QStringList declarationIncludes() const;
    void setImplementationIncludes( const QStringList &lst );
    void setDeclarationIncludes( const QStringList &lst );
    QStringList forwardDeclarations() const;
    void setForwardDeclarations( const QStringList &lst );
    QStringList signalList() const;
    void setSignalList( const QStringList &lst );
    void addMenu( const QString &text, const QString &name );
    void addMenuAction( const QString &menu, QAction *a );
    void addMenuSeparator( const QString &menu );
    void addToolBar( const QString &text, const QString &name );
    void addToolBarAction( const QString &tb, QAction *a );
    void addToolBarSeparator( const QString &tb );

    void onModificationChange( QObject *receiver, const char *slot );

private:
    FormWindow *formWindow;

};

class DesignerDockImpl: public DesignerDock
{
public:
    DesignerDockImpl();

    Q3DockWindow *dockWindow() const;
};

class DesignerOutputDockImpl: public DesignerOutputDock
{
public:
    DesignerOutputDockImpl( OutputWindow *ow );

    QWidget *addView( const QString &pageName );
    void appendDebug( const QString & );
    void clearDebug();
    void appendError( const QString &, int );
    void clearError();

private:
    OutputWindow *outWin;

};

class DesignerSourceFileImpl : public DesignerSourceFile
{
public:
    DesignerSourceFileImpl( SourceFile *e );
    QString fileName() const;

private:
    SourceFile *ed;

};

#endif
