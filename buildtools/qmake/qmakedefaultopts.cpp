/***************************************************************************
*   Copyright (C) 2006 by Andreas Pakulat                                 *
*   apaku@gmx.de                                                          *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "qmakedefaultopts.h"

#include <kdebug.h>
#include <ktempfile.h>
#include <qregexp.h>
#include <kprocess.h>

#include <blockingkprocess.h>

QMakeDefaultOpts::QMakeDefaultOpts()
{

}

void QMakeDefaultOpts::readVariables( const QString& qmake, const QString& projdir )
{
    KTempFile makefile (projdir+"/", ".mf");
    KTempFile qmakefile(projdir+"/", ".pro");
    if ( makefile.status() == 0 && qmakefile.status() == 0 )
    {
        makefile.close();
        qmakefile.close();

        BlockingKProcess proc;
        kdDebug(9024) << "KProc Working dir:" << projdir << endl;
        proc.setWorkingDirectory( projdir );
        proc << qmake;
        proc << "-d";
        proc << "-o";
        proc << makefile.name();
        proc << qmakefile.name();
        kdDebug(9024) << "Executing:" << proc.args() << endl;
        proc.start( KProcess::NotifyOnExit, KProcess::Stderr );
        if( !proc.isRunning() && !proc.normalExit() )
        {
            kdDebug(9024) << "Couldn't execute qmake: " << proc.args() << endl;
            makefile.unlink();
            qmakefile.unlink();
            m_variables.clear();
            m_keys.clear();
        }else
        {
            makefile.unlink();
            qmakefile.unlink();
            QStringList lines = QStringList::split( "\n", proc.stdErr() );
            for ( QStringList::const_iterator it = lines.begin(); it != lines.end(); ++it)
            {
                QString line = *it;
                QRegExp re( "DEBUG 1: ([^ =:]+) === (.*)" );
                if ( re.exactMatch( line ) )
                {
                    QString var = re.cap( 1 );
                    QStringList values = QStringList::split( " :: ", re.cap( 2 ) );
                    m_variables[var] = values;
                    m_keys.append( var );
                }
            }
        }
    }
}

QMakeDefaultOpts::~QMakeDefaultOpts()
{
}

const QStringList QMakeDefaultOpts::variableValues( const QString& var ) const
{
//     QStringList result;
    if ( m_variables.contains(var) )
        return m_variables[var];
    return QStringList();
}

const QStringList& QMakeDefaultOpts::variables() const
{
    return m_keys;
}

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
