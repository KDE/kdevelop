/*
    SPDX-FileCopyrightText: 2008 Andreas Pakulat <apaku@gmx.de>
    SPDX-FileCopyrightText: 2010 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "sessioncontroller.h"

#include <QDir>
#include <QHash>
#include <QStringList>
#include <QAction>
#include <QActionGroup>
#include <QCoreApplication>
#include <QDBusConnection>
#include <QInputDialog>
#include <QLabel>
#include <QLineEdit>
#include <QListView>
#include <QSortFilterProxyModel>
#include <QStandardItemModel>
#include <QVBoxLayout>

#include <KActionCollection>
#include <KConfigGroup>
#include <KIO/CopyJob>
#include <KJobWidgets>
#include <KLocalizedString>
#include <KMessageBox>
#include <KProcess>
#include <KStringHandler>

#include "session.h"
#include "core.h"
#include "uicontroller.h"
#include "shellextension.h"
#include "sessionlock.h"
#include "sessionchooserdialog.h"
#include "debug.h"

#include <sublime/mainwindow.h>
#include <serialization/itemrepositoryregistry.h>
#include <duchain/duchain.h>

namespace KDevelop
{

namespace {
    int argc = 0;
    char** argv = nullptr;
}

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
        if(arg.startsWith(QLatin1String("-graphicssystem")) || arg.startsWith(QLatin1String("-style")))
        {
            ret << QLatin1Char('-') + arg;
            if(a+1 < argc)
                ret << QString::fromLocal8Bit(argv[a+1]);
        }
    }
    return ret;
}

class SessionControllerPrivate : public QObject
{
    Q_OBJECT
public:
    explicit SessionControllerPrivate( SessionController* s )
        : q(s)
        , activeSession(nullptr)
        , grp(nullptr)
    {
    }

    ~SessionControllerPrivate() override {
    }

    Session* findSessionForName( const QString& name ) const
    {
        for (auto it = sessionActions.begin(), end = sessionActions.end(); it != end; ++it) {
            Session* s = it.key();
            if( s->name() == name )
                return s;
        }
        return nullptr;
    }

    Session* findSessionForId(const QString& idString) const
    {
        QUuid id(idString);

        for (auto it = sessionActions.begin(), end = sessionActions.end(); it != end; ++it) {
            Session* s = it.key();
            if( s->id() == id)
                return s;
        }
        return nullptr;
    }

    void newSession(const QString& name = {})
    {
        auto* session = new Session(QUuid::createUuid().toString());
        if (!name.isEmpty()) {
            session->setName(name);
        }

        KProcess::startDetached(ShellExtension::getInstance()->executableFilePath(), QStringList() << QStringLiteral("-s") << session->id().toString() << standardArguments());
        delete session;
#if 0
        //Terminate this instance of kdevelop if the user agrees
        const auto windows = Core::self()->uiController()->controller()->mainWindows();
        for (Sublime::MainWindow* window : windows) {
            window->close();
        }
#endif
    }

    void newNamedSession()
    {
        bool ok = false;
        const QString& newSessionName = QInputDialog::getText(
            Core::self()->uiController()->activeMainWindow(), i18nc("@title:window", "Start New Session"),
            i18nc("@label:textbox", "New session name:"), QLineEdit::Normal, {}, &ok);

        if (ok) {
            newSession(newSessionName);
        }
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
        bool ok;
        auto newSessionName = QInputDialog::getText(Core::self()->uiController()->activeMainWindow(),
                                                    i18nc("@title:window", "Rename Session"), i18nc("@label:textbox", "New session name:"),
                                                    QLineEdit::Normal, q->activeSession()->name(), &ok);
        if (ok) {
            static_cast<Session*>(q->activeSession())->setName(newSessionName);
        }

        q->updateXmlGuiActionList(); // resort
    }

    bool loadSessionExternally( Session* s )
    {
        Q_ASSERT( s );
        KProcess::startDetached(ShellExtension::getInstance()->executableFilePath(), QStringList() << QStringLiteral("-s") << s->id().toString() << standardArguments());
        return true;
    }

    TryLockSessionResult activateSession( Session* s )
    {
        Q_ASSERT( s );

        activeSession = s;
        TryLockSessionResult result = SessionController::tryLockSession( s->id().toString());
        if( !result.lock ) {
            activeSession = nullptr;
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

    void loadSessionFromAction(QAction* action)
    {
        auto session = action->data().value<Session*>();
        loadSessionExternally(session);
    }

    void addSession( Session* s )
    {
        if (Core::self()->setupFlags() & Core::NoUi) {
            sessionActions[s] = nullptr;
            return;
        }

        auto* a = new QAction( grp );
        a->setText( s->description() );
        a->setToolTip(s->description());
        a->setCheckable( false );
        a->setData(QVariant::fromValue<Session*>(s));

        sessionActions[s] = a;
        q->actionCollection()->addAction(QLatin1String("session_") + s->id().toString(), a);
        connect( s, &Session::sessionUpdated, this, &SessionControllerPrivate::sessionUpdated );
        sessionUpdated( s );
    }

    SessionController* const q;

    QHash<Session*, QAction*> sessionActions;
    ISession* activeSession;
    QActionGroup* grp;

    ISessionLock::Ptr sessionLock;

    [[nodiscard]] static const QString& sessionBaseDirectory()
    {
        static const QString path = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation)
            + QLatin1Char('/') + QCoreApplication::applicationName() + QLatin1String("/sessions/");
        return path;
    }

private Q_SLOTS:
    void sessionUpdated( KDevelop::ISession* s )
    {
        sessionActions[static_cast<Session*>( s )]->setText( KStringHandler::rsqueeze(s->description()) );
    }
};


SessionController::SessionController( QObject *parent )
    : QObject(parent)
    , d_ptr(new SessionControllerPrivate(this))
{
    Q_D(SessionController);

    setObjectName(QStringLiteral("SessionController"));
    setComponentName(QStringLiteral("kdevsession"), i18n("Session Manager"));

    setXMLFile(QStringLiteral("kdevsessionui.rc"));

    QDBusConnection::sessionBus().registerObject( QStringLiteral("/org/kdevelop/SessionController"),
        this, QDBusConnection::ExportScriptableSlots );

    if (Core::self()->setupFlags() & Core::NoUi) return;

    QAction* action = actionCollection()->addAction(QStringLiteral("new_session"));
    connect(action, &QAction::triggered,
            this, [this] { Q_D(SessionController); d->newSession(); });
    action->setText( i18nc("@action:inmenu", "Start New Session") );
    action->setToolTip( i18nc("@info:tooltip", "Start a new KDevelop instance with an empty session") );
    action->setIcon(QIcon::fromTheme(QStringLiteral("window-new")));

    action = actionCollection()->addAction(QStringLiteral("new_named_session"));
    connect(action, &QAction::triggered, this, [this] {
        Q_D(SessionController);
        d->newNamedSession();
    });
    action->setText(i18nc("@action:inmenu", "Start New Named Session..."));
    action->setToolTip(i18nc("@info:tooltip", "Start a new KDevelop instance with an empty named session"));
    action->setIcon(QIcon::fromTheme(QStringLiteral("window-new")));

    action = actionCollection()->addAction(QStringLiteral("rename_session"));
    connect(action, &QAction::triggered,
            this, [this] { Q_D(SessionController); d->renameSession(); });
    action->setText( i18nc("@action", "Rename Current Session...") );
    action->setIcon(QIcon::fromTheme(QStringLiteral("edit-rename")));

    action = actionCollection()->addAction(QStringLiteral("delete_session"));
    connect(action, &QAction::triggered,
            this, [this] { Q_D(SessionController); d->deleteCurrentSession(); });
    action->setText( i18nc("@action", "Delete Current Session...") );
    action->setIcon(QIcon::fromTheme(QStringLiteral("edit-delete")));

    action = actionCollection()->addAction( QStringLiteral("quit"), this, SIGNAL(quitSession()) );
    action->setText( i18nc("@action", "Quit") );
    action->setMenuRole( QAction::NoRole ); // OSX: prevent QT from hiding this due to conflict with 'Quit KDevelop...'
    actionCollection()->setDefaultShortcut( action, Qt::CTRL | Qt::Key_Q );
    action->setIcon(QIcon::fromTheme(QStringLiteral("application-exit")));

    d->grp = new QActionGroup( this );
    connect(d->grp, &QActionGroup::triggered,
            this, [this] (QAction* a) { Q_D(SessionController); d->loadSessionFromAction(a); } );
}

SessionController::~SessionController() = default;

void SessionController::startNewSession()
{
    Q_D(SessionController);

    d->newSession();
}

void SessionController::cleanup()
{
    Q_D(SessionController);

    if (d->activeSession) {
        Q_ASSERT(d->activeSession->id().toString() == d->sessionLock->id());

        if (d->activeSession->isTemporary()) {
            deleteSessionFromDisk(d->sessionLock);
        }
        d->activeSession = nullptr;
    }

    d->sessionLock.clear();
    qDeleteAll(d->sessionActions);
    d->sessionActions.clear();
}

void SessionController::initialize( const QString& session )
{
    Q_D(SessionController);

    QDir sessiondir( SessionControllerPrivate::sessionBaseDirectory() );

    const auto sessionDirs = sessiondir.entryList(QDir::AllDirs | QDir::NoDotAndDotDot);
    for (const QString& s : sessionDirs) {
        QUuid id( s );
        if( id.isNull() )
            continue;
        // Only create sessions for directories that represent proper uuid's
        auto* ses = new Session(id.toString(), this);

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

    updateXmlGuiActionList();
}


ISession* SessionController::activeSession() const
{
    Q_D(const SessionController);

    return d->activeSession;
}

ISessionLock::Ptr SessionController::activeSessionLock() const
{
    Q_D(const SessionController);

    return d->sessionLock;
}

void SessionController::loadSession( const QString& nameOrId )
{
    Q_D(SessionController);

    d->loadSessionExternally( session( nameOrId ) );
}

QList<QString> SessionController::sessionNames() const
{
    Q_D(const SessionController);

    QList<QString> l;
    const auto sessions = d->sessionActions.keys();
    l.reserve(sessions.size());
    for(const auto* s : sessions) {
        l << s->name();
    }
    return l;
}

QList< const KDevelop::Session* > SessionController::sessions() const
{
    Q_D(const SessionController);

    QList< const KDevelop::Session* > ret;
    const auto sessions = d->sessionActions.keys();
    ret.reserve(sessions.size());
    // turn to const pointers
    for (const auto* s : sessions) {
        ret << s;
    }
    return ret;
}

Session* SessionController::createSession( const QString& name )
{
    Q_D(SessionController);

    Session* s;
    if(name.startsWith(QLatin1Char('{'))) {
        s = new Session( QUuid(name).toString(), this );
    } else {
        s = new Session( QUuid::createUuid().toString(), this );
        s->setName( name );
    }
    d->addSession( s );
    updateXmlGuiActionList();
    return s;
}

void SessionController::deleteSession( const ISessionLock::Ptr& lock )
{
    Q_D(SessionController);

    Session* s  = session(lock->id());

    QHash<Session*,QAction*>::iterator it = d->sessionActions.find(s);
    Q_ASSERT( it != d->sessionActions.end() );

    unplugActionList( QStringLiteral("available_sessions") );
    actionCollection()->removeAction(*it);
    if (d->grp) { // happens in unit tests
        d->grp->removeAction(*it);
        plugActionList( QStringLiteral("available_sessions"), d->grp->actions() );
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
    qCDebug(SHELL) << "Deleting session:" << lock->id();

    static_cast<SessionLock*>(lock.data())->removeFromDisk();
    ItemRepositoryRegistry::deleteRepositoryFromDisk(DUChain::repositoryPathForSession(lock));
}

void SessionController::loadDefaultSession( const QString& session )
{
    Q_D(SessionController);

    QString load = session;
    if (load.isEmpty()) {
        KConfigGroup grp = KSharedConfig::openConfig()->group( cfgSessionGroup() );
        load = grp.readEntry( cfgActiveSessionEntry(), "default" );
    }

    // Iteratively try to load the session, asking user what to do in case of failure
    // If showForceOpenDialog() returns empty string, stop trying
    do
    {
        Session* s = this->session(load);
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
    Q_D(const SessionController);

    Session* ret = d->findSessionForName( nameOrId );
    if(ret)
        return ret;

    return d->findSessionForId( nameOrId );
}

QString SessionController::cloneSession( const QString& nameOrid )
{
    Q_D(SessionController);

    Session* origSession = session(nameOrid);
    QUuid id = QUuid::createUuid();
    auto copyJob = KIO::copy(QUrl::fromLocalFile(origSession->dataDirectory()),
                             QUrl::fromLocalFile(sessionDirectory( id.toString())));
    KJobWidgets::setWindow(copyJob, Core::self()->uiController()->activeMainWindow());
    copyJob->exec();

    auto* newSession = new Session(id.toString(), this);
    newSession->setName( i18n( "Copy of %1", origSession->name() ) );
    d->addSession(newSession);
    updateXmlGuiActionList();
    return newSession->name();
}

void SessionController::updateXmlGuiActionList()
{
    Q_D(SessionController);

    unplugActionList( QStringLiteral("available_sessions") );

    if (d->grp) {
        auto actions = d->grp->actions();
        std::sort(actions.begin(), actions.end(), [](const QAction* lhs, const QAction* rhs) {
            auto s1 = lhs->data().value<Session*>();
            auto s2 = rhs->data().value<Session*>();
            return QString::localeAwareCompare(s1->description(), s2->description()) < 0;
        });
        plugActionList(QStringLiteral("available_sessions"), actions);
    }
}


QString SessionController::cfgSessionGroup() { return QStringLiteral("Sessions"); }
QString SessionController::cfgActiveSessionEntry() { return QStringLiteral("Active Session ID"); }

SessionInfos SessionController::availableSessionInfos()
{
    SessionInfos sessionInfos;
    const auto sessionDirs = QDir(SessionControllerPrivate::sessionBaseDirectory()).entryList(QDir::AllDirs);
    sessionInfos.reserve(sessionDirs.size());
    for (const QString& sessionId : sessionDirs) {
        if( !QUuid( sessionId ).isNull() ) {
            sessionInfos << Session::parse( sessionId );
        }
    }
    sessionInfos.squeeze();
    return sessionInfos;
}

QString SessionController::sessionDirectory(const QString& sessionId)
{
    return SessionControllerPrivate::sessionBaseDirectory() + sessionId;
}

TryLockSessionResult SessionController::tryLockSession(const QString& id, bool doLocking)
{
    return SessionLock::tryLockSession(id, doLocking);
}

bool SessionController::isSessionRunning(const QString& id)
{
    return sessionRunInfo(id).isRunning;
}

SessionRunInfo SessionController::sessionRunInfo(const QString& id)
{
    return SessionLock::tryLockSession(id, false).runInfo;
}

QString SessionController::showSessionChooserDialog(const QString& headerText, bool onlyRunning)
{
    ///FIXME: move this code into sessiondialog.cpp
    auto* view = new QListView;
    auto* filter = new QLineEdit;
    filter->setClearButtonEnabled( true );
    filter->setPlaceholderText(i18nc("@info:placeholder", "Search..."));

    auto* model = new QStandardItemModel(view);

    auto *proxy = new QSortFilterProxyModel(model);
    proxy->setSourceModel(model);
    proxy->setFilterKeyColumn( 1 );
    proxy->setFilterCaseSensitivity( Qt::CaseInsensitive );
    connect(filter, &QLineEdit::textChanged, proxy, &QSortFilterProxyModel::setFilterFixedString);

    SessionChooserDialog dialog(view, proxy, filter);
    view->setEditTriggers(QAbstractItemView::NoEditTriggers);

    QVBoxLayout layout(dialog.mainWidget());
    if(!headerText.isEmpty()) {
        auto* heading = new QLabel(headerText);
        QFont font = heading->font();
        font.setBold(true);
        heading->setFont(font);
        layout.addWidget(heading);
    }

    model->setColumnCount(4);
    model->setHeaderData(0, Qt::Horizontal,i18nc("@title:column", "Identity"));
    model->setHeaderData(1, Qt::Horizontal,i18nc("@title:column", "Contents"));
    model->setHeaderData(2, Qt::Horizontal,i18nc("@title:column", "State"));
    model->setHeaderData(3, Qt::Horizontal,i18nc("@title:column", "Name"));

    view->setModel(proxy);
    view->setModelColumn(1);

    auto* filterLayout = new QHBoxLayout();
    filterLayout->addWidget(new QLabel(i18nc("@label:textbox", "Filter:")));
    filterLayout->addWidget(filter);
    layout.addLayout(filterLayout);
    layout.addWidget(view);
    filter->setFocus();

    int row = 0;

    QString defaultSession = KSharedConfig::openConfig()->group( cfgSessionGroup() ).readEntry( cfgActiveSessionEntry(), "default" );

    const auto availableSessionInfos = KDevelop::SessionController::availableSessionInfos();
    for (const KDevelop::SessionInfo& si : availableSessionInfos) {
        if ( si.name.isEmpty() && si.projects.isEmpty() ) {
            continue;
        }

        bool running = KDevelop::SessionController::isSessionRunning(si.uuid.toString());

        if(onlyRunning && !running)
            continue;
            
        model->setItem(row, 0, new QStandardItem(si.uuid.toString()));
        model->setItem(row, 1, new QStandardItem(si.description));
        model->setItem(row, 2, new QStandardItem);
        model->setItem(row, 3, new QStandardItem(si.name));

        ++row;
    }
    model->sort(1);

    if(!onlyRunning) {
        model->setItem(row, 0, new QStandardItem);
        model->setItem(row, 1, new QStandardItem(QIcon::fromTheme(QStringLiteral("window-new")), i18n("Create New Session")));
    }

    dialog.updateState();
    dialog.mainWidget()->layout()->setContentsMargins(0,0,0,0);

    const QModelIndex defaultSessionIndex = model->match(model->index(0, 0), Qt::DisplayRole, defaultSession, 1, Qt::MatchExactly).value(0);
    view->selectionModel()->setCurrentIndex(proxy->mapFromSource(defaultSessionIndex), QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
    view->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    ///@todo We need a way to get a proper size-hint from the view, but unfortunately, that only seems possible after the view was shown.
    dialog.resize(QSize(900, 600));

    if(dialog.exec() != QDialog::Accepted) // krazy:exclude=crashy
    {
        return QString();
    }

    QModelIndex selected = view->selectionModel()->currentIndex();
    if (!selected.isValid())
        return QString();

    const QString selectedSessionId = selected.sibling(selected.row(), 0).data().toString();
    if (selectedSessionId.isEmpty()) {
        // "Create New Session" item selected, return a fresh UUID
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
    Q_D(SessionController);

    if( !activeSession() )
        return QString();
    return d->activeSession->dataDirectory();
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
