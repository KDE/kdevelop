/***************************************************************************
 *   Copyright (C) 2002 by Roberto Raggi                                   *
 *   roberto@kdevelop.org                                                 *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "backgroundparser.h"
#include "cppsupportpart.h"
#include "cppsupport_events.h"

#include "driver.h"
#include "ast_utils.h"

#include <kparts/part.h>

#include <ktexteditor/editinterface.h>
#include <ktexteditor/document.h>
#include <ktexteditor/view.h>

#include <kdevpartcontroller.h>
#include <kdevproject.h>

#include <kurl.h>
#include <kdebug.h>
#include <kapplication.h>

#include <qfile.h>
#include <qfileinfo.h>
#include <qtextstream.h>
#include <qprocess.h>

#include <stdlib.h>
#include <unistd.h>

class KDevDriver: public Driver
{
public:
    KDevDriver( CppSupportPart* cppSupport )
        : m_cppSupport( cppSupport )
    {
	setupProject();
	setup();
    }

    void setupProject()
    {
	QMap<QString, bool> map;

	{
	    QStringList fileList = m_cppSupport->project()->allFiles();
	    QStringList::ConstIterator it = fileList.begin();
	    while( it != fileList.end() ){
		QFileInfo info( *it );
		++it;

		map.insert( info.dirPath(true), true );
	    }
	}

	{
	    QMap<QString, bool>::Iterator it = map.begin();
	    while( it != map.end() ){
		addIncludePath( it.key() );
		++it;
	    }
	}
    }

    // setup the preprocessor
    // code provided by Reginald Stadlbauer <reggie@trolltech.com>
    void setup()
    {
	QString kdedir = getenv( "KDEDIR" );
	if( !kdedir.isNull() )
	    addIncludePath( kdedir + "/include" );

	QString qtdir = getenv( "QTDIR" );
	if( !qtdir.isNull() )
	    addIncludePath( qtdir + "/include" );

	QString qmakespec = getenv( "QMAKESPEC" );
	if ( qmakespec.isNull() )
	    qmakespec = "linux-g++";
	// #### implement other mkspecs and find a better way to find the
	// #### proper mkspec (althoigh this will be no fun :-)

	addIncludePath( qtdir + "/mkspecs/" + qmakespec );
	if ( qmakespec == "linux-g++" ) {
	    addIncludePath( "/include" );
	    addIncludePath( "/usr/include" );
	    addIncludePath( "/ust/local/include" );
#if KDE_VERSION <= 305
	    return; // FIXME!!! Roberto, please review! ;-)
	    // If the QProcess from below is executed,
	    // it somehow breaks the gcc call in parts/outputviews/makewidget.cpp. :-(
	    // It then has the effect that KProcess will never exit, at least on KDE-3.0
#endif // KDE_VERSION
	    QProcess proc;
	    proc.addArgument( "gcc" );
	    proc.addArgument( "-print-file-name=include" );
	    if ( !proc.start() ) {
		qWarning( "Couldn't start gcc" );
		return;
	    }
	    while ( proc.isRunning() )
		usleep( 1 );

	    addIncludePath( proc.readStdout() );
	    addIncludePath( "/usr/include/g++-3" );
	    addIncludePath( "/usr/include/g++" );
	    proc.clearArguments();
	    proc.addArgument( "gcc" );
	    proc.addArgument( "-E" );
	    proc.addArgument( "-dM" );
	    proc.addArgument( "-ansi" );
	    proc.addArgument( "-" );
	    if ( !proc.start() ) {
		qWarning( "Couldn't start gcc" );
		return;
	    }
	    while ( !proc.isRunning() )
		usleep( 1 );
	    proc.closeStdin();
	    while ( proc.isRunning() )
		usleep( 1 );
	    while ( proc.canReadLineStdout() ) {
		QString l = proc.readLineStdout();
		QStringList lst = QStringList::split( ' ', l );
		if ( lst.count() != 3 )
		    continue;
		addMacro( Macro( lst[1], lst[2] ) );
	    }
	    addMacro( Macro( "__cplusplus", "1" ) );
	} else if ( qmakespec == "win32-borland" ) {
	    QString incl = getenv( "INCLUDE" );
	    QStringList includePaths = QStringList::split( ';', incl );
	    QStringList::Iterator it = includePaths.begin();
	    while( it != includePaths.end() ){
		addIncludePath( *it );
		++it;
	    }
	    // ### I am sure there are more standard include paths on
	    // ### windows. I will fix that soon
	    // ### Also do the compiler specific defines on windows
	}
    }

private:
    CppSupportPart* m_cppSupport;
};


class KDevSourceProvider: public SourceProvider
{
public:
    KDevSourceProvider( CppSupportPart* cppSupport ): m_cppSupport( cppSupport ) {}

    virtual QString contents( const QString& fileName )
    {
	//kapp->lock();
	QPtrList<KParts::Part> parts( *m_cppSupport->partController()->parts() );
	QPtrListIterator<KParts::Part> it( parts );
	while( it.current() ){
	    KTextEditor::Document* doc = dynamic_cast<KTextEditor::Document*>( it.current() );
	    ++it;

	    KTextEditor::EditInterface* editIface = dynamic_cast<KTextEditor::EditInterface*>( doc );
	    if( !doc || !editIface || doc->url().path() != fileName )
		continue;

	    QString contents = editIface->text();
	    return QString( contents.unicode(), contents.length() );
	}
	//kapp->unlock();

	QFile f( fileName );
	QTextStream stream( &f );
	if( f.open(IO_ReadOnly) ){
	    QString contents = stream.read();
	    f.close();
	    return contents;
	}

	return QString::null;
    }

    virtual bool isModified( const QString& fileName )
    {
	Q_UNUSED( fileName );
	return true;
    }

private:
    CppSupportPart*  m_cppSupport;

private:
    KDevSourceProvider( const KDevSourceProvider& source );
    void operator = ( const KDevSourceProvider& source );
};

BackgroundParser::BackgroundParser( CppSupportPart* part, QWaitCondition* consumed )
    : m_consumed( consumed ), m_cppSupport( part ), m_close( false )
{
    m_consumed = 0;
    m_driver = new KDevDriver( m_cppSupport );
    m_driver->setSourceProvider( new KDevSourceProvider(m_cppSupport) );
    //disabled for now m_driver->setResolveDependencesEnabled( true );
}

BackgroundParser::~BackgroundParser()
{
    removeAllFiles();
    delete( m_driver );
    m_driver = 0;
}

void BackgroundParser::addFile( const QString& fileName )
{
    m_mutex.lock();
    QString fn( fileName.unicode(), fileName.length() );
    bool added = false;
    if( m_fileList.find(fn) == m_fileList.end() ){
        m_fileList.push_back( fn );
	added = true;
    }
    m_mutex.unlock();

    if( added )
        m_canParse.wakeAll();
}

void BackgroundParser::removeAllFiles()
{
    kdDebug(9007) << "BackgroundParser::removeAllFiles()" << endl;
    m_mutex.lock();

    QMap<QString, Unit*>::Iterator it = m_unitDict.begin();
    while( it != m_unitDict.end() ){
        Unit* unit = it.data();
	++it;
	delete( unit );
    }
    m_unitDict.clear();
    m_driver->reset();
    m_fileList.clear();

    m_mutex.unlock();

    m_isEmpty.wakeAll();
}

void BackgroundParser::removeFile( const QString& fileName )
{
    m_mutex.lock();
    Unit* unit = findUnit( fileName );
    m_unitDict.remove( fileName );
    if( unit ){
        delete( unit );
	unit = 0;
    }
    m_driver->remove( fileName );
    m_mutex.unlock();

    if( m_fileList.isEmpty() )
        m_isEmpty.wakeAll();
}

Unit* BackgroundParser::parseFile( const QString& fileName )
{
    m_driver->remove( fileName );
    m_driver->parseFile( fileName );

    m_driver->parseFile( fileName );
    TranslationUnitAST::Node translationUnit = m_driver->takeTranslationUnit( fileName );

    Unit* unit = new Unit;
    unit->fileName = fileName;
    unit->translationUnit = translationUnit.release();
    unit->problems = m_driver->problems( fileName );

    return unit;
}

Unit* BackgroundParser::findOrCreateUnit( const QString& fileName, bool force )
{
    m_mutex.lock();
    QMap<QString, Unit*>::Iterator it = m_unitDict.find( fileName );
    Unit* unit = it != m_unitDict.end() ? *it : 0;

    if( unit && force ){
        m_unitDict.remove( fileName );
	delete( unit );
	unit = 0;
    }
    m_mutex.unlock();

    if( !unit && 0 != (unit = parseFile(fileName)) ){
        m_mutex.lock();
	m_unitDict.insert( fileName, unit );
        m_mutex.unlock();
    }

    return unit;
}

Unit* BackgroundParser::findUnit( const QString& fileName )
{
    QMap<QString, Unit*>::Iterator it = m_unitDict.find( fileName );
    return it != m_unitDict.end() ? *it : 0;
}

TranslationUnitAST* BackgroundParser::translationUnit( const QString& fileName )
{
    Unit* u = findUnit( fileName );
    return u ? u->translationUnit : 0;
}

QValueList<Problem> BackgroundParser::problems( const QString& fileName )
{
    Unit* u = findUnit( fileName );
    return u ? u->problems : QValueList<Problem>();
}

void BackgroundParser::reparse()
{
}

void BackgroundParser::close()
{
    m_close = true;
}

bool BackgroundParser::filesInQueue()
{
    m_mutex.lock();
    int n = m_fileList.count();
    m_mutex.unlock();
    return n;
}

void BackgroundParser::run()
{
    while( true ){
	if( m_close )
	    QThread::exit();

	while( m_fileList.isEmpty() ){
	    if( m_close )
	        break;

            m_canParse.wait();
	}

	if( m_close )
	    break;

	m_mutex.lock();
	QString fileName = m_fileList.front();
	fileName = QString( fileName.unicode(), fileName.length() );
	m_fileList.pop_front();
	m_mutex.unlock();

	Unit* unit = findOrCreateUnit( fileName, true );

	m_mutex.lock();
	if( unit ){
	    m_unitDict.insert( fileName, unit );
            KApplication::postEvent( m_cppSupport, new FileParsedEvent(fileName) );
	    KApplication::postEvent( m_cppSupport, new FoundProblemsEvent(fileName, unit->problems) );
	    if( m_consumed )
	        m_consumed->wait();

	} else {
	    m_unitDict.remove( fileName );
	}

	m_mutex.unlock();

	//kdDebug(9007) << "!!!!!!!!!!!!!!! PARSED " << fileName << "!!!!!!!!!!!!!!!!!!" << endl;

	if( m_fileList.isEmpty() )
	    m_isEmpty.wakeAll();
    }

    kdDebug(9007) << "!!!!!!!!!!!!!!!!!! BG PARSER DESTROYED !!!!!!!!!!!!" << endl;
}


