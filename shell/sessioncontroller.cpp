/* This file is part of KDevelop
Copyright 2008 Andreas Pakulat <apaku@gmx.de>

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public License
along with this library; see the file COPYING.LIB.  If not, write to
the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
Boston, MA 02110-1301, USA.
*/

#include "sessioncontroller.h"

#include <QtCore/QHash>
#include <QtCore/QDir>
#include <QtCore/QSignalMapper>
#include <QtCore/QStringList>

#include <kglobal.h>
#include <kcmdlineargs.h>
#include <kcomponentdata.h>
#include <kconfiggroup.h>
#include <kstandarddirs.h>
#include <klocale.h>
#include <kio/netaccess.h>
#include <kparts/mainwindow.h>
#include <kactioncollection.h>

#include "session.h"
#include "core.h"
#include "uicontroller.h"
#include "sessiondialog.h"
#include <interfaces/iprojectcontroller.h>
#include <qapplication.h>
#include <kprocess.h>
#include <sublime/mainwindow.h>
#include <KApplication>
#include <QLineEdit>
#include <KMessageBox>
#include <QGroupBox>
#include <QBoxLayout>

namespace KDevelop
{

static QStringList standardArguments()
{
    QStringList ret;
    for(int a = 0; a < QApplication::argc(); ++a)
    {
        QString arg = QString::fromLocal8Bit(QApplication::argv()[a]);
        if(arg.startsWith("--graphicssystem=") || arg.startsWith("--style="))
        {
            ret << arg;
        }else if(arg.startsWith("--graphicssystem") || arg.startsWith("--style"))
        {
            ret << arg;
            if(a+1 < QApplication::argc())
                ret << QString::fromLocal8Bit(QApplication::argv()[a+1]);
        }
    }
    return ret;
}

class SessionControllerPrivate : public QObject
{
    Q_OBJECT
public:
    SessionControllerPrivate( SessionController* s ) : q(s) {}

    Session* findSessionForName( const QString& name ) const
    {
        foreach( Session* s, sessionActions.keys() )
        {
            if( s->name() == name )
                return s;
        }
        return 0;
    }
    
    Session* findSessionForId(QString idString)
    {
        QUuid id(idString);
        
        foreach( Session* s, sessionActions.keys() )
        {
            if( s->id() == id)
                return s;
        }
        return 0;
    }
    
    void newSession()
    {
        Session* session = new Session( QUuid::createUuid() );
        
        KProcess::startDetached(QFileInfo(QApplication::applicationFilePath()).path() + "/kdevelop", QStringList() << "-s" << session->id().toString() << standardArguments());
        delete session;
#if 0        
        //Terminate this instance of kdevelop if the user agrees
        foreach(Sublime::MainWindow* window, Core::self()->uiController()->controller()->mainWindows())
            window->close();
#endif
    }
    
    void configureSessions()
    {
        SessionDialog dlg(ICore::self()->uiController()-> activeMainWindow());
        dlg.exec();
    }

    void deleteSession()
    {
        int choice = KMessageBox::warningContinueCancel(Core::self()->uiController()->activeMainWindow(), i18n("The current session and all contained settings will be deleted. The projects will stay unaffected. Do you really want to continue?"));
        
        if(choice == KMessageBox::Continue)
        {
            static_cast<Session*>(q->activeSession())->deleteFromDisk();
            q->emitQuitSession();
        }
    }

    void renameSession()
    {
        KDialog dialog;
        dialog.setWindowTitle(i18n("Rename Session"));
        QGroupBox box;
        QHBoxLayout layout(&box);
        
        box.setTitle(i18n("New Session Name"));
        QLineEdit edit;
        layout.addWidget(&edit);
        dialog.setButtons(KDialog::Ok | KDialog::Cancel);
        edit.setText(q->activeSession()->name());
        dialog.setMainWidget(&box);
        
        edit.setFocus();
        
        if(dialog.exec() == QDialog::Accepted)
        {
            static_cast<Session*>(q->activeSession())->setName(edit.text());
        }
    }

    void loadSessionExternally( Session* s )
    {
        Q_ASSERT( s );
        KProcess::startDetached(QFileInfo(QApplication::applicationFilePath()).path() + "/kdevelop", QStringList() << "-s" << s->id().toString() << standardArguments());
    }
    
    void activateSession( Session* s )
    {
        Q_ASSERT( s );

        KConfigGroup grp = KGlobal::config()->group( SessionController::cfgSessionGroup() );
        grp.writeEntry( SessionController::cfgActiveSessionEntry(), s->id().toString() );
        grp.sync();
        activeSession = s;
        if (Core::self()->setupFlags() & Core::NoUi) return;

        QHash<Session*,QAction*>::iterator it = sessionActions.find(s);
        Q_ASSERT( it != sessionActions.end() );
        (*it)->setCheckable(true);
        (*it)->setChecked(true);
        
        for(it = sessionActions.begin(); it != sessionActions.end(); ++it)
        {
            if(it.key() != s)
                (*it)->setCheckable(false);
        }
        
    }

    void loadSessionFromAction( QAction* a )
    {
        foreach( Session* s, sessionActions.keys() )
        {
            if( s->id() == QUuid( a->data().toString() ) && s != activeSession ) {
                loadSessionExternally( s );
                
                //Terminate this instance of kdevelop if the user agrees
                foreach(Sublime::MainWindow* window, Core::self()->uiController()->controller()->mainWindows())
                    window->close();
                break;
            }
        }
    }

    void addSession( Session* s )
    {
        if (Core::self()->setupFlags() & Core::NoUi) {
            sessionActions[s] = 0;
            return;
        }

        KAction* a = new KAction( grp );
        a->setText( s->description() );
        a->setCheckable( false );
        a->setData( s->id().toString() );
        sessionActions[s] = a;
        q->actionCollection()->addAction( "session_"+s->id().toString(), a );
        q->unplugActionList( "available_sessions" );
        q->plugActionList( "available_sessions", grp->actions() );
        connect(s, SIGNAL(nameChanged(QString, QString)), SLOT(nameChanged()));
    }

    QHash<Session*, QAction*> sessionActions;
    ISession* activeSession;
    SessionController* q;
    QActionGroup* grp;

private slots:
    void nameChanged()
    {
        Q_ASSERT(qobject_cast<Session*>(sender()));
        Session* s = static_cast<Session*>(sender());
        sessionActions[s]->setText( s->description() );
    }
};

void SessionController::updateSessionDescriptions()
{
    for(QHash< Session*, QAction* >::iterator it = d->sessionActions.begin(); it != d->sessionActions.end(); ++it) {
        it.key()->updateDescription();
        if (*it) (*it)->setText(it.key()->description());
    }
}

SessionController::SessionController( QObject *parent )
        : QObject( parent ), d(new SessionControllerPrivate(this))
{
    setObjectName("SessionController");
    setComponentData(KComponentData("kdevsession"));
    
    setXMLFile("kdevsessionui.rc");

    if (Core::self()->setupFlags() & Core::NoUi) return;

    KAction* action = actionCollection()->addAction( "new_session", this, SLOT( newSession() ) );
    action->setText( i18n("Start New Session") );
    action->setToolTip( i18n("Start a new KDevelop instance with an empty session") );
    action->setIcon(KIcon("window-new"));

    action = actionCollection()->addAction( "rename_session", this, SLOT( renameSession() ) );
    action->setText( i18n("Rename Session...") );
    action->setIcon(KIcon("edit-rename"));

    action = actionCollection()->addAction( "delete_session", this, SLOT( deleteSession() ) );
    action->setText( i18n("Delete Session...") );
    action->setIcon(KIcon("edit-delete"));

    action = actionCollection()->addAction( "quit", this, SIGNAL( quitSession() ) );
    action->setText( i18n("Quit Session") );
    action->setShortcut(Qt::CTRL | Qt::Key_Q);
    action->setIcon(KIcon("application-exit"));

    #if 0
    action = actionCollection()->addAction( "configure_sessions", this, SLOT( configureSessions() ) );
    action->setText( i18n("Configure Sessions...") );
    action->setToolTip( i18n("Create/Delete/Activate Sessions") );
    action->setWhatsThis( i18n( "<b>Configure Sessions</b><p>Shows a dialog to Create/Delete Sessions and set a new active session.</p>" ) );
    #endif

    d->grp = new QActionGroup( this );
    connect( d->grp, SIGNAL(triggered(QAction*)), this, SLOT(loadSessionFromAction(QAction*)) );
}

SessionController::~SessionController()
{
    delete d;
}

void SessionController::startNewSession()
{
    d->newSession();
}

void SessionController::cleanup()
{
    qDeleteAll(d->sessionActions);
}

void SessionController::initialize()
{
    QDir sessiondir( SessionController::sessionDirectory() );
    foreach( const QString& s, sessiondir.entryList( QDir::AllDirs ) )
    {
        QUuid id( s );
        if( id.isNull() )
            continue;
        // Only create sessions for directories that represent proper uuid's
        Session* session = new Session( id );
        
        //Delete sessions that have no name and are empty
        if( session->description().isEmpty() && (session->id().toString() != QString(getenv("KDEV_SESSION"))))
        {
            ///@todo Think about when we can do this. Another instance might still be using this session.
//             session->deleteFromDisk();
            delete session;
        }else{
            d->addSession( session );
        }
    }
    loadDefaultSession();
    
    connect(Core::self()->projectController(), SIGNAL(projectClosed(KDevelop::IProject*)), SLOT(updateSessionDescriptions()));
    connect(Core::self()->projectController(), SIGNAL(projectOpened(KDevelop::IProject*)), SLOT(updateSessionDescriptions()));
    updateSessionDescriptions();
}


ISession* SessionController::activeSession() const
{
    return d->activeSession;
}

void SessionController::loadSession( const QString& nameOrId )
{
    d->loadSessionExternally( session( nameOrId ) );
}

QList<QString> SessionController::sessionNames() const
{
    QStringList l;
    foreach( const Session* s, d->sessionActions.keys() )
    {
        l << s->name();
    }
    return l;
}

QList< const KDevelop::Session* > SessionController::sessions() const
{
    QList< const KDevelop::Session* > ret;
    foreach( const Session* s, d->sessionActions.keys() )
    {
        ret << s;
    }
    return ret;
}

Session* SessionController::createSession( const QString& name )
{
    Session* s = new Session( QUuid::createUuid() );
    s->setName( name );
    d->addSession( s );
    return s;
}

void SessionController::deleteSession( const QString& nameOrId )
{
    Session* s  = session(nameOrId);
    
    Q_ASSERT( s != d->activeSession ) ;
    
    QHash<Session*,QAction*>::iterator it = d->sessionActions.find(s);
    Q_ASSERT( it != d->sessionActions.end() );

    unplugActionList( "available_sessions" );
    d->grp->removeAction(*it);
    actionCollection()->removeAction(*it);
    plugActionList( "available_sessions", d->grp->actions() );
    s->deleteFromDisk();
    emit sessionDeleted( s->name() );
    d->sessionActions.remove(s);
    s->deleteLater();
}

void SessionController::loadDefaultSession()
{
    QString load = QString(getenv("KDEV_SESSION"));
    
    if(!load.isEmpty())
    {
        if(!session(load))
            load = createSession("")->id().toString();
        
        ///KDEV_SESSION must be the UUID of an existing session, and nothing else.
        ///If this assertion fails, that was not the case.
        Q_ASSERT(session(load)->id().toString() == load);
        
        d->activateSession( session(load) );
        return;
    }
    KConfigGroup grp = KGlobal::config()->group( cfgSessionGroup() );
    load = grp.readEntry( cfgActiveSessionEntry(), "default" );
    
    if( !session( load ) )
    {
        createSession( load );
    }  
    
    d->activateSession( session(load) );
}

Session* SessionController::session( const QString& nameOrId ) const
{
    Session* ret = d->findSessionForName( nameOrId );
    if(ret)
        return ret;

    return d->findSessionForId( nameOrId );
}

QString SessionController::cloneSession( const QString& nameOrid )
{
    Session* origSession = session( nameOrid );
    QUuid id = QUuid::createUuid();
    KIO::NetAccess::dircopy( KUrl( sessionDirectory() + '/' + origSession->id().toString() ), 
                             KUrl( sessionDirectory() + '/' + id.toString() ), 
                             Core::self()->uiController()->activeMainWindow() );
    Session* newSession = new Session( id );
    newSession->setName( i18n( "Copy of %1", origSession->name() ) );
    d->addSession(newSession);
    return newSession->name();
}

void SessionController::plugActions()
{
    unplugActionList( "available_sessions" );
    plugActionList( "available_sessions", d->grp->actions() );
}

}
#include "sessioncontroller.moc"
#include "moc_sessioncontroller.cpp"
