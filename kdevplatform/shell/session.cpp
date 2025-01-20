/*
    SPDX-FileCopyrightText: 2008 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "session.h"

#include <interfaces/iplugincontroller.h>
#include <interfaces/iplugin.h>
#include "core.h"
#include "sessioncontroller.h"
#include <interfaces/iprojectcontroller.h>
#include <interfaces/iproject.h>

#include <KConfigGroup>
#include <KLocalizedString>

#include <QDir>
#include <QRegularExpression>
#include <QUrl>

namespace KDevelop
{

const QString Session::cfgSessionNameEntry = QStringLiteral("SessionName");
const QString Session::cfgSessionDescriptionEntry = QStringLiteral("SessionPrettyContents");
const QString Session::cfgSessionProjectsEntry = QStringLiteral("Open Projects");
const QString Session::cfgSessionOptionsGroup = QStringLiteral("General Options");

class SessionPrivate
{
public:
    SessionInfo info;
    Session* const q;
    bool isTemporary;

    QUrl pluginArea( const IPlugin* plugin )
    {
        QString name = Core::self()->pluginController()->pluginInfo(plugin).pluginId();
        QUrl url = QUrl::fromLocalFile(info.path + QLatin1Char('/') + name );
        if( !QFile::exists( url.toLocalFile() ) ) {
            QDir( info.path ).mkdir( name );
        }
        return url;
    }

    SessionPrivate( Session* session, const QString& id )
        : info( Session::parse( id, true ) )
        , q( session )
        , isTemporary( false )
    {
    }

    void updateDescription()
    {
        buildDescription( info );
        emit q->sessionUpdated( q );
    }

    static QString generatePrettyContents( const SessionInfo& info );
    static QString generateDescription( const SessionInfo& info );
    static void buildDescription( SessionInfo& info );
};

Session::Session( const QString& id, QObject* parent )
        : ISession(parent)
        , d_ptr(new SessionPrivate(this, id))
{
}

Session::~Session() = default;

QString Session::name() const
{
    Q_D(const Session);

    return d->info.name;
}

QList<QUrl> Session::containedProjects() const
{
    Q_D(const Session);

    return d->info.projects;
}

QString Session::description() const
{
    Q_D(const Session);

    return d->info.description;
}

QUrl Session::pluginDataArea( const IPlugin* p )
{
    Q_D(Session);

    return d->pluginArea( p );
}

KSharedConfigPtr Session::config()
{
    Q_D(Session);

    return d->info.config;
}

QUuid Session::id() const
{
    Q_D(const Session);

    return d->info.uuid;
}

void Session::setName( const QString& newname )
{
    Q_D(Session);

    d->info.name = newname;
    d->info.config->group( QString() ).writeEntry( cfgSessionNameEntry, newname );
    d->updateDescription();
}

void Session::setContainedProjects( const QList<QUrl>& projects )
{
    Q_D(Session);

    d->info.projects = projects;
    d->info.config->group( cfgSessionOptionsGroup ).writeEntry( cfgSessionProjectsEntry, projects );
    d->updateDescription();
}

void Session::setTemporary(bool temp)
{
    Q_D(Session);

    d->isTemporary = temp;
}

bool Session::isTemporary() const
{
    Q_D(const Session);

    return d->isTemporary;
}

QString Session::path() const
{
    Q_D(const Session);

    return d->info.path;
}

QString SessionPrivate::generatePrettyContents( const SessionInfo& info )
{
    if( info.projects.isEmpty() )
        return i18n("(no projects)");

    QStringList projectNames;
    projectNames.reserve( info.projects.size() );

    for (const QUrl& url : info.projects) {
        IProject* project = nullptr;
        if( ICore::self() && ICore::self()->projectController() ) {
            project = ICore::self()->projectController()->findProjectForUrl( url );
        }

        if( project ) {
            projectNames << project->name();
        } else {
            QString projectName = url.fileName();
            static const QRegularExpression projectFileExtensionRegex(QStringLiteral("\\.kdev4$"),
                                                                      QRegularExpression::CaseInsensitiveOption);
            projectName.remove(projectFileExtensionRegex);
            projectNames << projectName;
        }
    }

    if( projectNames.isEmpty() ) {
        return i18n("(no projects)");
    } else {
        return projectNames.join(QLatin1String(", "));
    }
}

QString SessionPrivate::generateDescription( const SessionInfo& info )
{
    QString prettyContentsFormatted = generatePrettyContents( info );
    QString description;

    if( info.name.isEmpty() ) {
        description = prettyContentsFormatted;
    } else {
        description = info.name + QLatin1String(":  ") + prettyContentsFormatted;
    }

    return description;
}

void SessionPrivate::buildDescription( SessionInfo& info )
{
    QString description = generateDescription( info );

    info.description = description;
    info.config->group( QString() ).writeEntry( Session::cfgSessionDescriptionEntry, description );
    info.config->sync();
}

SessionInfo Session::parse( const QString& id, bool mkdir )
{
    SessionInfo ret;
    QString sessionPath = SessionController::sessionDirectory(id);

    QDir sessionDir( sessionPath );
    if( !sessionDir.exists() ) {
        if( mkdir ) {
            sessionDir.mkpath(sessionPath);
            Q_ASSERT( sessionDir.exists() );
        } else {
            return ret;
        }
    }

    ret.uuid = QUuid(id);
    ret.path = sessionPath;
    ret.config = KSharedConfig::openConfig(sessionPath + QLatin1String("/sessionrc"));

    KConfigGroup cfgRootGroup = ret.config->group( QString() );
    KConfigGroup cfgOptionsGroup = ret.config->group( cfgSessionOptionsGroup );

    ret.name = cfgRootGroup.readEntry( cfgSessionNameEntry, QString() );
    ret.projects = cfgOptionsGroup.readEntry( cfgSessionProjectsEntry, QList<QUrl>() );
    SessionPrivate::buildDescription( ret );

    return ret;
}

}

#include "moc_session.cpp"
