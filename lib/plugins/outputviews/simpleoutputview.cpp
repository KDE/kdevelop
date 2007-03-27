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

#include <config.h>

#include <QtCore/QStringList>

#include <QtDesigner/QExtensionFactory>
#include <QtGui/QListView>
#include <QtCore/QTimer>
#include <QtGui/QStandardItemModel>

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
        QListView* l = new QListView(parent);
        l->setModel( m_part->model() );
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
    Q_DECLARE_PUBLIC( SimpleOutputView )
    SimpleOutputView* q_ptr;
public:
    SimpleOutputViewViewFactory* m_factory;
    QStandardItemModel* m_model;
    QList<QPair<KUrl, QStringList> > m_jobs;
    K3Process* m_childProc;
    QStringList m_currentCmd;
    bool isRunning()
    {
        return (m_childProc->isRunning());
    }
    void startNextJob()
    {
        if( m_jobs.isEmpty() )
            return;
        m_model->clear();
        m_childProc->clearArguments();
//         m_childProc->setUseShell( true );
        QPair<KUrl, QStringList> job = m_jobs.takeFirst();
        m_childProc->setWorkingDirectory( job.first.path() );
        QStringList l = job.second;
        m_currentCmd = job.second;
//         QString cmd = l.takeFirst();
        QStandardItem* i = new QStandardItem( m_currentCmd.join(" ") );
        m_model->appendRow( i );
        foreach(QString s, l)
            if( !s.isEmpty() )
                *m_childProc << s;
        m_childProc->start( K3Process::OwnGroup, K3Process::AllOutput );
        if( !isRunning() )
            kDebug(9000) << "Couldn't start process" << endl;
    }
    void procReadStdout(K3Process* proc, char* buf, int len)
    {
        QString txt = QString::fromLocal8Bit( buf, len );
        QStringList l = txt.split("\n");
        foreach( QString s, l )
        {
            m_model->appendRow( new QStandardItem( s ) );
        }
    }

    void procReadStderr(K3Process* proc, char* buf, int len)
    {
        QString txt = QString::fromLocal8Bit( buf, len );
        QStringList l = txt.split("\n");
        foreach( QString s, l )
        {
            m_model->appendRow( new QStandardItem( s ) );
        }
    }

    void procFinished( K3Process* proc )
    {
        Q_Q(SimpleOutputView);
        if( !proc->exitStatus() )
        {
            QStandardItem* endItem = new QStandardItem(QString("Finished (%1)").arg(proc->exitStatus()) );
            m_model->appendRow( endItem );
            kDebug(9004) << "Finished Sucessfully" << endl;
            emit q->commandFinished( m_currentCmd );
        }
        else
        {
            QStandardItem* endItem = new QStandardItem(QString("Failed (%1)").arg(proc->exitStatus()));
            m_model->appendRow( endItem );
            kDebug(9004) << "Failed" << endl;
            emit q->commandFailed( m_currentCmd );
        }
        QTimer::singleShot(0, q, SLOT( startNextJob() ) );
    }
};

SimpleOutputView::SimpleOutputView(QObject *parent, const QStringList &)
    : KDevelop::IPlugin(SimpleOutputViewFactory::componentData(), parent),
      d(new SimpleOutputViewPrivate)
{
    d->q_ptr = this;
    KDEV_USE_EXTENSION_INTERFACE( KDevelop::IOutputView )
    d->m_model = new QStandardItemModel( this );
    d->m_childProc = new K3Process( this );
    d->m_factory = new SimpleOutputViewViewFactory( this );
    core()->uiController()->addToolView( "Output View", d->m_factory );
    connect( d->m_childProc, SIGNAL(receivedStdout(K3Process* , char*, int) ), this, SLOT( procReadStdout(K3Process* , char*, int) ) );
    connect( d->m_childProc, SIGNAL(receivedStderr(K3Process* , char*, int) ), this, SLOT( procReadStderr(K3Process* , char*, int) ) );
    connect( d->m_childProc, SIGNAL(processExited( K3Process* ) ),
             this, SLOT( procFinished( K3Process* ) ) );
}

SimpleOutputView::~SimpleOutputView()
{
    delete d;
}

QStandardItemModel* SimpleOutputView::model()
{
    return d->m_model;
}

void SimpleOutputView::queueCommand(const KUrl& dir, const QStringList& command, const QStringList& env )
{
    Q_UNUSED(env)
    kDebug(9004) << "Queueing Command: " << dir << "|" << command << endl;
    d->m_jobs.append(QPair<KUrl,QStringList>(dir,command));
    if( !d->isRunning() )
    {
        d->startNextJob();
    }
}


#include "simpleoutputview.moc"
// kate: space-indent on; indent-width 4; tab-width: 4; replace-tabs on; auto-insert-doxygen on
