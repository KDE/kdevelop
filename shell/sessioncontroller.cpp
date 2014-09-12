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
#include <QtCore/QStringList>
#include <QtCore/QTimer>

#include <kglobal.h>
#include <kcmdlineargs.h>
#include <kconfiggroup.h>
#include <kstandarddirs.h>
#include <KLocalizedString>
#include <kio/netaccess.h>
#include <kparts/mainwindow.h>
#include <kactioncollection.h>
#include <kpushbutton.h>
#include <kdebug.h>

#include "session.h"
#include "core.h"
#include "uicontroller.h"
#include "sessiondialog.h"
#include "shellextension.h"
#include "sessionlock.h"
#include "sessionchooserdialog.h"
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
#include <KStringHandler>
#include <QGroupBox>
#include <QBoxLayout>
#include <QStandardItemModel>
#include <QListView>
#include <QHeaderView>
#include <interfaces/idocumentcontroller.h>
#include <serialization/itemrepositoryregistry.h>
#include <ktexteditor/document.h>
#include <QLabel>
#include <QAction>
#include <QSortFilterProxyModel>
#include <QDBusConnectionInterface>

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
        Session* session = new Session( QUuid::createUuid().toString() );
        
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

    void deleteCurrentSession()
    {
        int choice = KMessageBox::warningContinueCancel(Core::self()->uiController()->activeMainWindow(), i18n("The current session and all contained settings will be deleted. The projects will stay unaffected. Do you really want to continue?"));
        
        if(choice == KMessageBox::Continue)
        {
            q->deleteSessionFromDisk(sessionLock);
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
    
    TryLockSessionResult activateSession( Session* s )
    {
        Q_ASSERT( s );

        activeSession = s;
        TryLockSessionResult result = SessionController::tryLockSession( s->id().toString());
        if( !result.lock ) {
            activeSession = 0;
            return result;
        }
        Q_ASSERT(s->id().toString() == result.lock->id());
        sessionLock = result.lock;

        KConfigGroup grp = KSharedConfig::openConfig()->group( SessionController::cfgSessionGroup() );
        grp.writeEntry( SessionController::cfgActiveSessionEntry(), s->id().toString() );
        grp.sync();
        if (Core::self()->setupFlags() & Core::NoUi) return result;

        QHash<Session*,QAction*>::iterator it = sessionActions.find(s);
        Q_ASSERT( it != sessionActions.end() );
        (*it)->setCheckable(true);
        (*it)->setChecked(true);

        for(it = sessionActions.begin(); it != sessionActions.end(); ++it)
        {
            if(it.key() != s)
                (*it)->setCheckable(false);
        }

        return result;
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

        QAction* a = new QAction( grp );
        a->setText( s->description() );
        a->setCheckable( false );
        a->setData( s->id().toString() );

        sessionActions[s] = a;
        q->actionCollection()->addAction( "session_"+s->id().toString(), a );
        q->unplugActionList( "available_sessions" );
        q->plugActionList( "available_sessions", grp->actions() );

        connect( s, SIGNAL(sessionUpdated(KDevelop::ISession*)), SLOT(sessionUpdated(KDevelop::ISession*)) );
        sessionUpdated( s );
    }

    SessionController* q;

    QHash<Session*, QAction*> sessionActions;
    ISession* activeSession;
    QActionGroup* grp;
    
    ISessionLock::Ptr sessionLock;
    
    // Whether this process owns the recovery directory
    bool recoveryDirectoryIsOwn;
    
    QTimer recoveryTimer;
    QMap<KUrl, QStringList > currentRecoveryFiles;

    static QString sessionBaseDirectory()
    {
        return QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) +'/'+ KGlobal::mainComponent().componentName() + "/sessions/";
    }

    QString ownSessionDirectory() const
    {
        Q_ASSERT(activeSession);
        return q->sessionDirectory( activeSession->id().toString() );
    }

    void clearRecoveryDirectory()
    {
        removeDirectory(ownSessionDirectory() + "/recovery");
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
                    recover.setIcon(QIcon::fromTheme("edit-redo"));
                    recover.setText(i18n("Recover"));
                    KGuiItem discard = KStandardGuiItem::discard();
                    int choice = KMessageBox::warningContinueCancelList(qApp->activeWindow(),
                        i18nc("%1: date of the last snapshot",
                              "The session crashed the last time it was used. "
                              "The following modified files can be recovered from a backup from %1.", date),
                        urlList, i18n("Crash Recovery"), recover, discard );

                    if(choice == KMessageBox::Continue)
                    {
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
                                KTextEditor::Document* recovery = doc->textDocument();
                                
                                if(recovery && recovery->isDataRecoveryAvailable())
                                    // Use the recovery from the kate swap-file if possible
                                    recovery->recoverData();
                                else
                                    // Use a simple recovery through "replace text"
                                    doc->textDocument()->setText(text);
                            }
                        }
                    }
                }
            }
        }
        
        recoveryDirectoryIsOwn = true;
    }

    void sessionUpdated( KDevelop::ISession* s )
    {
        sessionActions[static_cast<Session*>( s )]->setText( KStringHandler::rsqueeze(s->description()) );
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


SessionController::SessionController( QObject *parent )
        : QObject( parent ), d(new SessionControllerPrivate(this))
{
    setObjectName("SessionController");
    setComponentName(QStringLiteral("kdevsession"), QStringLiteral("KDevSession"));
    
    setXMLFile("kdevsessionui.rc");

    QDBusConnection::sessionBus().registerObject( "/kdevelop/SessionController",
        this, QDBusConnection::ExportScriptableSlots );

    if (Core::self()->setupFlags() & Core::NoUi) return;

    QAction* action = actionCollection()->addAction( "new_session", this, SLOT(newSession()) );
    action->setText( i18nc("@action:inmenu", "Start New Session") );
    action->setToolTip( i18nc("@info:tooltip", "Start a new KDevelop instance with an empty session") );
    action->setIcon(QIcon::fromTheme("window-new"));

    action = actionCollection()->addAction( "rename_session", this, SLOT(renameSession()) );
    action->setText( i18n("Rename Current Session...") );
    action->setIcon(QIcon::fromTheme("edit-rename"));

    action = actionCollection()->addAction( "delete_session", this, SLOT(deleteCurrentSession()) );
    action->setText( i18n("Delete Current Session...") );
    action->setIcon(QIcon::fromTheme("edit-delete"));

    action = actionCollection()->addAction( "quit", this, SIGNAL(quitSession()) );
    action->setText( i18n("Quit") );
    actionCollection()->setDefaultShortcut( action, Qt::CTRL | Qt::Key_Q );
    action->setIcon(QIcon::fromTheme("application-exit"));

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

    if (d->activeSession) {
        Q_ASSERT(d->activeSession->id().toString() == d->sessionLock->id());

        if (d->activeSession->isTemporary()) {
            deleteSessionFromDisk(d->sessionLock);
        }
        d->activeSession = 0;
    }

    d->sessionLock.clear();
    qDeleteAll(d->sessionActions);
    d->sessionActions.clear();
}

void SessionController::initialize( const QString& session )
{
    QDir sessiondir( SessionControllerPrivate::sessionBaseDirectory() );

    foreach( const QString& s, sessiondir.entryList( QDir::AllDirs | QDir::NoDotAndDotDot ) )
    {
        QUuid id( s );
        if( id.isNull() )
            continue;
        // Only create sessions for directories that represent proper uuid's
        Session* ses = new Session( id.toString(), this );

        //Delete sessions that have no name and are empty
        if( ses->containedProjects().isEmpty() && ses->name().isEmpty()
            && (session.isEmpty() || (ses->id().toString() != session && ses->name() != session)) )
        {
            TryLockSessionResult result = tryLockSession(s);
            if (result.lock) {
                deleteSessionFromDisk(result.lock);
            }
            delete ses;
        } else {
            d->addSession( ses );
        }
    }
    loadDefaultSession( session );
}


ISession* SessionController::activeSession() const
{
    return d->activeSession;
}

ISessionLock::Ptr SessionController::activeSessionLock() const
{
    return d->sessionLock;
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
        s = new Session( QUuid(name).toString() );
    }else{
        qsrand(QDateTime::currentDateTime().toTime_t());
        s = new Session( QUuid::createUuid().toString() );
        s->setName( name );
    }
    d->addSession( s );
    return s;
}

void SessionController::deleteSession( const ISessionLock::Ptr& lock )
{
    Session* s  = session(lock->id());

    QHash<Session*,QAction*>::iterator it = d->sessionActions.find(s);
    Q_ASSERT( it != d->sessionActions.end() );

    unplugActionList( "available_sessions" );
    actionCollection()->removeAction(*it);
    if (d->grp) { // happens in unit tests
        d->grp->removeAction(*it);
        plugActionList( "available_sessions", d->grp->actions() );
    }

    if (s == d->activeSession) {
        d->activeSession = nullptr;
    }
    deleteSessionFromDisk(lock);

    emit sessionDeleted( s->id().toString() );
    d->sessionActions.remove(s);
    delete s;
}

void SessionController::deleteSessionFromDisk( const ISessionLock::Ptr& lock )
{
    removeDirectory( sessionDirectory(lock->id()) );
    ItemRepositoryRegistry::deleteRepositoryFromDisk( lock );
}

void SessionController::loadDefaultSession( const QString& session )
{
    QString load = session;
    if (load.isEmpty()) {
        KConfigGroup grp = KSharedConfig::openConfig()->group( cfgSessionGroup() );
        load = grp.readEntry( cfgActiveSessionEntry(), "default" );
    }

    // Iteratively try to load the session, asking user what to do in case of failure
    // If showForceOpenDialog() returns empty string, stop trying
    Session* s = 0;
    do
    {
        s = this->session( load );
        if( !s ) {
            s = createSession( load );
        }
        TryLockSessionResult result = d->activateSession( s );
        if( result.lock ) {
            Q_ASSERT(d->activeSession == s);
            Q_ASSERT(d->sessionLock = result.lock);
            break;
        }
        load = handleLockedSession( s->name(), s->id().toString(), result.runInfo );
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
    KIO::NetAccess::dircopy( QUrl::fromLocalFile(sessionDirectory( origSession->id().toString() )),
                             QUrl::fromLocalFile(sessionDirectory( id.toString() )),
                             Core::self()->uiController()->activeMainWindow() );
    Session* newSession = new Session( id.toString() );
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
    foreach( const QString& sessionId, QDir( SessionControllerPrivate::sessionBaseDirectory() ).entryList( QDir::AllDirs ) ) {
        if( !QUuid( sessionId ).isNull() ) {
            available << Session::parse( sessionId );
        }
    }
    return available;
}

QString SessionController::sessionDirectory(const QString& sessionId)
{
    return SessionControllerPrivate::sessionBaseDirectory() + sessionId;
}

TryLockSessionResult SessionController::tryLockSession(const QString& id)
{
    return SessionLock::tryLockSession(id, true);
}

bool SessionController::isSessionRunning(const QString& id)
{
    return sessionRunInfo(id).isRunning;
}

SessionRunInfo SessionController::sessionRunInfo(const QString& id)
{
    return SessionLock::tryLockSession(id, false).runInfo;
}

QString SessionController::showSessionChooserDialog(QString headerText, bool onlyRunning)
{
    ///FIXME: move this code into sessiondialog.cpp
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

    QString defaultSession = KSharedConfig::openConfig()->group( cfgSessionGroup() ).readEntry( cfgActiveSessionEntry(), "default" );

    foreach(const KDevelop::SessionInfo& si, KDevelop::SessionController::availableSessionInfo())
    {
        if ( si.name.isEmpty() && si.projects.isEmpty() ) {
            continue;
        }

        bool running = KDevelop::SessionController::isSessionRunning(si.uuid.toString());

        if(onlyRunning && !running)
            continue;

        model->setItem(row, 0, new QStandardItem(si.uuid.toString()));
        model->setItem(row, 1, new QStandardItem(si.description));
        model->setItem(row, 2, new QStandardItem);

        ++row;
    }
    model->sort(1);

    if(!onlyRunning) {
        model->setItem(row, 0, new QStandardItem);
        model->setItem(row, 1, new QStandardItem(QIcon::fromTheme("window-new"), i18n("Create New Session")));
    }

    dialog.updateState();
    dialog.mainWidget()->layout()->setContentsMargins(0,0,0,0);

    const QModelIndex defaultSessionIndex = model->match(model->index(0, 0), Qt::DisplayRole, defaultSession, 1, Qt::MatchExactly).value(0);
    view->selectionModel()->setCurrentIndex(proxy->mapFromSource(defaultSessionIndex), QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
    view->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    ///@todo We need a way to get a proper size-hint from the view, but unfortunately, that only seems possible after the view was shown.
    dialog.setInitialSize(QSize(900, 600));

    if(dialog.exec() != QDialog::Accepted)
    {
        return QString();
    }

    QModelIndex selected = view->selectionModel()->currentIndex();
    if (!selected.isValid())
        return QString();

    const QString selectedSessionId = selected.sibling(selected.row(), 0).data().toString();
    if (selectedSessionId.isEmpty()) {
        // "Create New Session" item selected, return a fresh UUID
        qsrand(QDateTime::currentDateTime().toTime_t());
        return QUuid::createUuid().toString();
    }
    return selectedSessionId;
}

QString SessionController::handleLockedSession( const QString& sessionName, const QString& sessionId,
                                                const SessionRunInfo& runInfo )
{
    return SessionLock::handleLockedSession(sessionName, sessionId, runInfo);
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
