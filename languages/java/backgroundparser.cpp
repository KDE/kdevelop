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
#include "javasupportpart.h"
#include "javasupport_events.h"
#include "driver.h"
#include "kdevdeepcopy.h"
#include "kdevdriver.h"

#if QT_VERSION < 0x030100
#include <kdevmutex.h>
#else
#include <qmutex.h>
//Added by qt3to4:
#include <Q3ValueList>
#include <Q3PtrList>
#endif

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
#include <q3process.h>

class KDevSourceProvider: public SourceProvider
{
public:
    KDevSourceProvider( JavaSupportPart* javaSupport )
        : m_javaSupport( javaSupport ),
          m_readFromDisk( false ) {}

    void setReadFromDisk( bool b ) { m_readFromDisk = b; }
    bool readFromDisk() const { return m_readFromDisk; }

    virtual QString contents( const QString& fileName )
    {
	if( !m_readFromDisk ){
	    //kdDebug(9013) << "-------> kapp is locked = " << kapp->locked() << endl;
	    bool needToLock = kapp->locked() == false;

	    if( needToLock )
		kapp->lock();

	    //kdDebug(9013) << "-------> kapp locked" << endl;

	    Q3PtrList<KParts::Part> parts( *m_javaSupport->partController()->parts() );
	    Q3PtrListIterator<KParts::Part> it( parts );
	    while( it.current() ){
		KTextEditor::Document* doc = dynamic_cast<KTextEditor::Document*>( it.current() );
		++it;

		KTextEditor::EditInterface* editIface = dynamic_cast<KTextEditor::EditInterface*>( doc );
		if( !doc || !editIface || doc->url().path() != fileName )
		    continue;

		QString contents = QString( editIface->text().ascii() ); // deep copy

		if( needToLock )
		    kapp->unlock();

		//kdDebug(9013) << "-------> kapp unlocked" << endl;

		return contents;
	    }

	    if( needToLock )
		kapp->unlock();
	    //kdDebug(9013) << "-------> kapp unlocked" << endl;
	}

	QFile f( fileName );
	QTextStream stream( &f );
	if( f.open(QIODevice::ReadOnly) ){
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
    JavaSupportPart*  m_javaSupport;
    bool m_readFromDisk;
private:
    KDevSourceProvider( const KDevSourceProvider& source );
    void operator = ( const KDevSourceProvider& source );
};

class SynchronizedFileList
{
public:
    SynchronizedFileList() {}

    bool isEmpty() const
    {
	QMutexLocker locker( &m_mutex );
	return m_fileList.isEmpty();
    }

    uint count() const
    {
	QMutexLocker locker( &m_mutex );
	return m_fileList.count();
    }

    QPair<QString, bool> front() const
    {
	QMutexLocker locker( &m_mutex );
	return m_fileList.front();
    }

    void clear()
    {
	QMutexLocker locker( &m_mutex );
	m_fileList.clear();
    }

    void push_back( const QString& fileName, bool readFromDisk=false )
    {
	QMutexLocker locker( &m_mutex );
	m_fileList.append( qMakePair(fileName, readFromDisk) ); /// \FIXME ROBE deepcopy?!
    }

    void pop_front()
    {
	QMutexLocker locker( &m_mutex );
	m_fileList.pop_front();
    }

    bool contains( const QString& fileName ) const
    {
	QMutexLocker locker( &m_mutex );
	Q3ValueList< QPair<QString, bool> >::ConstIterator it = m_fileList.begin();
	while( it != m_fileList.end() ){
	    if( (*it).first == fileName )
		return true;
	    ++it;
	}
	return false;
    }

    void remove( const QString& fileName )
    {
	QMutexLocker locker( &m_mutex );
	Q3ValueList< QPair<QString, bool> >::Iterator it = m_fileList.begin();
	while( it != m_fileList.end() ){
	    if( (*it).first == fileName )
		m_fileList.remove( it );
	    ++it;
	}
    }

private:
    mutable QMutex m_mutex;
    Q3ValueList< QPair<QString, bool> > m_fileList;
};

BackgroundParser::BackgroundParser( JavaSupportPart* part, QWaitCondition* consumed )
    : m_consumed( consumed ), m_javaSupport( part ), m_close( false )
{
    m_fileList = new SynchronizedFileList();
    m_driver = new KDevDriver( m_javaSupport );
    m_driver->setSourceProvider( new KDevSourceProvider(m_javaSupport) );
    //disabled for now m_driver->setResolveDependencesEnabled( true );
}

BackgroundParser::~BackgroundParser()
{
    removeAllFiles();

    delete( m_driver );
    m_driver = 0;

    delete m_fileList;
    m_fileList = 0;
}

void BackgroundParser::addFile( const QString& fileName, bool readFromDisk )
{
    QString fn = deepCopy( fileName );

    bool added = false;
    if( !m_fileList->contains(fn) ){
        m_fileList->push_back( fn, readFromDisk );
	added = true;
    }

    if( added )
        m_canParse.wakeAll();
}

void BackgroundParser::removeAllFiles()
{
    kdDebug(9013) << "BackgroundParser::removeAllFiles()" << endl;
    QMutexLocker locker( &m_mutex );

    QMap<QString, Unit*>::Iterator it = m_unitDict.begin();
    while( it != m_unitDict.end() ){
        Unit* unit = it.data();
	++it;
	delete( unit );
	unit = 0;
    }
    m_unitDict.clear();
    m_driver->reset();
    m_fileList->clear();

    m_isEmpty.wakeAll();
}

void BackgroundParser::removeFile( const QString& fileName )
{
    QMutexLocker locker( &m_mutex );

    if( Unit* unit = findUnit(fileName) ){
        m_driver->remove( fileName );
        m_unitDict.remove( fileName );
        delete( unit );
	unit = 0;
    }

    if( m_fileList->isEmpty() )
        m_isEmpty.wakeAll();
}

Unit* BackgroundParser::parseFile( const QString& fileName, bool readFromDisk )
{
    static_cast<KDevSourceProvider*>( m_driver->sourceProvider() )->setReadFromDisk( readFromDisk );

    m_driver->remove( fileName );
    m_driver->parseFile( fileName );
    RefJavaAST translationUnit = m_driver->takeTranslationUnit( fileName );

    Unit* unit = new Unit;
    unit->fileName = fileName;
    unit->translationUnit = translationUnit;
    unit->problems = m_driver->problems( fileName );

    static_cast<KDevSourceProvider*>( m_driver->sourceProvider() )->setReadFromDisk( false );

    if( m_unitDict.find(fileName) != m_unitDict.end() ){
	Unit* u = m_unitDict[ fileName ];
	m_unitDict.remove( fileName );
	delete( u );
	u = 0;
    }

    m_unitDict.insert( fileName, unit );

    if( m_fileList->contains(fileName) ){
        kdDebug(9013) << "========================> FILE: " << fileName << " IN QUEUE <=============" << endl;
    } else {
        KApplication::postEvent( m_javaSupport, new FileParsedEvent(fileName, unit->problems) );
    }

    m_currentFile = QString::null;

    if( m_fileList->isEmpty() )
	m_isEmpty.wakeAll();

    return unit;
}

Unit* BackgroundParser::findUnit( const QString& fileName )
{
    QMap<QString, Unit*>::Iterator it = m_unitDict.find( fileName );
    return it != m_unitDict.end() ? *it : 0;
}

RefJavaAST BackgroundParser::translationUnit( const QString& fileName )
{
    Unit* u = 0;
    if( (u = findUnit(fileName)) == 0 ){
	m_fileList->remove( fileName );
	u = parseFile( fileName, false );
    }

    return u->translationUnit;
}

Q3ValueList<Problem> BackgroundParser::problems( const QString& fileName )
{
    Unit* u = 0;
    if( (u = findUnit(fileName)) == 0 ){
	m_fileList->remove( fileName );
	u = parseFile( fileName, false );
    }

    return u ? u->problems : Q3ValueList<Problem>();
}

void BackgroundParser::close()
{
    QMutexLocker locker( &m_mutex );
    m_close = true;
    m_canParse.wakeAll();
}

bool BackgroundParser::filesInQueue()
{
    QMutexLocker locker( &m_mutex );

    return m_fileList->count() || !m_currentFile.isEmpty();
}

void BackgroundParser::run()
{
    // (void) m_javaSupport->codeCompletion()->repository()->getEntriesInScope( QStringList(), false );

    while( !m_close ){

        m_mutex.lock();
	while( m_fileList->isEmpty() ){
            m_canParse.wait( &m_mutex );

            if( m_close ){
                break;
            }
        }

        if( m_close ){
            m_mutex.unlock();
            break;
        }

	QPair<QString, bool> entry = m_fileList->front();
        QString fileName = entry.first;
	bool readFromDisk = entry.second;
	m_currentFile = fileName;
	m_fileList->pop_front();

	(void) parseFile( fileName, readFromDisk );
        m_mutex.unlock();
    }

    kdDebug(9013) << "!!!!!!!!!!!!!!!!!! BG PARSER DESTROYED !!!!!!!!!!!!" << endl;

    QThread::exit();
}
