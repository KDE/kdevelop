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

#ifndef BACKGROUNDPARSER_H
#define BACKGROUNDPARSER_H

#include "driver.h"
#include "ast.h"

#include <qthread.h>
#include <qwaitcondition.h>
#include <qmutex.h>
#include <qasciidict.h>
#include <kdebug.h>

class CppSupportPart;
class TranslationUnitAST;

class Unit    
{
public:
    Unit(): translationUnit( 0 ) { kdDebug(9007) << "++ Unit()" << endl; }
    ~Unit() { kdDebug(9007) << "-- Unit()" << endl; delete translationUnit; }
    
    QString fileName;
    QValueList<Problem> problems;
    TranslationUnitAST* translationUnit;
    
protected:
    Unit( const Unit& source );
    void operator = ( const Unit& source );
};

class BackgroundParser: public QThread
{
public:
    BackgroundParser( CppSupportPart* );
    virtual ~BackgroundParser();
    
    void removeAllFiles();
    void removeFile( const QString& fileName );
    
    TranslationUnitAST* translationUnit( const QString& fileName );
    QValueList<Problem> problems( const QString& fileName );
    
    void close();
    void reparse();
    
    void lock();
    void unlock();
    
    virtual void run();
    
protected:
    Unit* findOrCreateUnit( const QString& fileName );
    Unit* parseFile( const QString& fileName );
    Unit* parseFile( const QString& fileName, const QString& contents );    
    
private:
    Driver m_driver;
    QWaitCondition m_changed;
    QMutex m_mutex;
    
    CppSupportPart* m_cppSupport;
    bool m_close;
    QAsciiDict<Unit> m_unitDict;
};


inline void BackgroundParser::lock()
{
    m_mutex.lock();
}

inline void BackgroundParser::unlock()
{
    m_mutex.unlock();
}

#endif
