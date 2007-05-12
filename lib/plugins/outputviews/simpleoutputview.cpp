/* KDevelop Simple OutputView
 *
 * Copyright 2006 Andreas Pakulat <apaku@gmx.de>
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

#include "simpleoutputview.h"
#include "outputwidget.h"
#include "outputviewcommand.h"

#include <QtCore/QStringList>

#include <QtDesigner/QExtensionFactory>
#include <QtGui/QListView>
#include <QtCore/QTimer>
#include <QtGui/QStandardItemModel>
#include <QtCore/QQueue>

#include <icore.h>
#include <iuicontroller.h>

#include <kgenericfactory.h>
#include <k3process.h>
#include <kdialog.h>
#include <kglobal.h>
#include <klocale.h>
#include <kdebug.h>

typedef KGenericFactory<SimpleOutputView> SimpleOutputViewFactory ;
K_EXPORT_COMPONENT_FACTORY( kdevsimpleoutputview,
                            SimpleOutputViewFactory( "kdevsimpleoutputview" ) )

class SimpleOutputViewViewFactory : public KDevelop::IToolViewFactory{
public:
    SimpleOutputViewViewFactory(SimpleOutputView *part): m_part(part) {}
    virtual QWidget* create(QWidget *parent = 0)
    {
        Q_UNUSED(parent)
        OutputWidget* l = new OutputWidget( parent, m_part);
        return l;
    }
    virtual Qt::DockWidgetArea defaultPosition(const QString &/*areaName*/)
    {
        return Qt::BottomDockWidgetArea;
    }
private:
    SimpleOutputView *m_part;
};

class SimpleOutputViewPrivate
{
public:
    SimpleOutputViewViewFactory* m_factory;
    QMap<QString, QStandardItemModel* > m_models;
    QMap<QString, QString> m_titles;
    QStringList m_ids;
    QMap<QString, QQueue<OutputViewCommand*> > m_jobs;
};

SimpleOutputView::SimpleOutputView(QObject *parent, const QStringList &)
    : KDevelop::IPlugin(SimpleOutputViewFactory::componentData(), parent),
      d(new SimpleOutputViewPrivate)
{
    KDEV_USE_EXTENSION_INTERFACE( KDevelop::IOutputView )
    d->m_factory = new SimpleOutputViewViewFactory( this );
    core()->uiController()->addToolView( "Output View", d->m_factory );

    connect( this, SIGNAL(commandFinished( const QString& )),
             this, SLOT(slotCommandFinished(const QString&)) );
    connect( this, SIGNAL(commandFailed( const QString& )),
             this, SLOT(slotCommandFailed(const QString&)) );
}

SimpleOutputView::~SimpleOutputView()
{
    foreach( QStandardItemModel* m, d->m_models )
        delete m;
    foreach( QQueue< OutputViewCommand* > queue, d->m_jobs )
    {
        foreach( OutputViewCommand* cmd, queue )
        {
            delete cmd;
        }
    }
    delete d;
}

void SimpleOutputView::queueCommand(const KUrl& dir, const QStringList& command, const QMap<QString, QString>& env )
{
    if( command.isEmpty() )
        return;
    kDebug(9004) << "Queueing Command: " << dir << "|" << command << endl;
    QString title = command.first();
    if( !d->m_jobs.contains(title) )
    {
        // Models will be created in the same place with QListView. not here.
//         QStandardItemModel* model = new QStandardItemModel();
        OutputViewCommand* cmd = new OutputViewCommand( dir, command, env, NULL );
        connect( cmd, SIGNAL( commandFinished( const QString& ) ),
                 this, SIGNAL( commandFinished( const QString& ) ) );
        connect( cmd, SIGNAL( commandFailed( const QString& ) ),
                 this, SIGNAL( commandFailed( const QString& ) ) );

        QQueue< OutputViewCommand* > cmdQueue;
        cmdQueue.enqueue( cmd );
        d->m_jobs[title] = cmdQueue;
        emit commandAdded( cmd );
        d->m_jobs[title].head()->start();
        kDebug(9004) << "Adding and start now " << endl;
    }
    else
    {
        // Append m_job. When the job named "title" is finished, completed job will be removed
        // and this new job will get started by slotCommandFinished/Failed()
//         QStandardItemModel* model = new QStandardItemModel();
        OutputViewCommand* cmd = new OutputViewCommand( dir, command, env, NULL );
        d->m_jobs[title].enqueue( cmd );
        kDebug(9004) << "Adding and pending" << endl;
    }
}

void SimpleOutputView::registerLogView( const QString& id, const QString& title )
{
    if( !d->m_ids.contains( id ) && !d->m_models.contains(id) && !d->m_titles.contains(id) )
    {
        d->m_ids << id;
        d->m_titles[id] = title;
        d->m_models[id] = new QStandardItemModel(this);
        emit modelAdded( d->m_titles[id], d->m_models[id] );
    }
}

void SimpleOutputView::appendLine( const QString& id, const QString& line )
{
    if( d->m_models.contains( id ) )
    {
        d->m_models[id]->appendRow( new QStandardItem( line ) );
    }
}

void SimpleOutputView::appendLines( const QString& id, const QStringList& lines )
{
    if( d->m_models.contains( id ) )
    {
        foreach( QString line, lines )
            d->m_models[id]->appendRow( new QStandardItem( line ) );
    }
}

QStringList SimpleOutputView::registeredViews()
{
    return d->m_ids;
}

QStandardItemModel* SimpleOutputView::registeredModel( const QString& id )
{
    if( d->m_models.contains( id ) )
    {
        return d->m_models[id];
    }
    return 0;
}

QString SimpleOutputView::registeredTitle( const QString& id )
{
    if( d->m_titles.contains( id ) )
    {
        return d->m_titles[id];
    }
    return QString();
}

void SimpleOutputView::slotCommandFinished( const QString& id )
{
    cleanupTerminatedJobs( id );
    startNextPendingJob( id );    
}

void SimpleOutputView::slotCommandFailed( const QString& id )
{
    cleanupTerminatedJobs( id );
    startNextPendingJob( id );
}

void SimpleOutputView::cleanupTerminatedJobs( const QString& id )
{
    Q_ASSERT( d->m_jobs.contains(id) );
    
    QQueue<OutputViewCommand*> &cmdQ = d->m_jobs[id];
    Q_ASSERT( cmdQ.isEmpty() == FALSE );
    OutputViewCommand *cmd = cmdQ.dequeue();
    cmd->deleteLater();
    kDebug(9004) << "OutputViewCommand removed and deleteLater()ed " << (long)cmd << endl;

    if( cmdQ.isEmpty() )
    {
        d->m_jobs.remove( id );
    }
}

void SimpleOutputView::startNextPendingJob( const QString &id )
{
    if( d->m_jobs.contains(id) && !(d->m_jobs[id].isEmpty()) )
    {
        // execute next pending job, whose title was the same with justly finished job.
        QQueue< OutputViewCommand* > &cmdQ = d->m_jobs[id];
        OutputViewCommand *nextCmd = cmdQ.head();
        
        connect( nextCmd, SIGNAL( commandFinished( const QString& ) ),
                 this, SIGNAL( commandFinished( const QString& ) ) );
        connect( nextCmd, SIGNAL( commandFailed( const QString& ) ),
                 this, SIGNAL( commandFailed( const QString& ) ) );

        emit commandAdded( nextCmd );
        nextCmd->start();
        kDebug(9004) << "Started next pended job " << (long)nextCmd << endl;
    }
}

#include "simpleoutputview.moc"
// kate: space-indent on; indent-width 4; tab-width: 4; replace-tabs on; auto-insert-doxygen on
