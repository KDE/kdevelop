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
#include <kprocess.h>
#include <kdialog.h>
#include <kglobal.h>
#include <klocale.h>
#include <kdebug.h>

typedef KGenericFactory<SimpleOutputView> SimpleOutputViewFactory ;
K_EXPORT_COMPONENT_FACTORY( kdevsimpleoutputview,
                            SimpleOutputViewFactory( "kdevsimpleoutputview" ) )

KDEV_ADD_EXTENSION_FACTORY_NS( KDevelop, IOutputView, SimpleOutputView )

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
public:
    SimpleOutputViewViewFactory* m_factory;
    QStandardItemModel* m_model;
    QList<QPair<KUrl, QStringList> > m_jobs;
    QProcess* m_childProc;
    QStringList m_currentCmd;
    bool isRunning()
    {
        return (m_childProc->state() == QProcess::Running);
    }
    void startNextJob()
    {
        if( m_jobs.isEmpty() )
            return;
        m_model->clear();
        m_childProc->close();
        QPair<KUrl, QStringList> job = m_jobs.takeFirst();
        m_childProc->setWorkingDirectory( job.first.path() );
        QStringList l = job.second;
        m_currentCmd = job.second;
        QString cmd = l.takeFirst();
        QStandardItem* i = new QStandardItem( m_currentCmd.join(" ") );
        m_model->appendRow( i );
        m_childProc->start(cmd, l);
    }
    void procReadyReadStdOut()
    {
        QString txt = QString::fromLocal8Bit( m_childProc->readAllStandardOutput() );
        QStringList l = txt.split("\n");
        foreach( QString s, l )
        {
            m_model->appendRow( new QStandardItem( s ) );
        }
    }

    void procReadyReadStdErr()
    {
        QString txt = QString::fromLocal8Bit( m_childProc->readAllStandardOutput() );
        QStringList l = txt.split("\n");
        foreach( QString s, l )
        {
            m_model->appendRow( new QStandardItem( s ) );
        }
    }

};

SimpleOutputView::SimpleOutputView(QObject *parent, const QStringList &)
    : KDevelop::IPlugin(SimpleOutputViewFactory::componentData(), parent),
      d(new SimpleOutputViewPrivate)
{
    d->m_model = new QStandardItemModel( this );
    d->m_childProc = new QProcess( this );
    d->m_factory = new SimpleOutputViewViewFactory( this );
    core()->uiController()->addToolView( "Output View", d->m_factory );
    connect( d->m_childProc, SIGNAL(readyReadStandardOutput() ), this, SLOT( procReadyReadStdOut() ) );
    connect( d->m_childProc, SIGNAL(readyReadStandardError() ), this, SLOT( procReadyReadStdOut() ) );
    connect( d->m_childProc, SIGNAL(finished(int, QProcess::ExitStatus) ),
             this, SLOT( procFinished( int, QProcess::ExitStatus ) ) );
}

SimpleOutputView::~SimpleOutputView()
{
    delete d;
}

QStandardItemModel* SimpleOutputView::model()
{
    return d->m_model;
}

void SimpleOutputView::queueCommand(const KUrl& dir, const QStringList& command )
{
    d->m_jobs.append(QPair<KUrl,QStringList>(dir,command));
    if( !d->isRunning() )
    {
        d->startNextJob();
    }
}


void SimpleOutputView::procFinished( int exitCode, QProcess::ExitStatus exitStatus )
{
    Q_UNUSED(exitCode);
    if( exitStatus == QProcess::NormalExit )
    {
        QStandardItem* endItem = new QStandardItem(QString("Finished (%1)").arg(exitCode) );
        d->m_model->appendRow( endItem );
        emit commandFinished( d->m_currentCmd );
    }
    else
    {
        QStandardItem* endItem = new QStandardItem(QString("Failed (%1)").arg(exitCode));
        d->m_model->appendRow( endItem );
        emit commandFailed( d->m_currentCmd );
    }
    QTimer::singleShot(0, this, SLOT( startNextJob() ) );
}

QStringList SimpleOutputView::extensions() const
{
    return QStringList() << "IOutputView";
}


void SimpleOutputView::registerExtensions()
{
    extensionManager()->registerExtensions( new SimpleOutputViewIOutputViewFactory(
    extensionManager() ), Q_TYPEID( KDevelop::IOutputView ) );
}
void SimpleOutputView::unregisterExtensions()
{
    extensionManager()->unregisterExtensions( new SimpleOutputViewIOutputViewFactory(
    extensionManager() ), Q_TYPEID( KDevelop::IOutputView ) );
}

#include "simpleoutputview.moc"
// kate: space-indent on; indent-width 4; tab-width: 4; replace-tabs on; auto-insert-doxygen on
