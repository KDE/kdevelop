/* This file is part of KDevelop
Copyright 2008 Andreas Pakulat <apaku@gmx.de>
Copyright 2010 David Nolden <david.nolden.kdevelop@art-master.de>

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
#include <QtCore/QTimer>

#include <kglobal.h>
#include <kcmdlineargs.h>
#include <kcomponentdata.h>
#include <kconfiggroup.h>
#include <kstandarddirs.h>
#include <klocale.h>
#include <kio/netaccess.h>
#include <kparts/mainwindow.h>
#include <kactioncollection.h>
#include <kpushbutton.h>

#include "session.h"
#include "core.h"
#include "uicontroller.h"
#include "sessiondialog.h"
#include "shellextension.h"
#include <interfaces/iprojectcontroller.h>
#include <util/fileutils.h>
#include <qapplication.h>
#include <kprocess.h>
#include <sublime/mainwindow.h>
#include <KApplication>
#include <QLineEdit>
#include <KMessageBox>
#include <KAboutData>
#include <KLineEdit>
#include <QGroupBox>
#include <QBoxLayout>
#include <QTimer>
#include <QStandardItemModel>
#include <QListView>
#include <QHeaderView>
#include <klockfile.h>
#include <interfaces/idocumentcontroller.h>
#include <ktexteditor/document.h>
#include <sublime/area.h>
#include <QLabel>
#include <QLayout>
#include <QSortFilterProxyModel>
#include <QKeyEvent>
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusInterface>
#include <QDBusReply>


#include <kdeversion.h>

#if KDE_IS_VERSION(4,5,60)
    #define HAVE_RECOVERY_INTERFACE
    #include <ktexteditor/recoveryinterface.h>
#endif

const int recoveryStorageInterval = 10; ///@todo Make this configurable

namespace KDevelop
{

namespace {
    int argc = 0;
    char** argv = 0; 
};

void SessionController::setArguments(int _argc, char** _argv)
{
    argc = _argc;
    argv = _argv;
}

static QStringList standardArguments()
{
    QStringList ret;
    for(int a = 0; a < argc; ++a)
    {
        QString arg = QString::fromLocal8Bit(argv[a]);
        kWarning() << "ARG:" << "" + arg + "";
/*        if(arg.startsWith("--graphicssystem=") || arg.startsWith("--style="))
        {
            ret << arg;
        }else */
        if(arg.startsWith("-graphicssystem") || arg.startsWith("-style"))
        {
            ret << '-' + arg;
            if(a+1 < argc)
                ret << QString::fromLocal8Bit(argv[a+1]);
        }
    }
    
    kWarning() << "ARGUMENTS: " << ret << "from" << argc;
    
    return ret;
}

class SessionControllerPrivate : public QObject
{
    Q_OBJECT
public:
    SessionControllerPrivate( SessionController* s )
        : q(s)
        , activeSession(0)
        , grp(0)
        , recoveryDirectoryIsOwn(false)
    {
        recoveryTimer.setInterval(recoveryStorageInterval * 1000);
        connect(&recoveryTimer, SIGNAL(timeout()), SLOT(recoveryStorageTimeout()));
        
        // Try the recovery only after the initialization has finished
        connect(ICore::self(), SIGNAL(initializationCompleted()), SLOT(lateInitialization()), Qt::QueuedConnection);
        
        recoveryTimer.setSingleShot(false);
        recoveryTimer.start();
    }

    ~SessionControllerPrivate() {
        if (activeSession) {
            // when session was active, we deleted the folder already
            // in that case activeSession = 0
            clearRecoveryDirectory();
        }
    }

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
        qsrand(QDateTime::currentDateTime().toTime_t());
        Session* session = new Session( QUuid::createUuid() );
        
        KProcess::startDetached(ShellExtension::getInstance()->binaryPath(), QStringList() << "-s" << session->id().toString() << standardArguments());
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

    bool loadSessionExternally( Session* s )
    {
        Q_ASSERT( s );
        KProcess::startDetached(ShellExtension::getInstance()->binaryPath(), QStringList() << "-s" << s->id().toString() << standardArguments());
        return true;
    }
    
    void activateSession( Session* s )
    {
        Q_ASSERT( s );

        activeSession = s;
        if( !lockSession() ) {
            activeSession = 0;
            return;
        }

        KConfigGroup grp = KGlobal::config()->group( SessionController::cfgSessionGroup() );
        grp.writeEntry( SessionController::cfgActiveSessionEntry(), s->id().toString() );
        grp.sync();
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
        
        connect(Core::self()->projectController(), SIGNAL(projectClosed(KDevelop::IProject*)), activeSession, SLOT(updateContainedProjects()));
        connect(Core::self()->projectController(), SIGNAL(projectOpened(KDevelop::IProject*)), activeSession, SLOT(updateContainedProjects()));
    }

    void loadSessionFromAction( QAction* a )
    {
        foreach( Session* s, sessionActions.keys() )
        {
            if( s->id() == QUuid( a->data().toString() ) && s != activeSession ) {
                loadSessionExternally( s );
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
        connect(s, SIGNAL(nameChanged(QString,QString)), SLOT(nameChanged()));
    }

    SessionController* q;

    QHash<Session*, QAction*> sessionActions;
    ISession* activeSession;
    QActionGroup* grp;
    
    KLockFile::Ptr sessionLock;
    SessionController::LockSessionState sessionLockState;
    
    // Whether this process owns the recovery directory
    bool recoveryDirectoryIsOwn;
    
    QTimer recoveryTimer;
    QMap<KUrl, QStringList > currentRecoveryFiles;

    static QString sessionBaseDirectory()
    {
        return KGlobal::mainComponent().dirs()->saveLocation( "data", KGlobal::mainComponent().componentName() + "/sessions/", true );
    }

    static QString sessionDirectory( const QString& id )
    {
        return sessionBaseDirectory() + id;
    }

    static QString lockFileForSession( const QString& id )
    {
        return sessionDirectory( id ) + "/lock";
    }

    static QString DBusServiceNameForSession( const QString& id )
    {
        // We remove starting "{" and ending "}" from the string UUID representation
        // as D-Bus apparently doesn't allow them in service names
        return QString( "org.kdevelop.kdevplatform-lock-" ) + QString( id ).mid( 1, id.size() - 2 );
    }

    QString ownDBusServiceName()
    {
        Q_ASSERT(activeSession);
        return DBusServiceNameForSession( activeSession->id() );
    }
    
    QString ownSessionDirectory() const
    {
        Q_ASSERT(activeSession);
        return sessionDirectory( activeSession->id().toString() );
    }

    QString ownLockFile()
    {
        Q_ASSERT(activeSession);
        return lockFileForSession( activeSession->id().toString() );
    }

    void clearRecoveryDirectory()
    {
        removeDirectory(ownSessionDirectory() + "/recovery");
    }
    
    bool lockSession()
    {
        sessionLockState = SessionController::tryLockSession( activeSession->id(), true );
        return sessionLockState;
    }

public slots:
    void documentSavedOrClosed( KDevelop::IDocument* document )
    {
        if(currentRecoveryFiles.contains(document->url()))
        {
            kDebug() << "deleting recovery-info for" << document->url();
            foreach(const QString& recoveryFileName, currentRecoveryFiles[document->url()])
            {
                bool result = QFile::remove(recoveryFileName);
                kDebug() << "deleted" << recoveryFileName << result;
            }
            currentRecoveryFiles.remove(document->url());
        }
    }
    
private slots:
    void lateInitialization()
    {
        performRecovery();
        connect(Core::self()->documentController(), SIGNAL(documentSaved(KDevelop::IDocument*)), SLOT(documentSavedOrClosed(KDevelop::IDocument*)));
        connect(Core::self()->documentController(), SIGNAL(documentClosed(KDevelop::IDocument*)), SLOT(documentSavedOrClosed(KDevelop::IDocument*)));
    }

    void performRecovery()
    {
        kDebug() << "Checking recovery";
        QDir recoveryDir(ownSessionDirectory() + "/recovery");
        
        if(recoveryDir.exists())
        {
            kDebug() << "Have recovery directory, starting recovery";
            QFile dateFile(recoveryDir.path() + "/date");
            dateFile.open(QIODevice::ReadOnly);
            QString date = QString::fromUtf8(dateFile.readAll());
            
            QDir recoverySubDir(recoveryDir.path() + "/current");
            
            if(!recoverySubDir.exists())
                recoverySubDir = QDir(recoveryDir.path() + "/backup");
            
            if(recoverySubDir.exists())
            {
                kWarning() << "Starting recovery from " << recoverySubDir.absolutePath();
                
                QStringList urlList;
                
                for(uint num = 0; ; ++num)
                {
                    QFile urlFile(recoverySubDir.path() + QString("/%1_url").arg(num));
                    if(!urlFile.exists())
                        break;
                    urlFile.open(QIODevice::ReadOnly);
                    KUrl originalFile(QString::fromUtf8(urlFile.readAll()));
                    urlList << originalFile.pathOrUrl();
                }
                
                if(!urlList.isEmpty())
                {
                    //Either recover, or delete the recovery directory
                    ///TODO: user proper runtime locale for date, it might be different
                    ///      from what was used when the recovery file was saved
                    KGuiItem recover = KStandardGuiItem::cont();
                    recover.setIcon(KIcon("edit-redo"));
                    recover.setText(i18n("Recover"));
                    KGuiItem discard = KStandardGuiItem::discard();
                    int choice = KMessageBox::warningContinueCancelList(qApp->activeWindow(),
                        i18nc("%1: date of the last snapshot",
                              "The session crashed the last time it was used. "
                              "The following modified files can be recovered from a backup from %1.", date),
                        urlList, i18n("Crash Recovery"), recover, discard );

                    if(choice == KMessageBox::Continue)
                    {
                        #if 0
                        {
                            //Put the recovered documents into the "Review" area, and clear the working set
                            ICore::self()->uiController()->switchToArea("review", KDevelop::IUiController::ThisWindow);
                            Sublime::MainWindow* window = static_cast<Sublime::MainWindow*>(ICore::self()->uiController()->activeMainWindow());
                            window->area()->setWorkingSet("recover");
                            window->area()->clearViews();
                        }
                        #endif
                        
                        //Recover the files
                        
                        for(uint num = 0; ; ++num)
                        {
                            QFile urlFile(recoverySubDir.path() + QString("/%1_url").arg(num));
                            if(!urlFile.exists())
                                break;
                            urlFile.open(QIODevice::ReadOnly);
                            KUrl originalFile(QString::fromUtf8(urlFile.readAll()));
                            
                            QFile f(recoverySubDir.path() + '/' + QString("/%1_text").arg(num));
                            f.open(QIODevice::ReadOnly);
                            QString text = QString::fromUtf8(f.readAll());
                            
                            if(text.isEmpty())
                            {
                                KMessageBox::error(ICore::self()->uiController()->activeMainWindow(), i18n("Could not recover %1, the recovery file is empty", originalFile.pathOrUrl()), i18n("Recovery"));
                                continue;
                            }
                            
                            kDebug() << "opening" << originalFile << "for recovery";
                            KDevelop::IDocument* doc = ICore::self()->documentController()->openDocument(originalFile);
                            if(!doc || !doc->textDocument())
                            {
                                kWarning() << "The document " << originalFile.prettyUrl() << " could not be opened as a text-document, creating a new document with the recovered contents";
                                doc = ICore::self()->documentController()->openDocumentFromText(text);
                            }else{
                                #ifdef HAVE_RECOVERY_INTERFACE
                                KTextEditor::RecoveryInterface* recovery = qobject_cast<KTextEditor::RecoveryInterface*>(doc->textDocument());
                                
                                if(recovery && recovery->isDataRecoveryAvailable())
                                    // Use the recovery from the kate swap-file if possible
                                    recovery->recoverData();
                                else
                                    // Use a simple recovery through "replace text"
                                    doc->textDocument()->setText(text);
                                #else
                                    // Use a simple recovery through "replace text"
                                    doc->textDocument()->setText(text);
                                #endif
                            }
                        }
                    }
                }
            }
        }
        
        recoveryDirectoryIsOwn = true;
    }
    
    void nameChanged()
    {
        Q_ASSERT(qobject_cast<Session*>(sender()));
        Session* s = static_cast<Session*>(sender());
        sessionActions[s]->setText( s->description() );
    }
    
    void recoveryStorageTimeout()
    {
        if(!recoveryDirectoryIsOwn)
            return;
        
        currentRecoveryFiles.clear();
        
        QDir recoveryDir(ownSessionDirectory() + "/recovery");
        
        if(!recoveryDir.exists())
        {
            // Try "taking" the recovery directory
            QDir sessionDir(ownSessionDirectory());
            if(!sessionDir.mkdir("recovery"))
                return;
        }

        if (recoveryDir.exists("backup")) {
            // Clear the old backup recovery directory, as we will create a new one
            if (!removeDirectory(recoveryDir.absoluteFilePath("backup"))) {
                kWarning() << "RECOVERY ERROR: Removing the old recovery backup directory failed in " << recoveryDir;
                return;
            }
        }
        
        //Make the current recovery dir the backup dir, so we always have a recovery available
        //This may fail, because "current" might be nonexistent
        recoveryDir.rename("current", "backup");

        {
            recoveryDir.mkdir("current_incomplete");
            
            QDir recoveryCurrentDir(recoveryDir.path() + "/current_incomplete");

            uint num = 0;
            
            foreach(KDevelop::IDocument* document, ICore::self()->documentController()->openDocuments())
            {
                if(document->state() == IDocument::Modified || document->state() == IDocument::DirtyAndModified)
                {
                    //This document was modified, create a recovery-backup
                    if(document->textDocument())
                    {
                        //Currently we can only back-up text documents
                        QString text = document->textDocument()->text();
                        
                        if(!text.isEmpty())
                        {
                            QString urlFilePath = recoveryCurrentDir.path() + QString("/%1_url").arg(num);
                            QFile urlFile(urlFilePath);
                            urlFile.open(QIODevice::WriteOnly);
                            urlFile.write(document->url().pathOrUrl().toUtf8());
                            urlFile.close();
                            
                            QString textFilePath = recoveryCurrentDir.path() + '/' + QString("/%1_text").arg(num);
                            QFile f(textFilePath);
                            f.open(QIODevice::WriteOnly);
                            f.write(text.toUtf8());
                            f.close();
                            
                            currentRecoveryFiles[document->url()] =
                                        QStringList() <<  (recoveryDir.path() + "/current" + QString("/%1_url").arg(num))
                                                      << (recoveryDir.path() + "/current" + QString("/%1_text").arg(num));
                            
                            if(urlFile.error() != QFile::NoError || f.error() != QFile::NoError)
                            {
                                kWarning() << "RECOVERY ERROR: Failed to write recovery for" << document->url() << "to" << textFilePath;
                                KMessageBox::error(ICore::self()->uiController()->activeMainWindow(),
                                                    i18n("Failed to write recovery copies to %1. Please make sure that your home directory is writable and not full. This application requires available space in the home directory to run stable. You may experience application crashes until you free up some space.", recoveryCurrentDir.path()),
                                                    i18n("Recovery Error"));
                                return;
                            }
                                                      
                            ++num;
                        }
                    }
                }
            }
        }
        
        recoveryDir.rename("current_incomplete", "current");
        
        {
            //Write down the date of the recovery
            QFile dateFile(recoveryDir.path() + "/date");
            dateFile.open(QIODevice::WriteOnly);
            dateFile.write(QDateTime::currentDateTime().toString(Qt::DefaultLocaleShortDate).toUtf8());
        }
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

    QDBusConnection::sessionBus().registerObject( "/kdevelop/SessionController",
        this, QDBusConnection::ExportScriptableSlots );

    if (Core::self()->setupFlags() & Core::NoUi) return;

    KAction* action = actionCollection()->addAction( "new_session", this, SLOT(newSession()) );
    action->setText( i18nc("@action:inmenu", "Start New Session") );
    action->setToolTip( i18nc("@info:tooltip", "Start a new KDevelop instance with an empty session") );
    action->setIcon(KIcon("window-new"));

    action = actionCollection()->addAction( "rename_session", this, SLOT(renameSession()) );
    action->setText( i18n("Rename Current Session...") );
    action->setIcon(KIcon("edit-rename"));

    action = actionCollection()->addAction( "delete_session", this, SLOT(deleteSession()) );
    action->setText( i18n("Delete Current Session...") );
    action->setIcon(KIcon("edit-delete"));

    action = actionCollection()->addAction( "quit", this, SIGNAL(quitSession()) );
    action->setText( i18n("Quit") );
    action->setShortcut(Qt::CTRL | Qt::Key_Q);
    action->setIcon(KIcon("application-exit"));

    #if 0
    action = actionCollection()->addAction( "configure_sessions", this, SLOT(configureSessions()) );
    action->setText( i18n("Configure Sessions...") );
    action->setToolTip( i18n("Create/Delete/Activate Sessions") );
    action->setWhatsThis( i18n( "Shows a dialog to Create/Delete Sessions and set a new active session." ) );
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
    d->recoveryTimer.stop();

    ISession* active = d->activeSession;
    d->activeSession = 0;
    if (active->isTemporary()) {
        deleteSession(active->name());
    }
    qDeleteAll(d->sessionActions);
    d->sessionActions.clear();
}

void SessionController::initialize( const QString& session )
{
    QDir sessiondir( SessionController::sessionDirectory() );
    
    foreach( const QString& s, sessiondir.entryList( QDir::AllDirs | QDir::NoDotAndDotDot ) )
    {
        QUuid id( s );
        if( id.isNull() )
            continue;
        // Only create sessions for directories that represent proper uuid's
        Session* ses = new Session( id, this );

        //Delete sessions that have no name and are empty
        if( ses->containedProjects().isEmpty() && ses->name().isEmpty()
            && (session.isEmpty() || (ses->id().toString() != session && ses->name() != session)) ) {
            if( tryLockSession(s) ) {
                ses->deleteFromDisk();
            }
            delete ses;
        } else {
            d->addSession( ses );
        }
    }
    loadDefaultSession( session );

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
    Session* s;
    if(name.startsWith('{'))
    {
        s = new Session( QUuid(name) );
    }else{
        qsrand(QDateTime::currentDateTime().toTime_t());
        s = new Session( QUuid::createUuid() );
        s->setName( name );
    }
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
    actionCollection()->removeAction(*it);
    if (d->grp) { // happens in unit tests
        d->grp->removeAction(*it);
        plugActionList( "available_sessions", d->grp->actions() );
    }
    s->deleteFromDisk();
    emit sessionDeleted( s->id().toString() );
    d->sessionActions.remove(s);
    s->deleteLater();
}

void SessionController::loadDefaultSession( const QString& session )
{
    QString load = session;
    if (load.isEmpty()) {
        KConfigGroup grp = KGlobal::config()->group( cfgSessionGroup() );
        load = grp.readEntry( cfgActiveSessionEntry(), "default" );
    }

    // Iteratively try to load the session, asking user what to do in case of failure
    // If showForceOpenDialog() returns empty string, stop trying
    Session* s;
    do
    {
        s = this->session( load );
        if( !s ) {
            s = createSession( load );
        }
        d->activateSession( s );
        if( activeSession() )
            break;
        load = handleLockedSession( s->name(), s->id().toString(), d->sessionLockState );
    } while( !load.isEmpty() );
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
    qsrand(QDateTime::currentDateTime().toTime_t());
    QUuid id = QUuid::createUuid();
    KIO::NetAccess::dircopy( SessionControllerPrivate::sessionDirectory( origSession->id().toString() ),
                             SessionControllerPrivate::sessionDirectory( id.toString() ),
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


QString SessionController::cfgSessionGroup() { return "Sessions"; }
QString SessionController::cfgActiveSessionEntry() { return "Active Session ID"; }

QList< SessionInfo > SessionController::availableSessionInfo()
{
    QList< SessionInfo > available;
    foreach( const QString& sessionId, QDir( SessionController::sessionDirectory() ).entryList( QDir::AllDirs ) ) {
        if( !QUuid( sessionId ).isNull() ) {
            available << Session::parse( sessionId );
        }
    }
    return available;
}

QString SessionController::sessionDirectory()
{
    return SessionControllerPrivate::sessionBaseDirectory();
}

SessionController::LockSessionState SessionController::tryLockSession(QString id, bool doLocking)
{
    /*
     * We've got two locking mechanisms here: D-Bus unique service name (based on the session id)
     * and a plain lockfile (KLockFile).
     * The latter is required to get the appname/pid of the locking instance
     * in case if it's stale/hanging/crashed (to make user know which PID he needs to kill).
     * D-Bus mechanism is the primary one.
     *
     * Since there is a kind of "logic tree", the code is a bit hard.
     */
    LockSessionState ret;
    ret.sessionId = id;

    QString service = SessionControllerPrivate::DBusServiceNameForSession( id );
    QDBusConnection connection = QDBusConnection::sessionBus();
    QDBusConnectionInterface* connectionInterface = connection.interface();
    ret.DBusService = service;

    ret.lockFilename = SessionControllerPrivate::lockFileForSession( id );
    ret.lockFile = new KLockFile( ret.lockFilename );

    bool canLockDBus = !connectionInterface->isServiceRegistered( service );
    bool lockedDBus = false;

    // Lock D-Bus if we can and we need to
    if( doLocking && canLockDBus ) {
        lockedDBus = connection.registerService( service );
    }

    // Attempt to lock file, despite the possibility to do so and presence of the request (doLocking)
    // This is required as KLockFile::getLockInfo() works only after KLockFile::lock() is called
    ret.attemptRelock();
    if( ret.lockResult == KLockFile::LockOK ) {
        // Unlock immediately if we shouldn't have locked it
        if( !lockedDBus ) {
            ret.lockFile->unlock();
        }
    } else {
        // If locking failed, retrieve the lock's metadata
        ret.lockFile->getLockInfo( ret.holderPid, ret.holderHostname, ret.holderApp );

        if( lockedDBus ) {
            // Since the lock-file is secondary, try to force-lock it if D-Bus locking succeeded
            ret.forceRemoveLockfile();
            ret.attemptRelock();

            // Finally, if removing didn't help, cancel D-Bus locking altogether.
            if( ret.lockResult != KLockFile::LockOK ) {
                connection.unregisterService( service );
                // do not set lockedDBus to false: ret.success will be then set to
                // lockedDBus, and we want it true to indicate that the D-Bus name is free
            }
        }
    }

    // Set the result by D-Bus status
    ret.success = doLocking ? lockedDBus : canLockDBus;
    return ret;
}

// Internal helper class
class SessionChooserDialog : public KDialog {
    Q_OBJECT
public:
    SessionChooserDialog(QListView* view, QAbstractItemModel* model, KLineEdit* filter);
    bool eventFilter(QObject* object, QEvent* event);

public Q_SLOTS:
    void updateState();
    void doubleClicked(QModelIndex);
    void filterTextChanged(QString);

private Q_SLOTS:
    void deleteButtonPressed();
    void showDeleteButton();
    void itemEntered(const QModelIndex& index);

private:
    QListView* m_view;
    QAbstractItemModel* m_model;
    KLineEdit* m_filter;
    QTimer m_updateStateTimer;

    QPushButton* m_deleteButton;
    QTimer m_deleteButtonTimer;
    int m_deleteCandidateRow;
};

SessionChooserDialog::SessionChooserDialog(QListView* view, QAbstractItemModel* model, KLineEdit* filter)
    : m_view(view), m_model(model), m_filter(filter), m_deleteCandidateRow(-1)
{
    m_updateStateTimer.setInterval(5000);
    m_updateStateTimer.setSingleShot(false);
    m_updateStateTimer.start();
    connect(&m_updateStateTimer, SIGNAL(timeout()), SLOT(updateState()));
    connect(view, SIGNAL(doubleClicked(QModelIndex)), SLOT(doubleClicked(QModelIndex)));
    connect(view, SIGNAL(entered(QModelIndex)), SLOT(itemEntered(QModelIndex)));

    m_deleteButton = new KPushButton(view->viewport());
    m_deleteButton->setIcon(KIcon("edit-delete"));
    m_deleteButton->setToolTip(i18nc("@info", "Delete session"));
    m_deleteButton->hide();
    connect(m_deleteButton, SIGNAL(clicked(bool)), SLOT(deleteButtonPressed()));

    m_deleteButtonTimer.setInterval(500);
    m_deleteButtonTimer.setSingleShot(true);
    connect(&m_deleteButtonTimer, SIGNAL(timeout()), SLOT(showDeleteButton()));

    view->setMouseTracking(true);
    view->installEventFilter(this);
    filter->installEventFilter(this);
    connect(filter, SIGNAL(textChanged(QString)), SLOT(filterTextChanged(QString)));
    
    setCaption(i18n("Pick a Session"));
    setButtons(KDialog::Ok | KDialog::Close);
    button(Ok)->setText(i18n("Run"));
    button(Ok)->setIcon(KIcon("media-playback-start"));
}

void SessionChooserDialog::filterTextChanged(QString)
{
    m_view->selectionModel()->setCurrentIndex(m_model->index(0, 0), QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
    bool enabled = m_view->model()->rowCount(QModelIndex())>0;
    button(KDialog::Ok)->setEnabled(enabled);
    m_deleteButton->setVisible(false);
}

void SessionChooserDialog::doubleClicked(QModelIndex index)
{
    if(m_model->flags(index) & Qt::ItemIsEnabled)
        accept();
}

void SessionChooserDialog::updateState() {
    // Sometimes locking may take some time, so we stop the timer, to prevent an 'avalanche' of events
    m_updateStateTimer.stop();
    for(int row = 0; row < m_model->rowCount(); ++row)
    {
        QString session = m_model->index(row, 0).data().toString();

        if(session.isEmpty()) //create new session
            continue;
        
        QString state, tooltip;
        SessionController::LockSessionState lockState = KDevelop::SessionController::tryLockSession(session);
        if(!lockState)
        {
            tooltip = i18n("Active session.\npid %1, app %2, host %3", lockState.holderPid, lockState.holderApp, lockState.holderHostname);
            state = i18n("Running");
        }
        
        m_model->setData(m_model->index(row, 1), lockState ? KIcon("") : KIcon("media-playback-start"), Qt::DecorationRole);
        m_model->setData(m_model->index(row, 1), tooltip, Qt::ToolTipRole);
        m_model->setData(m_model->index(row, 2), state, Qt::DisplayRole);
    }
    
    m_updateStateTimer.start();
}

QString SessionController::showSessionChooserDialog(QString headerText, bool onlyRunning)
{
    // The catalog hasn't been loaded yet
    KGlobal::locale()->insertCatalog("kdevplatform");

    QListView* view = new QListView;
    KLineEdit* filter = new KLineEdit;
    filter->setClearButtonShown( true );
    filter->setClickMessage(i18n("Search"));

    QStandardItemModel* model = new QStandardItemModel(view);

    QSortFilterProxyModel *proxy = new QSortFilterProxyModel(model);
    proxy->setSourceModel(model);
    proxy->setFilterKeyColumn( 1 );
    proxy->setFilterCaseSensitivity( Qt::CaseInsensitive );
    connect(filter, SIGNAL(textChanged(QString)), proxy, SLOT(setFilterFixedString(QString)));

    SessionChooserDialog dialog(view, proxy, filter);
    view->setEditTriggers(QAbstractItemView::NoEditTriggers);

    QVBoxLayout layout(dialog.mainWidget());
    if(!headerText.isEmpty()) {
        QLabel* heading = new QLabel(headerText);
        QFont font = heading->font();
        font.setBold(true);
        heading->setFont(font);
        layout.addWidget(heading);
    }
    
    model->setColumnCount(3);
    model->setHeaderData(0, Qt::Horizontal,i18n("Identity"));
    model->setHeaderData(1, Qt::Horizontal, i18n("Contents"));
    model->setHeaderData(2, Qt::Horizontal,i18n("State"));

    view->setModel(proxy);
    view->setModelColumn(1);

    QHBoxLayout* filterLayout = new QHBoxLayout();
    filterLayout->addWidget(new QLabel(i18n("Filter:")));
    filterLayout->addWidget(filter);
    layout.addLayout(filterLayout);
    layout.addWidget(view);
    filter->setFocus();

    int row = 0;
    int defaultRow = 0;
    
    QString defaultSession = KGlobal::config()->group( cfgSessionGroup() ).readEntry( cfgActiveSessionEntry(), "default" );
    
    
    foreach(const KDevelop::SessionInfo& si, KDevelop::SessionController::availableSessionInfo())
    {
        if ( si.name.isEmpty() && si.projects.isEmpty() ) {
            continue;
        }

        bool running = KDevelop::SessionController::tryLockSession(si.uuid.toString());
        
        if(onlyRunning && !running)
            continue;

        if(si.uuid.toString() == defaultSession)
            defaultRow = row;
        
        model->setItem(row, 0, new QStandardItem(si.uuid.toString()));
        model->setItem(row, 1, new QStandardItem(si.description));
        model->setItem(row, 2, new QStandardItem);
        
        if(defaultRow == row && running)
            ++defaultRow;
        
        ++row;
    }
    model->sort(1);

    int cnsRow = row;
    if(!onlyRunning) {
        model->setItem(row, 0, new QStandardItem);
        model->setItem(row, 1, new QStandardItem(KIcon("window-new"), i18n("Create New Session")));
    }

    dialog.updateState();
    dialog.mainWidget()->layout()->setContentsMargins(0,0,0,0);
    
    view->selectionModel()->setCurrentIndex(proxy->mapFromSource(model->index(defaultRow, 0)), QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
    view->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    ///@todo We need a way to get a proper size-hint from the view, but unfortunately, that only seems possible after the view was shown.
    dialog.setInitialSize(QSize(900, 600));

    if(dialog.exec() != QDialog::Accepted)
    {
        return QString();
    }
    
    QModelIndex selected = view->selectionModel()->currentIndex();
    if(selected.isValid())
    {
        QString ret;
        if( selected.row() == cnsRow ) {
            qsrand(QDateTime::currentDateTime().toTime_t());
            ret = QUuid::createUuid().toString();
        } else {
            selected = selected.sibling(selected.row(), 0);
            ret = selected.data().toString();
        }
        return ret;
    }

    return QString();
}

QString SessionController::handleLockedSession( const QString& sessionName, const QString& sessionId,
                                                const SessionController::LockSessionState& state )
{
    if( state ) {
        return sessionId;
    }

    // Set to true if lock-file is not free
    bool lockFileOwned = state.lockResult != KLockFile::LockOK;

    // Set the locking problem description.
    QString problemDescription;
    if( state.success ) {
        // Do not try to call D-Bus if the D-Bus service is free (unregistered).
        switch( state.lockResult ) {
            case KLockFile::LockStale:
                problemDescription = i18nc("@info", "The session lock-file is stale.");
                break;

            case KLockFile::LockError:
                problemDescription = i18nc("@info", "Error while taking the session lock-file.");
                break;

            case KLockFile::LockFail:
                problemDescription = i18nc("@info", "The session lock-file is owned.");
                break;

            case KLockFile::LockOK:
            default:
                // We shouldn't have both D-Bus service name and lockfile free by now.
                Q_ASSERT( false );
                break;
        }
    } else {
        // The timeout for "ensureVisible" call
        // Leave it sufficiently low to avoid waiting for hung instances.
        static const int timeout_ms = 1000;

        QDBusMessage message = QDBusMessage::createMethodCall( state.DBusService,
                                                               "/kdevelop/MainWindow",
                                                               "org.kdevelop.MainWindow",
                                                               "ensureVisible" );
        QDBusMessage reply = QDBusConnection::sessionBus().call( message,
                                                                 QDBus::Block,
                                                                 timeout_ms );
        if( reply.type() == QDBusMessage::ReplyMessage ) {
            kDebug() << i18nc( "@info:shell", "made running %1 instance (PID: %2) visible", state.holderApp, state.holderPid );
            return QString();
        } else {
            kDebug() << i18nc("@info:shell", "running %1 instance (PID: %2) is apparently hung", state.holderApp, state.holderPid);
        }

        problemDescription = i18nc("@info",
                                   "The given application did not respond to a DBUS call, "
                                   "it may have crashed or is hanging.");
    }

    QString problemHeader;
    if( lockFileOwned ) {
        problemHeader = i18nc("@info", "Failed to lock the session <em>%1</em>, "
                              "already locked by %2 on %3 (PID %4).",
                              sessionName, state.holderApp, state.holderHostname, state.holderPid);
    } else {
        problemHeader = i18nc("@info", "Failed to lock the session <em>%1</em> (lock-file unavailable).",
                              sessionName);
    }

    QString problemResolution;
    if( state.success ) {
        problemResolution = i18nc("@info %1 is program name",
                                  "<p>Do you want to remove the lock file and force a new %1 instance?<br/>"
                                  "<strong>Beware:</strong> Only do this if you are sure there is no running"
                                  " process using this session.</p>"
                                  "<p>Otherwise, close the offending application instance "
                                  "or choose another session to launch.</p>",
                                  KCmdLineArgs::aboutData()->programName());
    } else {
        problemResolution = i18nc("@info",
                                  "<p>Please, close the offending application instance "
                                  "or choose another session to launch.</p>");
    }

    QString errmsg = "<p>"
                     + problemHeader
                     + "<br>"
                     + problemDescription
                     + "</p>"
                     + problemResolution;

    KGuiItem retry = KStandardGuiItem::cont();
    if( state.success ) {
        retry.setText(i18nc("@action:button", "Remove lock file"));
    } else {
        retry.setText(i18nc("@action:button", "Retry startup"));
    }
    KGuiItem choose = KStandardGuiItem::configure();
    choose.setText(i18nc("@action:button", "Choose another session"));
    KGuiItem cancel = KStandardGuiItem::quit();
    int ret = KMessageBox::warningYesNoCancel(0, errmsg, i18nc("@title:window", "Failed to Lock Session %1", sessionName),
                                              retry, choose, cancel);
    switch( ret ) {
    case KMessageBox::Yes:
        if( state.success ) {
            state.forceRemoveLockfile();
        }
        return sessionId;
        break;

    case KMessageBox::No: {
        QString errmsg = i18nc("@info", "The session %1 is already active in another running instance.",
                               sessionName);
        return showSessionChooserDialog(errmsg);
        break;
    }

    case KMessageBox::Cancel:
    default:
        break;
    }

    return QString();
}

void SessionChooserDialog::itemEntered(const QModelIndex& index)
{
    // The last row says "Create new session", we don't want to delete that
    if(index.row() == m_model->rowCount()-1) {
        m_deleteButton->hide();
        m_deleteButtonTimer.stop();
        return;
    }

    // align the delete-button to stay on the right border of the item
    // we need the right most column's index
    QModelIndex in = m_model->index( index.row(), 1 );
    const QRect rect = m_view->visualRect(in);
    m_deleteButton->resize(rect.height(), rect.height());
    
    QPoint p(rect.right() - m_deleteButton->size().width(), rect.top()+rect.height()/2-m_deleteButton->height()/2);
    m_deleteButton->move(p);

    m_deleteCandidateRow = index.row();
    m_deleteButtonTimer.start();
}

void SessionChooserDialog::showDeleteButton()
{
    m_deleteButton->show();
}

bool SessionChooserDialog::eventFilter(QObject* object, QEvent* event)
{
    if(object == m_view && event->type() == QEvent::Leave ) {
        m_deleteButtonTimer.stop();
        m_deleteButton->hide();
    }
    if(object == m_filter && event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
        if(keyEvent->key() == Qt::Key_Up || keyEvent->key() == Qt::Key_Down) {
            QModelIndex currentIndex = m_view->selectionModel()->currentIndex();
            int selectRow = -1;
            if(keyEvent->key() == Qt::Key_Up) {
                if(!currentIndex.isValid()) {
                    selectRow = m_model->rowCount()-1;
                } else if(currentIndex.row()-1 >= 0) {
                    selectRow = currentIndex.row()-1;
                }
            } else if(keyEvent->key() == Qt::Key_Down) {
                if(!currentIndex.isValid()) {
                    selectRow = 0;
                } else if(currentIndex.row()+1 < m_model->rowCount()) {
                    selectRow = currentIndex.row()+1;
                }
            }
            if (selectRow != -1) {
                    m_view->selectionModel()->setCurrentIndex(m_model->index(selectRow, 0), QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
            }
            return true;
        }
    }

    return false;
}


void SessionChooserDialog::deleteButtonPressed()
{
    if(m_deleteCandidateRow == -1)
        return;

    QModelIndex index = m_model->index(m_deleteCandidateRow, 0);
    const QString uuid = m_model->data(index, Qt::DisplayRole).toString();

    {
        SessionController::LockSessionState state = SessionController::tryLockSession( uuid );
        if( !state ) {
            const QString errCaption = i18nc("@title", "Cannot Delete Session");
            QString errText = i18nc("@info", "<p>Cannot delete a locked session.");

            if( state.lockResult != KLockFile::LockOK ) {
                errText += i18nc("@info", "<p>The session is locked by %1 on %2 (PID %3).",
                                 state.holderApp, state.holderHostname, state.holderPid);
            }

            KMessageBox::error( this, errText, errCaption );
            return;
        }
    }

    const QString text = i18nc("@info", "The session and all contained settings will be deleted. The projects will stay unaffected. Do you really want to continue?");
    const QString caption = i18nc("@title", "Delete Session");
    const KGuiItem deleteItem = KStandardGuiItem::del();
    const KGuiItem cancelItem = KStandardGuiItem::cancel();

    if(KMessageBox::warningYesNo(this, text, caption, deleteItem, cancelItem) == KMessageBox::Yes) {
        QModelIndex index = m_model->index(m_deleteCandidateRow, 0);
        const QString uuid = m_model->data(index, Qt::DisplayRole).toString();

        //FIXME: What about running sessions?
        KDevelop::Session session( uuid );
        session.deleteFromDisk();

        m_model->removeRows( m_deleteCandidateRow, 1 );
        m_deleteCandidateRow = -1;
    }
}

QString SessionController::sessionDir()
{
    if( !activeSession() )
        return QString();
    return d->ownSessionDirectory();
}

QString SessionController::sessionName()
{
    if(!activeSession())
        return QString();
    return activeSession()->description();
}


}
#include "sessioncontroller.moc"
#include "moc_sessioncontroller.cpp"
