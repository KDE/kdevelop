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
#include "processlinemaker.h"

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
    m_proc->setWorkingDirectory( workdir.toLocalFile() );
    m_procLineMaker = new ProcessLineMaker( m_proc );
    foreach( QString s, env.keys() )
        m_proc->setEnvironment( s, env[s] );
    foreach(QString s, command)
        if( !s.isEmpty() )
            *m_proc << s;

    m_command = command.join(" ");
//     connect( m_proc, SIGNAL(receivedStdout(K3Process* , char*, int) ),
//              this, SLOT( procReadStdout(K3Process* , char*, int) ) );
//     connect( m_proc, SIGNAL(receivedStderr(K3Process* , char*, int) ),
//              this, SLOT( procReadStderr(K3Process* , char*, int) ) );
    connect( m_procLineMaker, SIGNAL(receivedStdoutLine(const QString&)),
             this, SLOT(procReadStdout(const QString&) ));
    connect( m_procLineMaker, SIGNAL(receivedStderrLine(const QString&)),
             this, SLOT(procReadStderr(const QString&) ));
    connect( m_proc, SIGNAL(processExited( K3Process* ) ),
             this, SLOT( procFinished( K3Process* ) ) );
}

OutputViewCommand::~OutputViewCommand()
{
    delete m_proc;
    delete m_procLineMaker;
//     delete m_model; // model is created and deleted in OutputViewPart
    kDebug(9004) << "OutputViewCommand destructor.." << endl;
}

void OutputViewCommand::start()
{
    QStandardItem* i = new QStandardItem( m_command );
    m_model->appendRow( i );
    m_proc->start( K3Process::OwnGroup, K3Process::AllOutput );
}

void OutputViewCommand::setModel( QStandardItemModel *model )
{
    m_model = model;
}

QStandardItemModel* OutputViewCommand::model()
{
    return m_model;
}

QString OutputViewCommand::title()
{
    return m_command.section( ' ', 0, 0 );
}

void OutputViewCommand::procReadStdout(const QString &line)
{
    m_model->appendRow( new QStandardItem( line ) );
}

void OutputViewCommand::procReadStderr(const QString &line)
{
    m_model->appendRow( new QStandardItem( line ) );
}

void OutputViewCommand::procFinished( K3Process* proc )
{
    if( !m_proc->exitStatus() )
    {
        QStandardItem* endItem = new QStandardItem(QString("Finished (%1)").arg(m_proc->exitStatus()) );
        m_model->appendRow( endItem );
        kDebug(9004) << "Finished Sucessfully" << endl;
        QString titlestring = title();
        emit commandFinished( titlestring );
    }
    else
    {
        QStandardItem* endItem = new QStandardItem(QString("Failed (%1)").arg(m_proc->exitStatus()));
        m_model->appendRow( endItem );
        kDebug(9004) << "Failed" << endl;
        QString titlestring = title();
        emit commandFailed( titlestring );
    }
}

#include "outputviewcommand.moc"

//kate: space-indent on; indent-width 4; replace-tabs on; auto-insert-doxygen on; indent-mode cstyle;
