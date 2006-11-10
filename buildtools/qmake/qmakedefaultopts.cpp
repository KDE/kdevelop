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
#include <stdlib.h>
#include <ktempfile.h>
#include <kprocess.h>
#include <qdir.h>
#include <qregexp.h>
#include <qtextstream.h>
#include <kprocess.h>
#include <kprocctrl.h>

QMakeDefaultOpts::QMakeDefaultOpts( QObject* parent, const char* name )
        : QObject(parent, name), makefile(0), qmakefile(0), proc(0)
{

}

void QMakeDefaultOpts::readVariables( const QString& qtdir, const QString& projdir )
{
    makefile = new KTempFile(projdir+"/", ".mf");
    qmakefile = new KTempFile(QString::null, ".pro");
    if ( makefile->status() == 0 && qmakefile->status() == 0  )
    {
        makefile->close();
        qmakefile->close();
        QString qmakebin = qtdir + QString( QChar( QDir::separator() ) ) + "bin" + QString( QChar( QDir::separator() ) ) + "qmake";

//         The following would be needed if we'd use KProcess::Block, because KProcessController installs its signal handlers only once
//         unsigned int refs = 0;
//         while( KProcessController::theKProcessController )
//         {
//             KProcessController::deref();
//             refs++;
//         }
//         kdDebug(9024) << "KProcCtrl has:" << refs << endl;
//         while( refs > 0 )
//         {
//             KProcessController::ref();
//             refs--;
//         }
        proc = new KProcess();
        proc->setUseShell( true );
        *proc <<  "cd " << KProcess::quote( projdir ) << "&&" << KProcess::quote( qmakebin );
        *proc <<  "-d" << "-o" << KProcess::quote( makefile->name() );
        *proc <<  KProcess::quote( qmakefile->name() );
        kdDebug(9024) << "Executing:" << proc->args() << endl;
        connect( proc, SIGNAL( processExited( KProcess* ) ), this, SLOT( slotFinished( KProcess* ) ) );
        connect( proc, SIGNAL( receivedStderr( KProcess*, char*, int ) ),
                 this, SLOT( slotReadStderr( KProcess*, char*, int ) ) );
        proc->start( KProcess::NotifyOnExit, KProcess::Stderr );
    }
}

QMakeDefaultOpts::~QMakeDefaultOpts()
{
    m_variables.clear();
}

void QMakeDefaultOpts::slotReadStderr(KProcess*, char* buf, int buflen)
{
    QString buffer = QString::fromLocal8Bit( buf, buflen );
    QStringList lines = QStringList::split( "\n", buffer );
    for ( QStringList::const_iterator it = lines.begin(); it != lines.end(); ++it)
    {
        QString line = *it;
        QRegExp re( "DEBUG 1: ([^ =:]+) === (.*)" );
        if ( re.exactMatch( line ) )
        {
            QString var = re.cap( 1 );
            QStringList values = QStringList::split( " :: ", re.cap( 2 ) );
            m_variables[var] = values;
        }
    }
}

void QMakeDefaultOpts::slotFinished(KProcess*)
{
    makefile->unlink();
    qmakefile->unlink();
    proc->closeAll();
    delete proc;
    proc = 0;
    emit variablesRead();
}

QStringList QMakeDefaultOpts::variableValues( const QString& var ) const
{
    QStringList result;
    if ( m_variables.contains(var) )
        result = m_variables[var];
    return result;
}

QStringList QMakeDefaultOpts::variables() const
{
    return m_variables.keys();
}

#include "qmakedefaultopts.moc"

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
