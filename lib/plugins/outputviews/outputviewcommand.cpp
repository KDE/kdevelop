/* This file is part of KDevelop
 *
 * Copyright 2007 Andreas Pakulat <apaku@gmx.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#include "outputviewcommand.h"

#include <QtCore/QMap>
#include <QtGui/QStandardItemModel>

#include <kurl.h>
#include <k3process.h>
#include <kdebug.h>

OutputViewCommand::OutputViewCommand( const KUrl& workdir, const QStringList& command,
                                      const QMap<QString, QString>& env,
                                      QStandardItemModel* model )
    : QObject(0), m_proc(0), m_model(model)
{
    m_proc = new K3Process();
    m_proc->setWorkingDirectory( workdir.path() );
    foreach( QString s, env.keys() )
        m_proc->setEnvironment( s, env[s] );
    foreach(QString s, command)
        if( !s.isEmpty() )
            *m_proc << s;

    m_command = command.join(" ");
    connect( m_proc, SIGNAL(receivedStdout(K3Process* , char*, int) ),
             this, SLOT( procReadStdout(K3Process* , char*, int) ) );
    connect( m_proc, SIGNAL(receivedStderr(K3Process* , char*, int) ),
             this, SLOT( procReadStderr(K3Process* , char*, int) ) );
    connect( m_proc, SIGNAL(processExited( K3Process* ) ),
             this, SLOT( procFinished( K3Process* ) ) );
}

OutputViewCommand::~OutputViewCommand()
{
    delete m_proc;
    delete m_model;
}

void OutputViewCommand::start()
{
    QStandardItem* i = new QStandardItem( m_command );
    m_model->appendRow( i );
    m_proc->start( K3Process::OwnGroup, K3Process::AllOutput );
}

void OutputViewCommand::procReadStdout(K3Process* proc, char* buf, int len)
{
    Q_UNUSED(proc);
    QString txt = QString::fromLocal8Bit( buf, len );
    QStringList l = txt.split("\n");
    foreach( QString s, l )
    {
        m_model->appendRow( new QStandardItem( s ) );
    }
}

void OutputViewCommand::procReadStderr(K3Process* proc, char* buf, int len)
{
    Q_UNUSED(proc);
    QString txt = QString::fromLocal8Bit( buf, len );
    QStringList l = txt.split("\n");
    foreach( QString s, l )
    {
        m_model->appendRow( new QStandardItem( s ) );
    }
}

void OutputViewCommand::procFinished( K3Process* proc )
{
    if( !m_proc->exitStatus() )
    {
        QStandardItem* endItem = new QStandardItem(QString("Finished (%1)").arg(m_proc->exitStatus()) );
        m_model->appendRow( endItem );
        kDebug(9004) << "Finished Sucessfully" << endl;
        emit commandFinished( m_command );
    }
    else
    {
        QStandardItem* endItem = new QStandardItem(QString("Failed (%1)").arg(m_proc->exitStatus()));
        m_model->appendRow( endItem );
        kDebug(9004) << "Failed" << endl;
        emit commandFailed( m_command );
    }
}

#include "outputviewcommand.moc"

//kate: space-indent on; indent-width 4; replace-tabs on; auto-insert-doxygen on; indent-mode cstyle;
