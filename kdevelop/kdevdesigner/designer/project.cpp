/**********************************************************************
** Copyright (C) 2000 Trolltech AS.  All rights reserved.
**
**1 This file is part of Qt Designer.
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

#include "project.h"
#include "formwindow.h"
#include "designerappiface.h"
#include "../interfaces/languageinterface.h"
#include "pixmapcollection.h"
#ifndef QT_NO_SQL
#include "dbconnectionimpl.h"
#endif
#include "resource.h"
#include <qwidgetfactory.h>
#include "outputwindow.h"

#include <qfile.h>
#include <qtextstream.h>
#include <qurl.h>
#include <qobjectlist.h>
#include <qfeatures.h>
#include <qtextcodec.h>
#include <qdom.h>
#include <qmessagebox.h>
#include <qapplication.h>
#include "mainwindow.h"
#include <qworkspace.h>

#ifndef QT_NO_SQL
#include <qsqldatabase.h>
#include <qsqlrecord.h>
#include <qdatatable.h>
#endif

#include <stdlib.h>
#ifdef Q_OS_UNIX
# include <unistd.h>
#endif

#include <klocale.h>

#ifndef QT_NO_SQL
DatabaseConnection::~DatabaseConnection()
{
    delete iface;
}

bool DatabaseConnection::refreshCatalog()
{
#ifndef QT_NO_SQL
    if ( loaded )
	return TRUE;
    if ( !open() )
	return FALSE;
    tbls = conn->tables( QSql::TableType( QSql::Tables | QSql::Views ) );
    flds.clear();
    for ( QStringList::Iterator it = tbls.begin(); it != tbls.end(); ++it ) {
	QSqlRecord fil = conn->record( *it );
	QStringList lst;
	for ( uint j = 0; j < fil.count(); ++j )
	    lst << fil.field( j )->name();
	flds.insert( *it, lst );
    }
    loaded = TRUE;
    conn->close();
    return loaded;
#else
    return FALSE;
#endif
}

#ifndef QT_NO_SQL
void DatabaseConnection::remove()
{
    if ( nm == "(default)" )
	QSqlDatabase::removeDatabase( QSqlDatabase::defaultConnection );
    else
	QSqlDatabase::removeDatabase( nm );
    // the above will effectively delete the current connection
    conn = 0;
}
#endif

bool DatabaseConnection::open( bool suppressDialog )
{
#ifndef QT_NO_SQL
    // register our name, if nec
    if ( nm == "(default)" ) {
	if ( !QSqlDatabase::contains() ) // default doesn't exists?
	    conn = QSqlDatabase::addDatabase( drv );
	else
	    conn = QSqlDatabase::database();
    } else {
	if ( !QSqlDatabase::contains( nm ) )
	    conn = QSqlDatabase::addDatabase( drv, nm );
	else
	    conn = QSqlDatabase::database( nm );
    }
    conn->setDatabaseName( dbName );
    conn->setUserName( uname );
    conn->setPassword( pword );
    conn->setHostName( hname );
    conn->setPort( prt );
    bool success = conn->open();
    for( ; suppressDialog == FALSE ; ) {
	bool done = FALSE;
	if ( !success ) {
	    DatabaseConnectionEditor dia( this, 0  , 0 , TRUE );
	    switch( dia.exec() ) {
	    case QDialog::Accepted:
		done = FALSE;
		break;
	    case QDialog::Rejected:
		done = TRUE;
		break;
	    }
	}
	if ( done )
	    break;
	conn->setUserName( uname );
	conn->setPassword( pword );
	conn->setHostName( hname );
	conn->setPort( prt );
	success = conn->open();
	if ( !success ) {
	    switch( QMessageBox::warning( project->messageBoxParent(), i18n( "Connection" ),
					  i18n( "Could not connect to the database.\n"
							    "Press 'OK' to continue or 'Cancel' to "
							    "specify different\nconnection information.\n" )
					  + QString( "[" + conn->lastError().driverText() + "\n" +
						     conn->lastError().databaseText() + "]\n" ),
					  i18n( "&OK" ),
					  i18n( "&Cancel" ), QString::null, 0, 1 ) ) {
	    case 0: // OK or Enter
		continue;
	    case 1: // Cancel or Escape
		done = TRUE;
		break;
	    }
	} else
	    break;
	if ( done )
	    break;
    }
    if ( !success ) {
	dbErr = conn->lastError().driverText() + "\n" + conn->lastError().databaseText();
	remove();
    }
    return success;
#else
    return FALSE;
#endif
}

void DatabaseConnection::close()
{
    if ( !loaded )
	return;
#ifndef QT_NO_SQL
    if ( conn ) {
	conn->close();
    }
#endif
}

DesignerDatabase *DatabaseConnection::iFace()
{
    if ( !iface )
	iface = new DesignerDatabaseImpl( this );
    return iface;
}

#endif

////////

bool Project::isDummy() const
{
    return isDummyProject;
}

Project::Project( const QString &fn, const QString &pName,
		  QPluginManager<ProjectSettingsInterface> *pm, bool isDummy,
		  const QString &l )
    : proName( pName ), projectSettingsPluginManager( pm ), isDummyProject( isDummy )
{
    modified = TRUE;
    pixCollection = new PixmapCollection( this );
    iface = 0;
    lang = l;
    is_cpp = lang == "C++";
    cfg.insert( "(all)", "qt warn_on release" );
    templ = "app";
    setFileName( fn );
    if ( !pName.isEmpty() )
	proName = pName;
    sourcefiles.setAutoDelete( TRUE );
    modified = FALSE;
    objs.setAutoDelete( FALSE );
    fakeFormFiles.setAutoDelete( FALSE );
}

Project::~Project()
{
    if ( singleProjectMode() )
	removeTempProject();
    delete iface;
    delete pixCollection;
}

void Project::setModified( bool b )
{
    modified = b;
    emit projectModified();
}

#ifndef QT_NO_SQL
DatabaseConnection *Project::databaseConnection( const QString &name )
{
    for ( DatabaseConnection *conn = dbConnections.first();
	  conn;
	  conn = dbConnections.next() ) {
	if ( conn->name() == name )
	    return conn;
    }
    return 0;
}
#endif

void Project::setFileName( const QString &fn, bool doClear )
{
    if ( fn == filename )
	return;

    if ( singleProjectMode() ) {
	QString qsa = QString( getenv( "HOME" ) ) + QString( "/.qsa" );
	if ( !QFile::exists( qsa ) ) {
	    QDir d;
	    d.mkdir( qsa );
	}
	if ( fn == singleProFileName )
	    return;
	singleProFileName = fn;
	static int counter = 0;
	QString str_counter = QString::number( counter++ );
	str_counter = "/.qsa/" + str_counter;
	LanguageInterface *iface = MetaDataBase::languageInterface( language() );
	filename = QString( getenv( "HOME" ) + str_counter + QString( "tmp_" ) +
			    QFileInfo( fn ).baseName() + "/" + QFileInfo( fn ).baseName() + ".pro" );
	removeTempProject();
	if ( iface && iface->supports( LanguageInterface::CompressProject ) ) {
	    filename = iface->uncompressProject( makeAbsolute( singleProFileName ),
						 QString( getenv( "HOME" ) +
							  str_counter + QString( "tmp_" ) +
							  QFileInfo( fn ).baseName() ) );
	    proName = makeAbsolute( singleProFileName );
	}
    } else {
	filename = fn;
	if ( !filename.endsWith( ".pro" ) )
	    filename += ".pro";
	proName = filename;
    }


    if ( proName.contains( '.' ) )
	proName = proName.left( proName.find( '.' ) );

    if ( !doClear )
	return;
    clear();
    if ( QFile::exists( filename ) )
	parse();
}

QString Project::fileName( bool singlePro ) const
{
    if ( singlePro )
	return singleProFileName;
    return filename;
}

QString Project::databaseDescription() const
{
    return dbFile;
}

QString Project::projectName() const
{
    return proName;
}

static QString parse_part( const QString &part )
{
    QString res;
    bool inName = FALSE;
    QString currName;
    for ( int i = 0; i < (int)part.length(); ++i ) {
	QChar c = part[ i ];
	if ( !inName ) {
	    if ( c != ' ' && c != '\t' && c != '\n' && c != '=' && c != '\\' && c != '+' )
		inName = TRUE;
	    else
		continue;
	}
	if ( inName ) {
	    if ( c == '\n' )
		break;
	    res += c;
	}
    }
    return res;
}

QStringList parse_multiline_part( const QString &contents, const QString &key, int *start = 0 )
{
    if ( start )
	*start = -1;
    QString lastWord;
    // Account for things like win32: SOURCES
    int extraWhiteSpaceCount = 0;
    int braceCount = 0;
    for ( int i = 0; i < (int)contents.length(); ++i ) {
	QChar c( contents[ i ] );
	switch ( c ) {
	case '{':
	    braceCount++;
	    lastWord = "";
	    break;
	case '}':
	    braceCount--;
	    lastWord = "";
	    break;
	case ' ': case '\t':
	    if (!key.startsWith(lastWord)) {
		lastWord = "";
		extraWhiteSpaceCount = 0;
	    } else {
		extraWhiteSpaceCount++;
	    }
	    break;
	case '\\': case '\n':
	    lastWord = "";
	    break;
	default:
	    lastWord += c;
	}

	// ### we should read the 'bla { SOURCES= ... }' stuff as well (braceCount > 0)
	if ( lastWord == key && braceCount == 0 ) {
	    if ( start )
		*start = i - lastWord.length() - extraWhiteSpaceCount + 1;
	    QStringList lst;
	    bool inName = FALSE;
	    QString currName;
	    bool hadEqual = FALSE;
	    for ( ; i < (int)contents.length(); ++i ) {
		c = contents[ i ];
		if ( !hadEqual && c != '=' )
		    continue;
		if ( !hadEqual ) {
		    hadEqual = TRUE;
		    continue;
		}
		if ( ( c.isLetter() || c.isDigit() || c == '.' || c == '/' || c == '_' || c == '\\' ||
		       c == '\"' || c == '\'' || c == '=' ||
		       c == '$' || c == '-' || c == '(' || c == ')' || c == ':'  || c == '+' || c == ',' || c == '~' ) &&
		     c != ' ' && c != '\t' && c != '\n' ) {
		    if ( !inName )
			currName = QString::null;
		    if ( c != '\\' || contents[i+1] != '\n' ) {
			currName += c;
			inName = TRUE;
		    }
		} else {
		    if ( inName ) {
			inName = FALSE;
			if ( currName.simplifyWhiteSpace() != "\\" )
			    lst.append( currName );
		    }
		    if ( c == '\n' && i > 0 && contents[ (int)i - 1 ] != '\\' )
			break;
		}
	    }
	    return lst;
	}
    }

    return QStringList();
}

void Project::parse()
{
    QFile f( filename );
    if ( !f.exists() || !f.open( IO_ReadOnly ) )
	return;
    QTextStream ts( &f );
    QString contents = ts.read();
    f.close();

    proName = QFileInfo( filename ).baseName();

    QStringList::ConstIterator it;

    int i = contents.find( "LANGUAGE" );
    if ( i != -1 ) {
	lang = "";
	is_cpp = FALSE;
	QString part = contents.mid( i + QString( "LANGUAGE" ).length() );
	lang = parse_part( part );
	is_cpp = lang == "C++";
    }

    i = contents.find( "DBFILE" );
    if ( i != -1 ) {
	dbFile = "";
	QString part = contents.mid( i + QString( "DBFILE" ).length() );
	dbFile = parse_part( part );
    }

    QStringList uifiles = parse_multiline_part( contents, "FORMS" );
    uifiles += parse_multiline_part( contents, "INTERFACES" ); // compatibility
    for ( it = uifiles.begin(); it != uifiles.end(); ++it ) {
	if ( (*it).startsWith( "__APPOBJ" ) )
	    continue;
	(void) new FormFile( *it, FALSE, this );
    }


    i = contents.find( "TEMPLATE" );
    if ( i != -1 ) {
	templ = "";
	QString part = contents.mid( i + QString( "TEMPLATE" ).length() );
	templ = parse_part( part );
    }

    readPlatformSettings( contents, "CONFIG", cfg );
    readPlatformSettings( contents, "LIBS", lbs );
    readPlatformSettings( contents, "INCLUDEPATH", inclPath );
    readPlatformSettings( contents, "DEFINES", defs );
    readPlatformSettings( contents, "SOURCES", sources );
    readPlatformSettings( contents, "HEADERS", headers );

    LanguageInterface *iface = MetaDataBase::languageInterface( lang );
    if ( iface ) {
	QStringList sourceKeys;
	iface->sourceProjectKeys( sourceKeys );
	for ( QStringList::Iterator it = sourceKeys.begin(); it != sourceKeys.end(); ++it ) {
	    QStringList lst = parse_multiline_part( contents, *it );
	    for ( QStringList::Iterator it = lst.begin(); it != lst.end(); ++it )
		(void) new SourceFile( *it, FALSE, this );
	}
    }

    updateCustomSettings();

    for ( it = csList.begin(); it != csList.end(); ++it ) {
	i = contents.find( *it );
	if ( i != -1 ) {
	    QString val = "";
	    QString part = contents.mid( i + QString( *it ).length() );
	    val = parse_part( part );
	    customSettings.replace( *it, val );
	}
    }

    loadConnections();

    QStringList images = parse_multiline_part( contents, "IMAGES" );

    // ### remove that for the final - this is beta-compatibility
    if ( images.isEmpty() && QDir( QFileInfo( filename ).dirPath( TRUE ) + "/images" ).exists() ) {
	    images = QDir( QFileInfo( filename ).dirPath( TRUE ) + "/images" ).entryList();
	    for ( int i = 0; i < (int)images.count(); ++i )
		images[ i ].prepend( "images/" );
	    modified = TRUE;
    }

    for ( QStringList::ConstIterator pit = images.begin(); pit != images.end(); ++pit )
	pixCollection->load( *pit );
}

void Project::clear()
{
    dbFile = "";
    proName = "unnamed";
    desc = "";
}

bool Project::removeSourceFile( SourceFile *sf )
{
    if ( !sourcefiles.containsRef( sf ) )
	return FALSE;
    if ( !sf->close() )
	return FALSE;
    sourcefiles.removeRef( sf );
    modified = TRUE;
    emit sourceFileRemoved( sf );
    return TRUE;
}

void Project::setDatabaseDescription( const QString &db )
{
    dbFile = db;
}

void Project::setDescription( const QString &s )
{
    desc = s;
}

QString Project::description() const
{
    return desc;
}


bool Project::isValid() const
{
     // #### do more checking here?
    if ( filename.isEmpty() || proName.isEmpty() )
	return FALSE;

    return TRUE;
}

QString Project::makeAbsolute( const QString &f )
{
    if ( isDummy() )
	return f;
    QString encodedUrl = QFileInfo( filename ).dirPath( TRUE );
    QUrl::encode( encodedUrl );
    QUrl u( encodedUrl, f );
    return u.path();
}

QString Project::makeRelative( const QString &f )
{
    if ( isDummy() )
	return f;
    QString p = QFileInfo( filename ).dirPath( TRUE );
    QString f2 = f;
    if ( f2.left( p.length() ) == p )
	f2.remove( 0, p.length() + 1 );
    return f2;
}

static void remove_contents( QString &contents, const QString &s )
{
    int i = contents.find( s );
    if ( i != -1 ) {
	int start = i;
	int end = contents.find( '\n', i );
	if ( end == -1 )
	    end = contents.length() - 1;
	contents.remove( start, end - start + 1 );
    }
}

static void remove_multiline_contents( QString &contents, const QString &s, int *strt = 0 )
{
    int i = contents.find( s );
    if ( strt )
	*strt = i;
    int start = i;
    bool lastWasBackspash = FALSE;
    if ( i != -1 && ( i == 0 || contents[ i - 1 ] != '{' || contents[ i - 1 ] != ':' ) ) {
	for ( ; i < (int)contents.length(); ++i ) {
	    if ( contents[ i ] == '\n' && !lastWasBackspash )
		break;
	    lastWasBackspash = ( contents[ i ] == '\\' ||
				 lastWasBackspash && ( contents[ i ] == ' ' || contents[ i ] == '\t' ) );
	}
	contents.remove( start, i - start + 1 );
    }
}

void Project::save( bool onlyProjectFile )
{
    bool anythingModified = FALSE;

    //  save sources and forms
    if ( !onlyProjectFile ) {

	saveConnections();

	for ( SourceFile *sf = sourcefiles.first(); sf; sf = sourcefiles.next() ) {
	    anythingModified = anythingModified || sf->isModified();
	    if ( !sf->save() )
		return;
	}

	for ( FormFile *ff = formfiles.first(); ff; ff = formfiles.next() ) {
	    anythingModified = anythingModified || ff->isModified();
	    if ( !ff->save() )
		return;
	}
    }

    if ( isDummy() || filename.isEmpty() )
	return;

    if ( !modified ) {
	if ( singleProjectMode() ) {
	    LanguageInterface *iface = MetaDataBase::languageInterface( language() );
	    if ( iface && iface->supports( LanguageInterface::CompressProject ) )
		iface->compressProject( makeAbsolute( filename ), singleProFileName, anythingModified );
	}
 	return;
    }

    QFile f( filename );
    QString original = "";

    // read the existing file
    bool hasPreviousContents = FALSE;
    if ( f.open( IO_ReadOnly ) ) {
	QTextStream ts( &f );
	original = ts.read();
	f.close();
	hasPreviousContents = TRUE;
	remove_contents( original, "{SOURCES+=" ); // ### compatibility with early 3.0 betas
	remove_contents( original, "DBFILE" );
	remove_contents( original, "LANGUAGE" );
	remove_contents( original, "TEMPLATE" );
	removePlatformSettings( original, "CONFIG" );
	removePlatformSettings( original, "DEFINES" );
	removePlatformSettings( original, "LIBS" );
	removePlatformSettings( original, "INCLUDEPATH" );
	removePlatformSettings( original, "SOURCES" );
	removePlatformSettings( original, "HEADERS" );
	remove_multiline_contents( original, "FORMS" );
	remove_multiline_contents( original, "INTERFACES" ); // compatibility
	remove_multiline_contents( original, "IMAGES" );
	for ( QStringList::Iterator it = csList.begin(); it != csList.end(); ++it )
	    remove_contents( original, *it );
    }

    // the contents of the saved file
    QString contents;

    // template
    contents += "TEMPLATE\t= " + templ + "\n";

    // language
    contents += "LANGUAGE\t= " + lang + "\n";
    contents += "\n";

    // config
    writePlatformSettings( contents, "CONFIG", cfg );
    LanguageInterface *iface = MetaDataBase::languageInterface( lang );
    if ( iface ) {
	QStringList sourceKeys;
	iface->sourceProjectKeys( sourceKeys );
	for ( QStringList::Iterator spit = sourceKeys.begin(); spit != sourceKeys.end(); ++spit )
	    remove_multiline_contents( contents, *spit );
    }
    contents += "\n";

    // libs, defines, includes
    writePlatformSettings( contents, "LIBS", lbs );
    writePlatformSettings( contents, "DEFINES", defs );
    writePlatformSettings( contents, "INCLUDEPATH", inclPath );
    writePlatformSettings( contents, "SOURCES", sources );
    writePlatformSettings( contents, "HEADERS", headers );

    // unix
    if ( !hasPreviousContents ) {
 	contents +=
 	    "unix {\n"
 	    "  UI_DIR = .ui\n"
 	    "  MOC_DIR = .moc\n"
 	    "  OBJECTS_DIR = .obj\n"
 	    "}\n";
    }
    contents += "\n";

    // sources
    if ( !sourcefiles.isEmpty() && iface ) {
	QMap<QString, QStringList> sourceToKey;
	for ( SourceFile *f = sourcefiles.first(); f; f = sourcefiles.next() ) {
	    QString key = iface->projectKeyForExtension( QFileInfo( f->fileName() ).extension() );
	    QStringList lst = sourceToKey[ key ];
	    lst << makeRelative( f->fileName() );
	    sourceToKey.replace( key, lst );
	}

	for ( QMap<QString, QStringList>::Iterator skit = sourceToKey.begin();
	      skit != sourceToKey.end(); ++skit ) {
	    QString part = skit.key() + "\t+= ";
	    QStringList lst = *skit;
	    for ( QStringList::Iterator sit = lst.begin(); sit != lst.end(); ++sit ) {
		part += *sit;
		part += ++sit != lst.end() ? " \\\n\t" : "";
		--sit;
	    }
	    part += "\n";
	    contents += part;
	}
    }

    // forms and interfaces
    if ( !formfiles.isEmpty() ) {
	contents += "FORMS\t= ";
	for ( QPtrListIterator<FormFile> fit = formfiles; fit.current(); ++fit ) {
	    contents += fit.current()->fileName() +
		 (fit != formfiles.last() ? " \\\n\t" : "");
	}
	contents += "\n";
    }

    // images
     if ( !pixCollection->isEmpty() ) {
	contents += "IMAGES\t= ";
	QValueList<PixmapCollection::Pixmap> pixmaps = pixCollection->pixmaps();
	for ( QValueList<PixmapCollection::Pixmap>::Iterator it = pixmaps.begin();
	      it != pixmaps.end(); ++it ) {
		  contents += makeRelative( (*it).absname );
		  contents += ++it != pixmaps.end() ? " \\\n\t" : "";
		  --it;
	}
	contents += "\n";
    }

    // database
    if ( !dbFile.isEmpty() )
	contents += "DBFILE\t= " + dbFile + "\n";
    contents += "\n";

    // custom settings
    for ( QStringList::Iterator it = csList.begin(); it != csList.end(); ++it ) {
	QString val = *customSettings.find( *it );
	if ( !val.isEmpty() )
	    contents += *it + "\t= " + val + "\n";
    }

    if ( !f.open( IO_WriteOnly | IO_Translate ) ) {
	QMessageBox::warning( messageBoxParent(),
			      "Save Project Failed", "Couldn't write project file " + filename );
	return;
    }

    QTextStream os( &f );
    os << contents;
    if (hasPreviousContents)
        os << original;

    f.close();

    setModified( FALSE );

    if ( singleProjectMode() ) {
	LanguageInterface *iface = MetaDataBase::languageInterface( language() );
	if ( iface && iface->supports( LanguageInterface::CompressProject ) )
	    iface->compressProject( makeAbsolute( filename ), singleProFileName, TRUE );
    }
}

#ifndef QT_NO_SQL
QPtrList<DatabaseConnection> Project::databaseConnections() const
{
    return dbConnections;
}
#endif

#ifndef QT_NO_SQL
void Project::setDatabaseConnections( const QPtrList<DatabaseConnection> &lst )
{
    dbConnections = lst;
}
#endif

#ifndef QT_NO_SQL
void Project::addDatabaseConnection( DatabaseConnection *conn )
{
    dbConnections.append( conn );
    modified = TRUE;
}
#endif

#ifndef QT_NO_SQL
void Project::removeDatabaseConnection( const QString &c )
{
    for ( DatabaseConnection *conn = dbConnections.first(); conn; conn = dbConnections.next() ) {
	if ( conn->name() == c ) {
	    conn->remove();
	    dbConnections.removeRef( conn );
	    delete conn;
	    return;
	}
    }
}
#endif

#ifndef QT_NO_SQL
QStringList Project::databaseConnectionList()
{
    QStringList lst;
    for ( DatabaseConnection *conn = dbConnections.first(); conn; conn = dbConnections.next() )
	lst << conn->name();
    return lst;
}
#endif

#ifndef QT_NO_SQL
QStringList Project::databaseTableList( const QString &connection )
{
    DatabaseConnection *conn = databaseConnection( connection );
    if ( !conn ) {
	return QStringList();
    }
    return conn->tables();
}
#endif

#ifndef QT_NO_SQL
QStringList Project::databaseFieldList( const QString &connection, const QString &table )
{
    DatabaseConnection *conn = databaseConnection( connection );
    if ( !conn )
	return QStringList();
    return conn->fields( table );
}
#endif

#ifndef QT_NO_SQL
static QString makeIndent( int indent )
{
    QString s;
    s.fill( ' ', indent * 4 );
    return s;
}
#endif

#ifndef QT_NO_SQL
static void saveSingleProperty( QTextStream &ts, const QString& name, const QString& value, int indent )
{
    ts << makeIndent( indent ) << "<property name=\"" << name << "\">" << endl;
    ++indent;
    ts << makeIndent( indent ) << "<string>" << value << "</string>" << endl;
    --indent;
    ts << makeIndent( indent ) << "</property>" << endl;
}
#endif

void Project::saveConnections()
{
#ifndef QT_NO_SQL
    if ( dbFile.isEmpty() ) {
	QFileInfo fi( fileName() );
	setDatabaseDescription( fi.baseName() + ".db" );
    }

    QFile f( makeAbsolute( dbFile ) );

    if ( dbConnections.isEmpty() ) {
	if ( f.exists() )
	    f.remove();
	setDatabaseDescription( "" );
	modified = TRUE;
	return;
    }

    /* .db xml */
    if ( f.open( IO_WriteOnly | IO_Translate ) ) {
	QTextStream ts( &f );
	ts.setCodec( QTextCodec::codecForName( "UTF-8" ) );
	ts << "<!DOCTYPE DB><DB version=\"1.0\">" << endl;

	/* db connections */
	int indent = 0;
	for ( DatabaseConnection *conn = dbConnections.first(); conn; conn = dbConnections.next() ) {
	    ts << makeIndent( indent ) << "<connection>" << endl;
	    ++indent;
	    saveSingleProperty( ts, "name", conn->name(), indent );
	    saveSingleProperty( ts, "driver", conn->driver(), indent );
	    saveSingleProperty( ts, "database", conn->database(), indent );
	    saveSingleProperty( ts, "username", conn->username(), indent );
	    saveSingleProperty( ts, "hostname", conn->hostname(), indent );
	    saveSingleProperty( ts, "port", QString::number( conn->port() ), indent );

	    /* connection tables */
	    QStringList tables = conn->tables();
	    for ( QStringList::Iterator it = tables.begin();
		  it != tables.end(); ++it ) {
		ts << makeIndent( indent ) << "<table>" << endl;
		++indent;
		saveSingleProperty( ts, "name", (*it), indent );

		/* tables fields */
		QStringList fields = conn->fields( *it );
		for ( QStringList::Iterator it2 = fields.begin();
		      it2 != fields.end(); ++it2 ) {
		    ts << makeIndent( indent ) << "<field>" << endl;
		    ++indent;
		    saveSingleProperty( ts, "name", (*it2), indent );
		    --indent;
		    ts << makeIndent( indent ) << "</field>" << endl;
		}

		--indent;
		ts << makeIndent( indent ) << "</table>" << endl;
	    }

	    --indent;
	    ts << makeIndent( indent ) << "</connection>" << endl;
	}

	ts << "</DB>" << endl;
	f.close();
    }
#endif
}

#ifndef QT_NO_SQL
static QDomElement loadSingleProperty( QDomElement e, const QString& name )
{
    QDomElement n;
    for ( n = e.firstChild().toElement();
	  !n.isNull();
	  n = n.nextSibling().toElement() ) {
	if ( n.tagName() == "property" && n.toElement().attribute("name") == name )
	    return n;
    }
    return n;
}
#endif

void Project::loadConnections()
{
#ifndef QT_NO_SQL
    if ( dbFile.isEmpty() || !QFile::exists( makeAbsolute( dbFile ) ) )
	return;

    QFile f( makeAbsolute( dbFile ) );
    if ( f.open( IO_ReadOnly ) ) {
	QDomDocument doc;
	QString errMsg;
	int errLine;
	if ( doc.setContent( &f, &errMsg, &errLine ) ) {
	    QDomElement e;
	    e = doc.firstChild().toElement();

	    /* connections */
	    QDomNodeList connections = e.toElement().elementsByTagName( "connection" );
	    for ( uint i = 0; i <  connections.length(); i++ ) {
		QDomElement connection = connections.item(i).toElement();
		QDomElement connectionName = loadSingleProperty( connection, "name" );
		QDomElement connectionDriver = loadSingleProperty( connection, "driver" );
		QDomElement connectionDatabase = loadSingleProperty( connection,
								     "database" );
		QDomElement connectionUsername = loadSingleProperty( connection,
								     "username" );
		QDomElement connectionHostname = loadSingleProperty( connection,
								     "hostname" );
		QDomElement connectionPort = loadSingleProperty( connection,
								     "port" );

		DatabaseConnection *conn = new DatabaseConnection( this );
		conn->setName( connectionName.firstChild().firstChild().toText().data() );
		conn->setDriver( connectionDriver.firstChild().firstChild().toText().data() );
		conn->setDatabase( connectionDatabase.firstChild().firstChild().toText().data() );
		conn->setUsername( connectionUsername.firstChild().firstChild().toText().data() );
		conn->setHostname( connectionHostname.firstChild().firstChild().toText().data() );
		conn->setPort( QString( connectionPort.firstChild().firstChild().toText().data() ).toInt() );

		/* connection tables */
		QDomNodeList tables = connection.toElement().elementsByTagName( "table" );
		for ( uint j = 0; j <  tables.length(); j++ ) {
		    QDomElement table = tables.item(j).toElement();
		    QDomElement tableName = loadSingleProperty( table, "name" );
		    conn->addTable( tableName.firstChild().firstChild().toText().data() );

		    /* table fields */
		    QStringList fieldList;
		    QDomNodeList fields = table.toElement().elementsByTagName( "field" );
		    for ( uint k = 0; k <  fields.length(); k++ ) {
			QDomElement field = fields.item(k).toElement();
			QDomElement fieldName = loadSingleProperty( field, "name" );
			fieldList.append( fieldName.firstChild().firstChild().toText().data() );
		    }
		    conn->setFields( tableName.firstChild().firstChild().toText().data(),
					 fieldList );
		}

		dbConnections.append( conn );
	    }
	} else {
	    qDebug( QString("Parse error: ") + errMsg + QString(" in line %d"), errLine );
	}
	f.close();
    }
#endif
}

/*! Opens the database \a connection.  The connection remains open and
can be closed again with closeDatabase().
*/

bool Project::openDatabase( const QString &connection, bool suppressDialog )
{
#ifndef QT_NO_SQL
    DatabaseConnection *conn = databaseConnection( connection );
    if ( connection.isEmpty() && !conn )
	conn = databaseConnection( "(default)" );
    if ( !conn )
	return FALSE;
    bool b = conn->open( suppressDialog );
    return b;
#else
    Q_UNUSED( connection );
    Q_UNUSED( suppressDialog );
    return FALSE;
#endif
}

/*! Closes the database \a connection.
*/
void Project::closeDatabase( const QString &connection )
{
#ifndef QT_NO_SQL
    DatabaseConnection *conn = databaseConnection( connection );
    if ( connection.isEmpty() && !conn )
	conn = databaseConnection( "(default)" );
    if ( !conn )
	return;
    conn->close();
#else
    Q_UNUSED( connection );
#endif
}

// void Project::formClosed( FormWindow *fw )
// {
//     formWindows.remove( fw );
// }

QObjectList *Project::formList( bool resolveFakeObjects ) const
{
    QObjectList *l = new QObjectList;
    for ( QPtrListIterator<FormFile> forms(formfiles);   forms.current(); ++forms ) {
	FormFile* f = forms.current();
	if ( f->formWindow() ) {
	    if ( resolveFakeObjects && f->formWindow()->isFake() )
		l->append( objectForFakeForm( f->formWindow() ) );
	    else
		l->append( f->formWindow()->child( 0, "QWidget" ) );
	} else if ( f->isFake() ) {
	    l->append( objectForFakeFormFile( f ) );
	}
    }
    return l;
}

DesignerProject *Project::iFace()
{
    if ( !iface )
	iface = new DesignerProjectImpl( this );
    return iface;
}

void Project::setLanguage( const QString &l )
{
    if ( l == lang )
	return;
    lang = l;
    is_cpp = lang == "C++";
    updateCustomSettings();
    modified = TRUE;
}

QString Project::language() const
{
    return lang;
}

void Project::setCustomSetting( const QString &key, const QString &value )
{
    customSettings.remove( key );
    customSettings.insert( key, value );
    modified = TRUE;
}

QString Project::customSetting( const QString &key ) const
{
    return *customSettings.find( key );
}

void Project::updateCustomSettings()
{
    if ( !projectSettingsPluginManager )
	return;

/*
    ProjectSettingsInterface *iface = 0;
    projectSettingsPluginManager->queryInterface( lang, (QUnknownInterface**)&iface );
    if ( !iface )
	return;
    csList = iface->projectSettings();
    iface->release();
*/

    QInterfacePtr<ProjectSettingsInterface> iface;
    projectSettingsPluginManager->queryInterface( lang, &iface );
    if ( !iface )
	return;
    csList = iface->projectSettings();
    customSettings.clear();

}

void Project::setActive( bool b )
{
    pixCollection->setActive( b );
}

void Project::addSourceFile( SourceFile *sf )
{
    sourcefiles.append( sf );
    modified = TRUE;
    emit sourceFileAdded( sf );
}


SourceFile* Project::findSourceFile( const QString& filename, SourceFile *ignore ) const
{
    QPtrListIterator<SourceFile> it(sourcefiles);
    while ( it.current() ) {
	if ( it.current() != ignore && it.current()->fileName() == filename )
	    return it.current();
	++it;
    }
    return 0;
}

FormFile* Project::findFormFile( const QString& filename, FormFile *ignore ) const
{
    QPtrListIterator<FormFile> it(formfiles);
    while ( it.current() ) {
	if ( it.current() != ignore && it.current()->fileName() == filename )
	    return it.current();
	++it;
    }
    return 0;
}

void Project::setIncludePath( const QString &platform, const QString &path )
{
    if ( inclPath[platform] == path )
	return;
    inclPath.replace( platform, path );
    modified = TRUE;
}

void Project::setLibs( const QString &platform, const QString &path )
{
    lbs.replace( platform, path );
}

void Project::setDefines( const QString &platform, const QString &path )
{
    defs.replace( platform, path );
}

void Project::setConfig( const QString &platform, const QString &config )
{
    cfg.replace( platform, config );
}

QString Project::config( const QString &platform ) const
{
    return cfg[ platform ];
}

QString Project::libs( const QString &platform ) const
{
    return lbs[ platform ];
}

QString Project::defines( const QString &platform ) const
{
    return defs[ platform ];
}

QString Project::includePath( const QString &platform ) const
{
    return inclPath[ platform ];
}

QString Project::templte() const
{
    return templ;
}

void Project::setTemplate( const QString &t )
{
    templ = t;
}

void Project::readPlatformSettings( const QString &contents,
				    const QString &setting,
				    QMap<QString, QString> &res )
{
    const QString platforms[] = { "", "win32", "unix", "mac", QString::null };
    for ( int i = 0; platforms[ i ] != QString::null; ++i ) {
	QString p = platforms[ i ];
	if ( !p.isEmpty() )
	    p += ":";
	QStringList lst = parse_multiline_part( contents, p + setting );
	QString s = lst.join( " " );
	QString key = platforms[ i ];
	if ( key.isEmpty() )
	    key = "(all)";
	res.replace( key, s );
    }
}

void Project::removePlatformSettings( QString &contents, const QString &setting )
{
    const QString platforms[] = { "win32", "unix", "mac", "", QString::null };
    for ( int i = 0; platforms[ i ] != QString::null; ++i ) {
	QString p = platforms[ i ];
	if ( !p.isEmpty() )
	    p += ":";
	remove_multiline_contents( contents, p + setting );
    }
}

void Project::writePlatformSettings( QString &contents, const QString &setting,
				     const QMap<QString, QString> &input )
{
    const QString platforms[] = { "", "win32", "unix", "mac", QString::null };
    int i;
    LanguageInterface *iface = MetaDataBase::languageInterface( lang );
    if (iface && (setting == "SOURCES" || setting == "HEADERS")) // The (all) part will be saved later on
        i = 1;
    else
	i = 0;
    for (; platforms[ i ] != QString::null; ++i ) {
	QString p = platforms[ i ];
	if ( !p.isEmpty() )
	    p += ":";
	QString key = platforms[ i ];
	if ( key.isEmpty() )
	    key = "(all)";
	QMap<QString, QString>::ConstIterator it = input.find( key );
	if ( it == input.end() || (*it).isEmpty() )
	    continue;
	contents += p + setting + "\t+= " + *it + "\n";
    }
}

void Project::addFormFile( FormFile *ff )
{
    formfiles.append( ff );
    modified = TRUE;
    emit formFileAdded( ff );
}

bool Project::removeFormFile( FormFile *ff )
{
    if ( !formfiles.containsRef( ff ) )
	return FALSE;
    if ( !ff->close() )
	return FALSE;
    formfiles.removeRef( ff );
    modified = TRUE;
    emit formFileRemoved( ff );
    return TRUE;
}

void Project::addObject( QObject *o )
{
    bool wasModified = modified;
    objs.append( o );
    FormFile *ff = new FormFile( "", FALSE, this, "qt_fakewindow" );
    ff->setFileName( "__APPOBJ" + QString( o->name() ) + ".ui" );
    fakeFormFiles.insert( (void*)o, ff );
    MetaDataBase::addEntry( o );
    if ( hasGUI() ) {
	QWidget *parent = MainWindow::self ? MainWindow::self->qWorkspace() : 0;
	FormWindow *fw = new FormWindow( ff, MainWindow::self, parent, "qt_fakewindow" );
	fw->setProject( this );
	if ( QFile::exists( ff->absFileName() ) )
	    Resource::loadExtraSource( ff, ff->absFileName(),
				       MetaDataBase::languageInterface( language() ), FALSE );
	if ( MainWindow::self )
	    fw->setMainWindow( MainWindow::self );
	if ( MainWindow::self ) {
	    QApplication::sendPostedEvents( MainWindow::self->qWorkspace(), QEvent::ChildInserted );
	    connect( fw,
		     SIGNAL( undoRedoChanged( bool, bool, const QString &, const QString & ) ),
		     MainWindow::self,
		     SLOT( updateUndoRedo( bool, bool, const QString &, const QString & ) )
		);
	}
	if ( fw->parentWidget() ) {
	    fw->parentWidget()->setFixedSize( 1, 1 );
	    fw->show();
	}
    } else {
	if ( QFile::exists( ff->absFileName() ) )
	    Resource::loadExtraSource( ff, ff->absFileName(),
				       MetaDataBase::languageInterface( language() ), FALSE );
    }
    emit objectAdded( o );
    modified = wasModified;
}

void Project::setObjects( const QObjectList &ol )
{
    for ( QObjectListIt it( ol ); it.current(); ++it )
	addObject( it.current() );
}

void Project::removeObject( QObject *o )
{
    bool wasModified = modified;
    objs.removeRef( o );
    MetaDataBase::removeEntry( o );
    fakeFormFiles.remove( (void*)o );
    emit objectRemoved( o );
    modified = wasModified;
}

QObjectList Project::objects() const
{
    return objs;
}

FormFile *Project::fakeFormFileFor( QObject *o ) const
{
    return fakeFormFiles.find( (void*)o );
}

QObject *Project::objectForFakeForm( FormWindow *fw ) const
{
    for ( QPtrDictIterator<FormFile> it( fakeFormFiles ); it.current(); ++it ) {
	if ( it.current()->formWindow() == fw ||
	    it.current() == fw->formFile() )
	    return (QObject*)it.currentKey();
    }
    return 0;
}

QObject *Project::objectForFakeFormFile( FormFile *ff ) const
{
    for ( QPtrDictIterator<FormFile> it( fakeFormFiles ); it.current(); ++it ) {
	if ( it.current() == ff )
	    return (QObject*)it.currentKey();
    }
    return 0;
}

void Project::removeTempProject()
{
    if ( !singleProjectMode() )
	return;
    QDir d( QFileInfo( filename ).dirPath() );
    if ( !d.exists( QFileInfo( filename ).dirPath() ) )
	return;
    QStringList files = d.entryList( QDir::Files );
    QStringList::Iterator it;
    for ( it = files.begin(); it != files.end(); ++it ) {
	d.remove( *it );
    }
    if ( d.exists( QFileInfo( filename ).dirPath() + "/images" ) ) {
	d = QDir( QFileInfo( filename ).dirPath() + "/images" );
	files = d.entryList( QDir::Files );
	for ( it = files.begin(); it != files.end(); ++it )
	    d.remove( *it );
	d = QDir( QFileInfo( filename ).dirPath() );
	d.remove( "images" );
    }
    d.remove( QFileInfo( filename ).dirPath() );
#if defined(Q_OS_UNIX)
    // ##### implement for all platforms, ideally should be in Qt
    ::rmdir( QFile::encodeName( d.absPath() ) );
#endif
}

void Project::addAndEditFunction( const QString &function, const QString &functionBody, bool openDeveloper )
{
    for ( SourceFile *f = sourcefiles.first(); f; f = sourcefiles.next() ) {
	if ( QFileInfo( f->fileName() ).baseName() == "main" ) {
	    QValueList<LanguageInterface::Function> funcs;
	    LanguageInterface *iface = MetaDataBase::languageInterface( language() );
	    if ( !iface )
		return;
	    iface->functions( f->text(), &funcs );
	    QString func = function;
	    int i = func.find( '(' );
	    if ( i != -1 )
		func = func.left( i );

	    bool found = FALSE;
	    for ( QValueList<LanguageInterface::Function>::Iterator it = funcs.begin();
		  it != funcs.end(); ++it ) {
		if ( (*it).name.left( (*it).name.find( '(' ) ) == func ) {
		    found = TRUE;
		    break;
		}
	    }

	    if ( !found ) {
		QString code = f->text();
		if ( functionBody.isEmpty() )
		    code += "\n\n" + iface->createFunctionStart( "", func, "", "" ) + "()\n{\n\n}\n";
		else
		    code += "\n\n" + iface->createFunctionStart( "", func, "", "" ) +
			    "()\n" + functionBody + "\n";
		f->setText( code );
		if ( f->editor() )
		    f->editor()->refresh( FALSE );
	    }

	    if ( openDeveloper ) {
		if ( MainWindow::self )
		    MainWindow::self->editSource( f );
		f->editor()->setFunction( func, "" );
	    }

	    break;
	}
    }
}

bool Project::hasParentObject( QObject *o )
{
    for ( QObject *p = objs.first(); p; p = objs.next() ) {
	QObject *c = p->child( o->name(), o->className() );
	if ( c )
	    return TRUE;
    }
    return FALSE;
}

QString Project::qualifiedName( QObject *o )
{
    QString name = o->name();
    QObject *p = o->parent();
    while ( p ) {
	name.prepend( QString( p->name() ) + "." );
	if ( objs.findRef( p ) != -1 )
	    break;
	p = p->parent();
    }
    return name;
}

bool Project::singleProjectMode() const
{
    return !MainWindow::self || MainWindow::self->singleProjectMode();
}

QWidget *Project::messageBoxParent() const
{
    return MainWindow::self;
}

void Project::designerCreated()
{
    for ( FormFile *ff = formfiles.first(); ff; ff = formfiles.next() ) {
	FormWindow *fw = ff->formWindow();
	if ( !fw || fw->mainWindow() )
	    continue;
	fw->setMainWindow( MainWindow::self );
	connect( fw, SIGNAL( undoRedoChanged( bool, bool, const QString &,
					      const QString & ) ),
		 MainWindow::self, SLOT( updateUndoRedo( bool, bool,
					 const QString &, const QString & ) ) );
	fw->reparent( MainWindow::self->qWorkspace(), QPoint( 0, 0 ), FALSE );
	QApplication::sendPostedEvents( MainWindow::self->qWorkspace(),
					QEvent::ChildInserted );
	fw->parentWidget()->setFixedSize( 1, 1 );
	fw->show();
    }
}

void Project::formOpened( FormWindow *fw )
{
    if ( fw->isFake() )
	return;
    emit newFormOpened( fw );
}

QString Project::locationOfObject( QObject *o )
{
    if ( !o )
	return QString::null;

    if ( MainWindow::self ) {
	QWidgetList windows = MainWindow::self->qWorkspace()->windowList();
	for ( QWidget *w = windows.first(); w; w = windows.next() ) {
	    FormWindow *fw = ::qt_cast<FormWindow*>(w);
	    SourceEditor *se = ::qt_cast<SourceEditor*>(w);
	    if ( fw ) {
		if ( fw->isFake() )
		    return objectForFakeForm( fw )->name() + QString( " [Source]" );
		else
		    return fw->name() + QString( " [Source]" );
	    } else if ( se ) {
		if ( !se->object() )
		    continue;
		if ( se->formWindow() )
		    return se->formWindow()->name() + QString( " [Source]" );
		else
		    return makeRelative( se->sourceFile()->fileName() );
	    }
	}
    }

    if ( ::qt_cast<SourceFile*>(o) ) {
	for ( QPtrListIterator<SourceFile> sources = sourceFiles();
	      sources.current(); ++sources ) {
	    SourceFile* f = sources.current();
	    if ( f == o )
		return makeRelative( f->fileName() );
	}
    }

    extern QMap<QWidget*, QString> *qwf_forms;
    if ( !qwf_forms ) {
	qWarning( "Project::locationOfObject: qwf_forms is NULL!" );
	return QString::null;
    }

    QString s = makeRelative( *qwf_forms->find( (QWidget*)o ) );
    s += " [Source]";
    return s;
}

bool Project::hasGUI() const
{
    return qApp->type() != QApplication::Tty;
}
