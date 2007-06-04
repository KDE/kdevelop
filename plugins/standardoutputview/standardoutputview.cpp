/* KDevelop Standard OutputView
 *
 * Copyright 2006-2007 Andreas Pakulat <apaku@gmx.de>
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

#include "standardoutputview.h"
#include "outputwidget.h"
#include "outputviewcommand.h"
#include "ioutputviewitemfactory.h"

#include <QtCore/QStringList>

#include <QtDesigner/QExtensionFactory>
#include <QtGui/QListView>
#include <QtCore/QTimer>
#include <QtGui/QStandardItemModel>
#include <QtCore/QQueue>

#include <QtGui/QAction>
#include <QtGui/QKeySequence>
#include <icore.h>
#include <iuicontroller.h>

#include <kgenericfactory.h>
#include <k3process.h>
#include <kdialog.h>
#include <kglobal.h>
#include <klocale.h>
#include <kdebug.h>
#include <kactioncollection.h>

typedef KGenericFactory<StandardOutputView> StandardOutputViewFactory ;
K_EXPORT_COMPONENT_FACTORY( kdevstandardoutputview,
                            StandardOutputViewFactory( "kdevstandardoutputview" ) )

class StandardOutputViewViewFactory : public KDevelop::IToolViewFactory{
public:
    StandardOutputViewViewFactory(StandardOutputView *part): m_part(part) {}
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
    StandardOutputView *m_part;
};

class StandardOutputViewPrivate
{
public:
    StandardOutputViewViewFactory* m_factory;
    QMap<QString, QStandardItemModel* > m_models;
    QMap<QString, QString> m_titles;
    QStringList m_ids;
    QMap<QString, QQueue<OutputViewCommand*> > m_jobs;
    // command model can live longer than command itself.
    QMap<QString, QStandardItemModel* > m_cmdModels;
};

StandardOutputView::StandardOutputView(QObject *parent, const QStringList &)
    : KDevelop::IPlugin(StandardOutputViewFactory::componentData(), parent),
      d(new StandardOutputViewPrivate)
{
    KDEV_USE_EXTENSION_INTERFACE( KDevelop::IOutputView )
    d->m_factory = new StandardOutputViewViewFactory( this );
    core()->uiController()->addToolView( "Output View", d->m_factory );

    connect( this, SIGNAL(commandFinished( const QString& )),
             this, SLOT(slotCommandFinished(const QString&)) );
    connect( this, SIGNAL(commandFailed( const QString& )),
             this, SLOT(slotCommandFailed(const QString&)) );

    setXMLFile("kdevstandardoutputview.rc");

    // setup actions
    QAction *action;

    action = actionCollection()->addAction("next_error");
    // TODO more general namechoose other than "next error"
    // Not all messages user is interested are error.
    action->setText("Next Error");
    action->setShortcut( QKeySequence(Qt::Key_F4) );
    connect(action, SIGNAL(triggered(bool)), this, SIGNAL(searchNextError()));

    action = actionCollection()->addAction("prev_error");
    action->setText("Previous Error");
    action->setShortcut( QKeySequence(Qt::SHIFT | Qt::Key_F4) );
    connect(action, SIGNAL(triggered(bool)), this, SIGNAL(searchPrevError()));
}

StandardOutputView::~StandardOutputView()
{
    foreach( QStandardItemModel* m, d->m_models )
        delete m;
    foreach( QStandardItemModel* m, d->m_cmdModels )
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

void StandardOutputView::queueCommand(const KUrl& dir, const QStringList& command,
                                    const QMap<QString, QString>& env,
                                    IOutputViewItemFactory *factory)
{
    if( command.isEmpty() )
        return;
    kDebug(9004) << "Queueing Command: " << dir << "|" << command << endl;
    QString title = command.first();
    // todo: when all the outputviews using this model are closed by user, delete this model
    // maybe use KSharedPtr or something..
    OutputViewCommand* cmd = new OutputViewCommand( dir, command, env, 0, factory );
    if( !d->m_jobs.contains(title) )
    {
        // set model into command. Model lives longer than command, so although the command
        // doesn't exist, model may exist.
        QStandardItemModel *model = 0;
        if( d->m_cmdModels.contains(title) )
        {
            model = d->m_cmdModels[title]; // reuse the previous model.
            model->clear();
        }
        else
        {
            model = new QStandardItemModel(this);
            d->m_cmdModels.insert( title, model );
        }
        cmd->setModel( model );

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
        // If there are pending command named "title", there should exist model named "title" as well.
        QStandardItemModel *model = d->m_cmdModels[title];
        cmd->setModel( model );
        // Append m_job. When the job named "title" is finished, completed job will be removed
        // and this new job will get started by slotCommandFinished/Failed()
        d->m_jobs[title].enqueue( cmd );
        kDebug(9004) << "Adding and pending" << endl;
    }
}

void StandardOutputView::registerLogView( const QString& id, const QString& title )
{
    if( !d->m_ids.contains( id ) && !d->m_models.contains(id) && !d->m_titles.contains(id) )
    {
        d->m_ids << id;
        d->m_titles[id] = title;
        d->m_models[id] = new QStandardItemModel(this);
        emit modelAdded( d->m_titles[id], d->m_models[id] );
    }
}

void StandardOutputView::appendLine( const QString& id, const QString& line )
{
    if( d->m_models.contains( id ) )
    {
        d->m_models[id]->appendRow( new QStandardItem( line ) );
    }
}

void StandardOutputView::appendLines( const QString& id, const QStringList& lines )
{
    if( d->m_models.contains( id ) )
    {
        foreach( QString line, lines )
            d->m_models[id]->appendRow( new QStandardItem( line ) );
    }
}

QStringList StandardOutputView::registeredViews()
{
    return d->m_ids;
}

QStandardItemModel* StandardOutputView::registeredModel( const QString& id )
{
    if( d->m_models.contains( id ) )
    {
        return d->m_models[id];
    }
    return 0;
}

QString StandardOutputView::registeredTitle( const QString& id )
{
    if( d->m_titles.contains( id ) )
    {
        return d->m_titles[id];
    }
    return QString();
}

void StandardOutputView::slotCommandFinished( const QString& id )
{
    cleanupTerminatedJobs( id );
    startNextPendingJob( id );
}

void StandardOutputView::slotCommandFailed( const QString& id )
{
    cleanupTerminatedJobs( id );
    startNextPendingJob( id );
}

void StandardOutputView::cleanupTerminatedJobs( const QString& id )
{
    Q_ASSERT( d->m_jobs.contains(id) );

    QQueue<OutputViewCommand*> &cmdQ = d->m_jobs[id];
    Q_ASSERT( cmdQ.isEmpty() == false );
    OutputViewCommand *cmd = cmdQ.dequeue();
    cmd->deleteLater();
    kDebug(9004) << "OutputViewCommand removed and deleteLater()ed " << (long)cmd << endl;

    if( cmdQ.isEmpty() )
    {
        d->m_jobs.remove( id );
    }
}

void StandardOutputView::startNextPendingJob( const QString &id )
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

#include "standardoutputview.moc"
// kate: space-indent on; indent-width 4; tab-width: 4; replace-tabs on; auto-insert-doxygen on
