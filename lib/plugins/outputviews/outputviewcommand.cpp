/* This file is part of KDevelop
 *
 * Copyright 2007 Andreas Pakulat <apaku@gmx.de>
 * Copyright 2007 Dukju Ahn <dukjuahn@gmail.com>
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
#include "ioutputviewitem.h"
#include "ioutputviewitemfactory.h"

#include <QtCore/QMap>
#include <QtGui/QStandardItemModel>
#include <QtCore/QList>
#include <QAction>

#include <kurl.h>
#include <kdebug.h>

OutputViewCommand::OutputViewCommand( const KUrl& workdir, const QStringList& command,
                                      const QMap<QString, QString>& env,
                                      QStandardItemModel* model,
                                      IOutputViewItemFactory *itemFactory )
    : QObject(0), m_proc(0), m_model(model)
{
    m_proc = new QProcess();
    m_proc->setWorkingDirectory( workdir.toLocalFile() );
    m_procLineMaker = new ProcessLineMaker( m_proc );

    if( itemFactory )
    {
        m_factory = itemFactory;
    }
    else
    {
        m_factory = new StandardOutputViewItemFactory<IOutputViewItem>();
    }

    QMap<QString,QString> sysenv = buildEnvMap(QProcess::systemEnvironment());

    foreach( QString s, env.keys() )
        sysenv[s] = env[s];
    m_proc->setEnvironment( buildEnvList( sysenv ) );

    m_command = command.join(" ");
    connect( m_procLineMaker, SIGNAL(receivedStdoutLines(const QStringList&)),
             this, SLOT(procReadStdout(const QStringList&) ));
    connect( m_procLineMaker, SIGNAL(receivedStderrLines(const QStringList&)),
             this, SLOT(procReadStderr(const QStringList&) ));
    connect( m_proc, SIGNAL(finished( int, QProcess::ExitStatus ) ),
             this, SLOT( procFinished( int, QProcess::ExitStatus  ) ) );
}

OutputViewCommand::~OutputViewCommand()
{
    delete m_proc;
    delete m_procLineMaker;
//     delete m_model; // model is created and deleted in OutputViewPart
    delete m_factory;
    kDebug(9004) << "OutputViewCommand destructor.." << endl;
}

QMap<QString,QString> OutputViewCommand::buildEnvMap( const QStringList& list )
{
    QMap<QString,QString> env;
    foreach( QString s, list )
    {
        int index = s.indexOf("=");
        env[s.left(index)] = s.right(index+1);
    }
    return env;
}

QStringList OutputViewCommand::buildEnvList( const QMap<QString,QString>& map )
{
    QStringList env;
    foreach( QString s, map.keys() )
    {
        env.append(s+"="+map[s]);
    }
    return env;
}

void OutputViewCommand::start()
{
    IOutputViewItem *i = m_factory->createItem( m_command );
    m_model->appendRow( i );
    m_proc->start( m_command );
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

void OutputViewCommand::procReadStdout(const QStringList &lineList)
{
    Q_FOREACH( QString line, lineList )
        m_model->appendRow( m_factory->createItem( line ) );
}

void OutputViewCommand::procReadStderr(const QStringList &lineList)
{
    Q_FOREACH( QString line, lineList )
        m_model->appendRow( m_factory->createItem( line ) );
}

void OutputViewCommand::procFinished( int exitstatus, QProcess::ExitStatus status )
{
    if( !exitstatus )
    {
        IOutputViewItem* endItem = m_factory->createItem(QString("Finished (%1)").arg(exitstatus));
        m_model->appendRow( endItem );
        kDebug(9004) << "Finished Sucessfully" << endl;
        QString titlestring = title();
        emit commandFinished( titlestring );
    }
    else
    {
        IOutputViewItem* endItem = m_factory->createItem(QString("Failed (%1)").arg(exitstatus));
        m_model->appendRow( endItem );
        kDebug(9004) << "Failed" << endl;
        QString titlestring = title();
        emit commandFailed( titlestring );
    }
}

#include "outputviewcommand.moc"

//kate: space-indent on; indent-width 4; replace-tabs on; auto-insert-doxygen on; indent-mode cstyle;
