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

#ifndef PROJECT_H
#define PROJECT_H

#include <qfeatures.h>

#include <qstring.h>
#include <qstringlist.h>
#include <qptrlist.h>
#include <qmap.h>
#include <private/qpluginmanager_p.h>
#include "../interfaces/projectsettingsiface.h"
#include "sourcefile.h"
#include "formfile.h"
#include <qobjectlist.h>
#include <qptrdict.h>
#include "formfile.h"

class QObjectList;
struct DesignerProject;
struct DesignerDatabase;
class PixmapCollection;
class Project;

#ifndef QT_NO_SQL
class QSqlDatabase;

class DatabaseConnection
{
public:
    DatabaseConnection( Project *p ) :
#ifndef QT_NO_SQL
	conn( 0 ),
#endif
	project( p ), loaded( FALSE ), iface( 0 ) {}
    ~DatabaseConnection();

    bool refreshCatalog();
    bool open( bool suppressDialog = TRUE );
    void close();
    DesignerDatabase *iFace();

    bool isLoaded() const { return loaded; }
    void setName( const QString& n ) { nm = n; }
    QString name() const { return nm; }
    void setDriver( const QString& d ) { drv = d; }
    QString driver() const { return drv; }
    void setDatabase( const QString& db ) { dbName = db; }
    QString database() const { return dbName; }
    void setUsername( const QString& u ) { uname = u; }
    QString username() const { return uname; }
    void setPassword( const QString& p ) { pword = p; }
    QString password() const { return pword; }
    void setHostname( const QString& h ) { hname = h; }
    QString hostname() const { return hname; }
    void setPort( int p ) { prt = p; }
    int port() const { return prt; }
    QString lastError() const { return dbErr; }
    void addTable( const QString& t ) { tbls.append(t); }
    void setFields( const QString& t, const QStringList& f ) { flds[t] = f; }
    QStringList tables() const { return tbls; }
    QStringList fields( const QString& t ) { return flds[t]; }
    QMap<QString, QStringList> fields() { return flds; }
#ifndef QT_NO_SQL
    QSqlDatabase* connection() const { return conn; }
    void remove();
#endif

private:
    QString nm;
    QString drv, dbName, uname, pword, hname;
    QString dbErr;
    int prt;
    QStringList tbls;
    QMap<QString, QStringList> flds;
#ifndef QT_NO_SQL
    QSqlDatabase *conn;
#endif
    Project *project;
    bool loaded;
    DesignerDatabase *iface;
};

#endif

class Project : public QObject
{
    Q_OBJECT
    friend class DatabaseConnection;

public:
    Project( const QString &fn, const QString &pName = QString::null,
	     QPluginManager<ProjectSettingsInterface> *pm = 0, bool isDummy = FALSE,
	     const QString &l = "C++" );
    ~Project();

    void setFileName( const QString &fn, bool doClear = TRUE );
    QString fileName( bool singlePro = FALSE ) const;
    QString projectName() const;

    void setDatabaseDescription( const QString &db );
    QString databaseDescription() const;

    void setDescription( const QString &s );
    QString description() const;

    void setLanguage( const QString &l );
    QString language() const;


    bool isValid() const;

    // returns TRUE if this project is the <No Project> project
    bool isDummy() const;

    QString makeAbsolute( const QString &f );
    QString makeRelative( const QString &f );

    void save( bool onlyProjectFile = FALSE );

#ifndef QT_NO_SQL
    QPtrList<DatabaseConnection> databaseConnections() const;
    void setDatabaseConnections( const QPtrList<DatabaseConnection> &lst );
    void addDatabaseConnection( DatabaseConnection *conn );
    void removeDatabaseConnection( const QString &conn );
    DatabaseConnection *databaseConnection( const QString &name );
    QStringList databaseConnectionList();
    QStringList databaseTableList( const QString &connection );
    QStringList databaseFieldList( const QString &connection, const QString &table );
#endif
    void saveConnections();
    void loadConnections();

    bool openDatabase( const QString &connection, bool suppressDialog = TRUE );
    void closeDatabase( const QString &connection );

    QObjectList *formList( bool resolveFakeObjects = FALSE ) const;

    DesignerProject *iFace();

    void setCustomSetting( const QString &key, const QString &value );
    QString customSetting( const QString &key ) const;

    PixmapCollection *pixmapCollection() const { return pixCollection; }

    void setActive( bool b );

    QPtrListIterator<SourceFile> sourceFiles() const { return QPtrListIterator<SourceFile>(sourcefiles); }
    void addSourceFile( SourceFile *sf );
    bool removeSourceFile( SourceFile *sf );
    SourceFile* findSourceFile( const QString& filename, SourceFile *ignore = 0 ) const;

    QPtrListIterator<FormFile> formFiles() const { return QPtrListIterator<FormFile>(formfiles); }
    void addFormFile( FormFile *ff );
    bool removeFormFile( FormFile *ff );
    FormFile* findFormFile( const QString& filename, FormFile *ignore = 0 ) const;

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

    bool isModified() const { return !isDummy() && modified; }
    void setModified( bool b );

    void addObject( QObject *o );
    void setObjects( const QObjectList &ol );
    void removeObject( QObject *o );
    QObjectList objects() const;
    FormFile *fakeFormFileFor( QObject *o ) const;
    QObject *objectForFakeForm( FormWindow *fw ) const;
    QObject *objectForFakeFormFile( FormFile *ff ) const;

    void addAndEditFunction( const QString &functionName, const QString &functionBody,
			     bool openDeveloper );

    void removeTempProject();
    bool hasParentObject( QObject *o );
    QString qualifiedName( QObject *o );

    bool isCpp() const { return is_cpp; }

    void designerCreated();

    void formOpened( FormWindow *fw );

    QString locationOfObject( QObject *o );

    bool hasGUI() const;

signals:
    void projectModified();
    void sourceFileAdded( SourceFile* );
    void sourceFileRemoved( SourceFile* );
    void formFileAdded( FormFile* );
    void formFileRemoved( FormFile* );
    void objectAdded( QObject * );
    void objectRemoved( QObject * );
    void newFormOpened( FormWindow *fw );

private:
    void parse();
    void clear();
    void updateCustomSettings();
    void readPlatformSettings( const QString &contents,
			       const QString &setting,
			       QMap<QString, QString> &res );
    void removePlatformSettings( QString &contents, const QString &setting );
    void writePlatformSettings( QString &contents, const QString &setting,
				const QMap<QString, QString> &input );
    bool singleProjectMode() const;
    QWidget *messageBoxParent() const;

private:
    QString filename;
    QString proName;
    QString desc;
    QString dbFile;
#ifndef QT_NO_SQL
    QPtrList<DatabaseConnection> dbConnections;
#endif
    QString lang;
    DesignerProject *iface;
    QMap<QString, QString> customSettings;
    QStringList csList;
    QPluginManager<ProjectSettingsInterface> *projectSettingsPluginManager;
    PixmapCollection *pixCollection;
    QPtrList<SourceFile> sourcefiles;
    QPtrList<FormFile> formfiles;
    QMap<QString, QString> inclPath, defs, lbs, cfg, sources, headers;
    QString templ;
    bool isDummyProject;
    bool modified;
    QObjectList objs;
    QPtrDict<FormFile> fakeFormFiles;
    QString singleProFileName;
    bool is_cpp;

};

#endif
