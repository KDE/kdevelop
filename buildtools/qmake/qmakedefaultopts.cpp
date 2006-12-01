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
#include <qdir.h>
#include <qregexp.h>
#include <kprocess.h>
#include <kmessagebox.h>
#include <klocale.h>

QMakeDefaultOpts::QMakeDefaultOpts( QObject* parent, const char* name )
        : QObject(parent, name), makefile(0), qmakefile(0), proc(0)
{

}

void QMakeDefaultOpts::readVariables( const QString& qmake, const QString& projdir )
{
    makefile = new KTempFile(projdir+"/", ".mf");
    qmakefile = new KTempFile(projdir+"/", ".pro");
    if ( makefile->status() == 0 )
    {
        makefile->close();
        qmakefile->close();

        proc = new KProcess();
        kdDebug(9024) << "KProc Working dir:" << projdir << endl;
        proc->setWorkingDirectory( projdir );
        *proc << qmake;
        *proc << "-d";
        *proc << "-o";
        *proc << makefile->name();
        *proc << qmakefile->name();
        kdDebug(9024) << "Executing:" << proc->args() << endl;
        connect( proc, SIGNAL( processExited( KProcess* ) ), this, SLOT( slotFinished( KProcess* ) ) );
        connect( proc, SIGNAL( receivedStderr( KProcess*, char*, int ) ),
                 this, SLOT( slotReadStderr( KProcess*, char*, int ) ) );
//         proc->setCommunication( QProcess::Stderr );
        proc->start( KProcess::NotifyOnExit, KProcess::Stderr );
        if( !proc->isRunning() && !proc->normalExit() )
        {
            kdDebug(9024) << "Couldn't execute qmake: " << proc->args() << endl;
//             kdDebug(9024) << "message box" << endl;
//             KMessageBox::error( 0, i18n("Error running QMake.\nTried to execute:\n%1\n\nPlease check the path to Qt under Project Options->C++ Support->Qt tab.").arg(proc->arguments().join(" ")), i18n("Couldn't execute qmake") );
            makefile->unlink();
            delete makefile;
            makefile = 0;
            qmakefile->unlink();
            delete qmakefile;
            qmakefile = 0;
            delete proc;
            proc = 0;
            m_variables.clear();
            m_keys.clear();
            emit variablesRead();
        }
    }
}

QMakeDefaultOpts::~QMakeDefaultOpts()
{
    m_variables.clear();
    delete proc;
    proc = 0;
    delete makefile;
    makefile = 0;
    delete qmakefile;
    qmakefile = 0;
}

void QMakeDefaultOpts::slotReadStderr( KProcess* , char* buf, int len )
{
    QString buffer = QString::fromLocal8Bit( buf, len );
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

void QMakeDefaultOpts::slotFinished( KProcess* )
{
    kdDebug(9024) << "Proc finished" << endl;
    makefile->unlink();
    delete makefile;
    makefile = 0;
    qmakefile->unlink();
    delete qmakefile;
    qmakefile = 0;
    delete proc;
    proc = 0;
    m_keys = m_variables.keys();
    emit variablesRead();
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

#include "qmakedefaultopts.moc"

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
