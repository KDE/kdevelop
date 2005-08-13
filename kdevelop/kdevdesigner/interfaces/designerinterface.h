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

#ifndef DESIGNERINTERFACE_H
#define DESIGNERINTERFACE_H

//
//  W A R N I N G  --  PRIVATE INTERFACES
//  --------------------------------------
//
// This file and the interfaces declared in the file are not
// public. It exists for internal purpose. This header file and
// interfaces may change from version to version (even binary
// incompatible) without notice, or even be removed.
//
// We mean it.
//
//

#include <private/qcom_p.h>
#include <qptrlist.h>
#include <qstring.h>
#include <qmap.h>
#include <qstringlist.h>
#include <qwidgetlist.h>
#include <qobjectlist.h>

struct DesignerProject;
struct DesignerDatabase;
struct DesignerFormWindow;
struct DesignerDock;
struct DesignerOutputDock;
struct DesignerOutput;
struct DesignerOutputError;
struct DesignerPixmapCollection;
struct DesignerSourceFile;

class QDockWindow;
class QWidget;
class QObject;
class QAction;
class QIconSet;
class QSqlDatabase;

// {0e661da-f45c-4830-af47-03ec53eb1633}
#ifndef IID_Designer
#define IID_Designer QUuid( 0xa0e661da, 0xf45c, 0x4830, 0xaf, 0x47, 0x3, 0xec, 0x53, 0xeb, 0x16, 0x33 )
#endif

/*! These are the interfaces implemented by the Qt Designer which should
  be used by plugins to access and use functionality of the Qt Designer.
*/

struct DesignerInterface : public QUnknownInterface
{
    virtual DesignerProject *currentProject() const = 0;
    virtual DesignerFormWindow *currentForm() const = 0;
    virtual DesignerSourceFile *currentSourceFile() const = 0;
    virtual QPtrList<DesignerProject> projectList() const = 0;
    virtual void showStatusMessage( const QString &, int ms = 0 ) const = 0;
    virtual DesignerDock *createDock() const = 0;
    virtual DesignerOutputDock *outputDock() const = 0;
    virtual void setModified( bool b, QWidget *window ) = 0;
    virtual void updateFunctionList() = 0;

    virtual void onProjectChange( QObject *receiver, const char *slot ) = 0;
    virtual void onFormChange( QObject *receiver, const char *slot ) = 0;

    virtual bool singleProjectMode() const = 0;
    virtual void showError( QWidget *widget, int line, const QString &message ) = 0;
    virtual void runFinished() = 0;
    virtual void showStackFrame( QWidget *w, int line ) = 0;
    virtual void showDebugStep( QWidget *w, int line ) = 0;
    virtual void runProjectPrecondition() = 0;
    virtual void runProjectPostcondition( QObjectList *l ) = 0;

    };

struct DesignerProject
{
    virtual QPtrList<DesignerFormWindow> formList() const = 0;
    virtual QStringList formNames() const = 0;
    virtual QString formFileName( const QString &form ) const = 0;
    virtual void addForm( DesignerFormWindow * ) = 0;
    virtual void removeForm( DesignerFormWindow * ) = 0;
    virtual QString fileName() const = 0;
    virtual void setFileName( const QString & ) = 0;
    virtual QString projectName() const = 0;
    virtual void setProjectName( const QString & ) = 0;
    virtual QString databaseFile() const = 0;
    virtual void setDatabaseFile( const QString & ) = 0;
    virtual void setupDatabases() const = 0;
    virtual QPtrList<DesignerDatabase> databaseConnections() const = 0;
    virtual void addDatabase( DesignerDatabase * ) = 0;
    virtual void removeDatabase( DesignerDatabase * ) = 0;
    virtual void save() const = 0;
    virtual void setLanguage( const QString & ) = 0;
    virtual QString language() const = 0;
    virtual void setCustomSetting( const QString &key, const QString &value ) = 0;
    virtual QString customSetting( const QString &key ) const = 0;
    virtual DesignerPixmapCollection *pixmapCollection() const = 0;
    virtual void breakPoints( QMap<QString, QValueList<uint> > &bps ) const = 0;
    virtual QString breakPointCondition( QObject *o, int line ) const = 0;
    virtual void setBreakPointCondition( QObject *o, int line, const QString &condition ) = 0;
    virtual void clearAllBreakpoints() const = 0;
    virtual void setIncludePath( const QString &platform, const QString &path ) = 0;
    virtual void setLibs( const QString &platform, const QString &path ) = 0;
    virtual void setDefines( const QString &platform, const QString &path ) = 0;
    virtual void setConfig( const QString &platform, const QString &config ) = 0;
    virtual void setTemplate( const QString &t ) = 0;
    virtual QString config( const QString &platform ) const = 0;
    virtual QString libs( const QString &platform ) const = 0;
    virtual QString defines( const QString &platform ) const = 0;
    virtual QString includePath( const QString &platform ) const = 0;
    virtual QString templte() const = 0;
    virtual bool isGenericObject( QObject *o ) const = 0;

};

struct DesignerDatabase
{
    virtual QString name() const = 0;
    virtual void setName( const QString & ) = 0;
    virtual QString driver() const = 0;
    virtual void setDriver( const QString & ) = 0;
    virtual QString database() const = 0;
    virtual void setDatabase( const QString & ) = 0;
    virtual QString userName() const = 0;
    virtual void setUserName( const QString & ) = 0;
    virtual QString password() const = 0;
    virtual void setPassword( const QString & ) = 0;
    virtual QString hostName() const = 0;
    virtual void setHostName( const QString & ) = 0;
    virtual QStringList tables() const = 0;
    virtual void setTables( const QStringList & ) = 0;
    virtual QMap<QString, QStringList> fields() const = 0;
    virtual void setFields( const QMap<QString, QStringList> & ) = 0;
    virtual void open( bool suppressDialog = FALSE ) const = 0;
    virtual void close() const = 0;
    virtual QSqlDatabase* connection() = 0;

};

struct DesignerPixmapCollection
{
    virtual void addPixmap( const QPixmap &p, const QString &name, bool force ) = 0;
    virtual QPixmap pixmap( const QString &name ) const = 0;
};

struct DesignerFormWindow
{
    virtual QString name() const = 0;
    virtual void setName( const QString &n ) = 0;
    virtual QString fileName() const = 0;
    virtual void setFileName( const QString & ) = 0;
    virtual void save() const = 0;
    virtual bool isModified() const = 0;
    virtual void insertWidget( QWidget * ) = 0;
    virtual void removeWidget( QWidget * ) = 0;
    virtual QWidget *create( const char *className, QWidget *parent, const char *name ) = 0;
    virtual QWidgetList widgets() const = 0;
    virtual void undo() = 0;
    virtual void redo() = 0;
    virtual void cut() = 0;
    virtual void copy() = 0;
    virtual void paste() = 0;
    virtual void adjustSize() = 0;
    virtual void editConnections() = 0;
    virtual void checkAccels() = 0;
    virtual void layoutH() = 0;
    virtual void layoutV() = 0;
    virtual void layoutHSplit() = 0;
    virtual void layoutVSplit() = 0;
    virtual void layoutG() = 0;
    virtual void layoutHContainer( QWidget* w ) = 0;
    virtual void layoutVContainer( QWidget* w ) = 0;
    virtual void layoutGContainer( QWidget* w ) = 0;
    virtual void breakLayout() = 0;
    virtual void selectWidget( QWidget * w ) = 0;
    virtual void selectAll() = 0;
    virtual void clearSelection() = 0;
    virtual bool isWidgetSelected( QWidget * ) const = 0;
    virtual QWidgetList selectedWidgets() const = 0;
    virtual QWidget *currentWidget() const = 0;
    virtual QWidget *form() const = 0;
    virtual void setCurrentWidget( QWidget * ) = 0;
    virtual QPtrList<QAction> actionList() const = 0;
    virtual QAction *createAction( const QString& text, const QIconSet& icon, const QString& menuText, int accel,
				   QObject* parent, const char* name = 0, bool toggle = FALSE ) = 0;
    virtual void addAction( QAction * ) = 0;
    virtual void removeAction( QAction * ) = 0;
    virtual void preview() const = 0;
    virtual void addFunction( const QCString &slot, const QString &specifier, const QString &access,
			     const QString &type, const QString &language, const QString &returnType ) = 0;
    virtual void addConnection( QObject *sender, const char *signal, QObject *receiver, const char *slot ) = 0;
    virtual void setProperty( QObject *o, const char *property, const QVariant &value ) = 0;
    virtual QVariant property( QObject *o, const char *property ) const = 0;
    virtual void setPropertyChanged( QObject *o, const char *property, bool changed ) = 0;
    virtual bool isPropertyChanged( QObject *o, const char *property ) const = 0;
    virtual void setColumnFields( QObject *o, const QMap<QString, QString> & ) = 0;
    virtual QStringList implementationIncludes() const = 0;
    virtual QStringList declarationIncludes() const = 0;
    virtual void setImplementationIncludes( const QStringList &lst ) = 0;
    virtual void setDeclarationIncludes( const QStringList &lst ) = 0;
    virtual QStringList forwardDeclarations() const = 0;
    virtual void setForwardDeclarations( const QStringList &lst ) = 0;
    virtual QStringList signalList() const = 0;
    virtual void setSignalList( const QStringList &lst ) = 0;
    virtual void addMenu( const QString &text, const QString &name ) = 0;
    virtual void addMenuAction( const QString &menu, QAction *a ) = 0;
    virtual void addMenuSeparator( const QString &menu ) = 0;
    virtual void addToolBar( const QString &text, const QString &name ) = 0;
    virtual void addToolBarAction( const QString &tb, QAction *a ) = 0;
    virtual void addToolBarSeparator( const QString &tb ) = 0;

    virtual void onModificationChange( QObject *receiver, const char *slot ) = 0;
};

struct DesignerSourceFile
{
    virtual QString fileName() const = 0;
};

struct DesignerDock
{
    virtual QDockWindow *dockWindow() const = 0;
};

struct DesignerOutputDock
{
    virtual QWidget *addView( const QString &pageName ) = 0;
    virtual void appendDebug( const QString & ) = 0;
    virtual void clearDebug() = 0;
    virtual void appendError( const QString &, int ) = 0;
    virtual void clearError() = 0;
};

#endif
