/*
    SPDX-FileCopyrightText: 2007 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "builderjob.h"
#include "debug.h"

#include <KConfigGroup>
#include <KLocalizedString>

#include <interfaces/iproject.h>
#include <interfaces/icore.h>
#include <interfaces/isession.h>
#include <interfaces/idocumentcontroller.h>
#include <interfaces/idocument.h>
#include <project/projectmodel.h>
#include <project/interfaces/iprojectbuilder.h>
#include <project/interfaces/ibuildsystemmanager.h>

using namespace KDevelop;

struct SubJobData
{
    BuilderJob::BuildType type;
    KJob* job;
    ProjectBaseItem* item;
};
Q_DECLARE_TYPEINFO(SubJobData, Q_MOVABLE_TYPE);

namespace KDevelop
{
class BuilderJobPrivate
{
public:
    explicit BuilderJobPrivate( BuilderJob* job )
        : q(job)
        , failOnFirstError(true)
    {
    }

    BuilderJob* q;

    void addJob( BuilderJob::BuildType, ProjectBaseItem* );
    bool failOnFirstError;

    QString buildTypeToString( BuilderJob::BuildType type ) const;

    bool hasJobForProject( BuilderJob::BuildType type, IProject* project ) const
    {
        for (const SubJobData& data : m_metadata) {
            if (data.type == type && data.item->project() == project) {
                return true;
            }
        }
        return false;
    }

    /**
     * a structure to keep metadata of all registered jobs
     */
    QVector<SubJobData> m_metadata;

    /**
     * get the subjob list and clear this composite job
     */
    QVector<SubJobData> takeJobList();
};
}

QString BuilderJobPrivate::buildTypeToString(BuilderJob::BuildType type) const
{
    switch( type ) {
        case BuilderJob::Build:
            return i18nc( "@info:status", "build" );
        case BuilderJob::Clean:
            return i18nc( "@info:status", "clean" );
        case BuilderJob::Configure:
            return i18nc( "@info:status", "configure" );
        case BuilderJob::Install:
            return i18nc( "@info:status", "install" );
        case BuilderJob::Prune:
            return i18nc( "@info:status", "prune" );
    }
    return QString();
}

void BuilderJobPrivate::addJob( BuilderJob::BuildType t, ProjectBaseItem* item )
{
    Q_ASSERT(item);
    qCDebug(PROJECT) << "adding build job for item:" << item->text();
    Q_ASSERT(item->project());
    qCDebug(PROJECT) << "project for item:" << item->project()->name();
    Q_ASSERT(item->project()->projectItem());
    qCDebug(PROJECT) << "project item for the project:" << item->project()->projectItem()->text();
    if( !item->project()->buildSystemManager() )
    {
        qCWarning(PROJECT) << "no buildsystem manager for:" << item->text() << item->project()->name();
        return;
    }
    qCDebug(PROJECT) << "got build system manager";
    Q_ASSERT(item->project()->buildSystemManager()->builder());
    KJob* j = nullptr;
    switch( t )
    {
        case BuilderJob::Build:
            j = item->project()->buildSystemManager()->builder()->build( item );
            break;
        case BuilderJob::Clean:
            j = item->project()->buildSystemManager()->builder()->clean( item );
            break;
        case BuilderJob::Install:
            j = item->project()->buildSystemManager()->builder()->install( item );
            break;
        case BuilderJob::Prune:
            if (!hasJobForProject(t, item->project())) {
                j = item->project()->buildSystemManager()->builder()->prune( item->project() );
            }
            break;
        case BuilderJob::Configure:
            if (!hasJobForProject(t, item->project())) {
                j = item->project()->buildSystemManager()->builder()->configure( item->project() );
            }
            break;
    }
    if( j )
    {
        q->addCustomJob( t, j, item );
    }
}

BuilderJob::BuilderJob()
    : d_ptr(new BuilderJobPrivate(this))
{
}

BuilderJob::~BuilderJob() = default;

void BuilderJob::addItems( BuildType t, const QList<ProjectBaseItem*>& items )
{
    Q_D(BuilderJob);

    for (ProjectBaseItem* item : items) {
        d->addJob( t, item );
    }
}

void BuilderJob::addProjects( BuildType t, const QList<IProject*>& projects )
{
    Q_D(BuilderJob);

    for (IProject* project : projects) {
        d->addJob( t, project->projectItem() );
    }
}

void BuilderJob::addItem( BuildType t, ProjectBaseItem* item )
{
    Q_D(BuilderJob);

    d->addJob( t, item );
}

void BuilderJob::addCustomJob( BuilderJob::BuildType type, KJob* job, ProjectBaseItem* item )
{
    Q_D(BuilderJob);

    if (auto* builderJob = qobject_cast<BuilderJob*>(job)) {
        // If a subjob is a builder job itself, re-own its job list to avoid having recursive composite jobs.
        const QVector<SubJobData> subjobs = builderJob->d_func()->takeJobList();
        builderJob->deleteLater();
        for (const SubJobData& subjob : subjobs) {
            subjob.job->setParent(this);
            addSubjob( subjob.job );
        }
        d->m_metadata << subjobs;
    } else {
        job->setParent(this);
        addSubjob( job );

        SubJobData data;
        data.type = type;
        data.job = job;
        data.item = item;
        d->m_metadata << data;
    }
}

QVector< SubJobData > BuilderJobPrivate::takeJobList()
{
    QVector< SubJobData > ret = m_metadata;
    m_metadata.clear();
    q->clearSubjobs();
    q->setObjectName( QString() );
    return ret;
}

void BuilderJob::updateJobName()
{
    Q_D(BuilderJob);

    // Which items are mentioned in the set
    // Make it a list to preserve ordering; search overhead (n^2) isn't too big
    QList< ProjectBaseItem* > registeredItems;
    // Which build types are mentioned in the set
    // (Same rationale applies)
    QList< BuildType > buildTypes;
    // Whether there are jobs without any specific item
    bool hasNullItems = false;

    for (const SubJobData& subjob : std::as_const(d->m_metadata)) {
        if( subjob.item ) {
            if( !registeredItems.contains( subjob.item ) ) {
                registeredItems.append( subjob.item );
            }
            if( !buildTypes.contains( subjob.type ) ) {
                buildTypes.append( subjob.type );
            }
        } else {
            hasNullItems = true;
        }
    }

    QString itemNames;
    if( !hasNullItems ) {
        QStringList itemNamesList;
        itemNamesList.reserve(registeredItems.size());
        for (ProjectBaseItem* item : std::as_const(registeredItems)) {
            itemNamesList << item->text();
        }
        itemNames = itemNamesList.join(QLatin1String(", "));
    } else {
        itemNames = i18nc( "Unspecified set of build items (e. g. projects, targets)", "Various items" );
    }

    QString methodNames;
    QStringList methodNamesList;
    methodNamesList.reserve(buildTypes.size());
    for (BuildType type : std::as_const(buildTypes)) {
        methodNamesList << d->buildTypeToString( type );
    }
    methodNames = methodNamesList.join(QLatin1String(", "));

    QString jobName = QStringLiteral( "%1: %2" ).arg( itemNames, methodNames );
    setObjectName( jobName );
}

void BuilderJob::start()
{
    // Automatically save all documents before starting to build
    // might need an option to turn off at some point
    // Also should be moved into the builder and there try to find target(s) for the given item and then just save the documents of that target -> list??
    if (ICore::self()->activeSession()->config()->group(QStringLiteral("Project Manager")).readEntry("Save All Documents Before Building", true))
    {
        ICore::self()->documentController()->saveAllDocuments(IDocumentController::SaveSelectionMode::DontAskUser);
    }

    ExecuteCompositeJob::start();
}

#include "moc_builderjob.cpp"
