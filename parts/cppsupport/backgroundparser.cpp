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

#include <kurl.h>
#include <kdebug.h>
#include <kapplication.h>

#include <qfile.h>
#include <qfileinfo.h>
#include <qtextstream.h>

BackgroundParser::BackgroundParser( CppSupportPart* part, QWaitCondition* consumed )
    : m_consumed( consumed ), m_cppSupport( part ), m_close( false )
{
    m_consumed = 0;
}

BackgroundParser::~BackgroundParser()
{
    removeAllFiles();
}

void BackgroundParser::addFile( const QString& fileName )
{
    m_mutex.lock();
    QString fn( fileName.unicode(), fileName.length() );
    if( m_fileList.find(fn) == m_fileList.end() )
        m_fileList.push_back( fn );
    m_mutex.unlock();

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
    m_driver.reset();
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
    m_driver.clear( fileName );
    m_mutex.unlock();

    if( m_fileList.isEmpty() )
        m_isEmpty.wakeAll();
}

Unit* BackgroundParser::parseFile( const QString& fileName, const QString& contents )
{
    m_driver.removeAllMacrosInFile( fileName );
    TranslationUnitAST::Node translationUnit = m_driver.parseFile( fileName, contents );

    Unit* unit = new Unit;
    unit->fileName = fileName;
    unit->translationUnit = translationUnit.release();
    unit->problems = m_driver.problems( fileName );

    return unit;
}

Unit* BackgroundParser::parseFile( const QString& fileName )
{
    Unit* unit = 0;

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
	unit = parseFile( fileName, contents );
    }
    //kapp->unlock();

    if( !unit ){
	QFile f( fileName );
	QTextStream stream( &f );
	if( f.open(IO_ReadOnly) ){
	    QString contents = stream.read();
	    f.close();
	    unit = parseFile( fileName, contents );
	}
    }

    return unit;
}

Unit* BackgroundParser::findOrCreateUnit( const QString& fileName, bool force )
{
    QMap<QString, Unit*>::Iterator it = m_unitDict.find( fileName );
    Unit* unit = it != m_unitDict.end() ? *it : 0;

    if( unit && force ){
        m_unitDict.remove( fileName );
	delete( unit );
	unit = 0;
    }

    if( !unit && 0 != (unit = parseFile(fileName)) )
	m_unitDict.insert( fileName, unit );

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
	m_fileList.pop_front();

	Unit* unit = findOrCreateUnit( fileName, true );
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

	kdDebug(9007) << "!!!!!!!!!!!!!!! PARSED " << fileName << "!!!!!!!!!!!!!!!!!!" << endl;

	if( m_fileList.isEmpty() )
	    m_isEmpty.wakeAll();
    }

    kdDebug(9007) << "!!!!!!!!!!!!!!!!!! BG PARSER DESTROYED !!!!!!!!!!!!" << endl;
}


