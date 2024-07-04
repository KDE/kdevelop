/*
    SPDX-FileCopyrightText: 2007 Dukju Ahn <dukjuahn@gmail.com>
    SPDX-FileCopyrightText: 2008 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kdevsvnplugin.h"

#include <QAction>
#include <QDialog>
#include <QMenu>
#include <QVBoxLayout>
#include <QVariant>

#include <KFile>
#include <KIO/Global>
#include <KLocalizedString>
#include <KMessageBox>
#include <KPluginFactory>
#include <KUrlRequester>
#include <KUrlRequesterDialog>

#include <interfaces/idocument.h>
#include <interfaces/icore.h>
#include <interfaces/iruncontroller.h>
#include <project/projectmodel.h>
#include <interfaces/context.h>
#include <interfaces/contextmenuextension.h>
#include <vcs/vcsrevision.h>
#include <vcs/vcsevent.h>
#include <vcs/vcsstatusinfo.h>
#include <vcs/vcsannotation.h>
#include <vcs/widgets/vcsdiffwidget.h>
#include <vcs/widgets/vcscommitdialog.h>

#include "kdevsvncpp/apr.hpp"

#include "svncommitjob.h"
#include "svnstatusjob.h"
#include "svnaddjob.h"
#include "svnrevertjob.h"
#include "svnremovejob.h"
#include "svnupdatejob.h"
#include "svninfojob.h"
#include "svndiffjob.h"
#include "svncopyjob.h"
#include "svnmovejob.h"
#include "svnlogjob.h"
#include "svnblamejob.h"
#include "svnimportjob.h"
#include "svncheckoutjob.h"

#include "svnimportmetadatawidget.h"
#include <vcs/vcspluginhelper.h>
#include <vcs/widgets/standardvcslocationwidget.h>
#include "svnlocationwidget.h"
#include "debug.h"

K_PLUGIN_FACTORY_WITH_JSON(KDevSvnFactory, "kdevsubversion.json", registerPlugin<KDevSvnPlugin>();)

KDevSvnPlugin::KDevSvnPlugin(QObject* parent, const KPluginMetaData& metaData, const QVariantList&)
    : KDevelop::IPlugin(QStringLiteral("kdevsubversion"), parent, metaData)
    , m_common(new KDevelop::VcsPluginHelper(this, this))
    , copy_action(nullptr)
    , move_action(nullptr)
    , m_jobQueue(new ThreadWeaver::Queue(this))
{
    qRegisterMetaType<KDevelop::VcsStatusInfo>();
    qRegisterMetaType<SvnInfoHolder>();
    qRegisterMetaType<KDevelop::VcsEvent>();
    qRegisterMetaType<KDevelop::VcsRevision>();
    qRegisterMetaType<KDevelop::VcsRevision::RevisionSpecialType>();
    qRegisterMetaType<KDevelop::VcsAnnotation>();
    qRegisterMetaType<KDevelop::VcsAnnotationLine>();
}

KDevSvnPlugin::~KDevSvnPlugin()
{
}

bool KDevSvnPlugin::isValidRemoteRepositoryUrl(const QUrl& remoteLocation)
{
    const QString scheme = remoteLocation.scheme();
    if (scheme == QLatin1String("svn") ||
        scheme == QLatin1String("svn+ssh")) {
        return true;
    }
    return false;
}


bool KDevSvnPlugin::isVersionControlled(const QUrl &localLocation)
{
    ///TODO: also check this in the other functions?
    if (!localLocation.isValid()) {
        return false;
    }

    auto* job = new SvnInfoJob(this);

    job->setLocation(localLocation);

    if (job->exec()) {
        QVariant result = job->fetchResults();

        if (result.isValid()) {
            SvnInfoHolder h = result.value<SvnInfoHolder>();
            return !h.name.isEmpty();
        }
    } else {
        qCDebug(PLUGIN_SVN) << "Couldn't execute job";
    }

    return false;
}

KDevelop::VcsJob* KDevSvnPlugin::repositoryLocation(const QUrl &localLocation)
{
    auto* job = new SvnInfoJob(this);

    job->setLocation(localLocation);
    job->setProvideInformation(SvnInfoJob::RepoUrlOnly);
    return job;
}

KDevelop::VcsJob* KDevSvnPlugin::status(const QList<QUrl>& localLocations,
                                        KDevelop::IBasicVersionControl::RecursionMode mode)
{
    auto* job = new SvnStatusJob(this);
    job->setLocations(localLocations);
    job->setRecursive((mode == KDevelop::IBasicVersionControl::Recursive));
    return job;
}

KDevelop::VcsJob* KDevSvnPlugin::add(const QList<QUrl>& localLocations,
                                     KDevelop::IBasicVersionControl::RecursionMode recursion)
{
    auto* job = new SvnAddJob(this);
    job->setLocations(localLocations);
    job->setRecursive((recursion == KDevelop::IBasicVersionControl::Recursive));
    return job;
}

KDevelop::VcsJob* KDevSvnPlugin::remove(const QList<QUrl>& localLocations)
{
    auto* job = new SvnRemoveJob(this);
    job->setLocations(localLocations);
    return job;
}

KDevelop::VcsJob* KDevSvnPlugin::edit(const QUrl& /*localLocation*/)
{
    return nullptr;
}

KDevelop::VcsJob* KDevSvnPlugin::unedit(const QUrl& /*localLocation*/)
{
    return nullptr;
}

KDevelop::VcsJob* KDevSvnPlugin::localRevision(const QUrl &localLocation, KDevelop::VcsRevision::RevisionType type)
{
    auto* job = new SvnInfoJob(this);

    job->setLocation(localLocation);
    job->setProvideInformation(SvnInfoJob::RevisionOnly);
    job->setProvideRevisionType(type);
    return job;
}

KDevelop::VcsJob* KDevSvnPlugin::copy(const QUrl &localLocationSrc, const QUrl& localLocationDstn)
{
    auto* job = new SvnCopyJob(this);
    job->setSourceLocation(localLocationSrc);
    job->setDestinationLocation(localLocationDstn);
    return job;
}

KDevelop::VcsJob* KDevSvnPlugin::move(const QUrl &localLocationSrc, const QUrl& localLocationDst)
{
    auto* job = new SvnMoveJob(this);
    job->setSourceLocation(localLocationSrc);
    job->setDestinationLocation(localLocationDst);
    return job;
}

KDevelop::VcsJob* KDevSvnPlugin::revert(const QList<QUrl>& localLocations,
                                        KDevelop::IBasicVersionControl::RecursionMode recursion)
{
    auto* job = new SvnRevertJob(this);
    job->setLocations(localLocations);
    job->setRecursive((recursion == KDevelop::IBasicVersionControl::Recursive));
    return job;
}

KDevelop::VcsJob* KDevSvnPlugin::update(const QList<QUrl>& localLocations,
                                        const KDevelop::VcsRevision& rev,
                                        KDevelop::IBasicVersionControl::RecursionMode recursion)
{
    auto* job = new SvnUpdateJob(this);
    job->setLocations(localLocations);
    job->setRevision(rev);
    job->setRecursive((recursion == KDevelop::IBasicVersionControl::Recursive));
    return job;
}

KDevelop::VcsJob* KDevSvnPlugin::commit(const QString& message, const QList<QUrl>& localLocations,
                                        KDevelop::IBasicVersionControl::RecursionMode recursion)
{
    auto* job = new SvnCommitJob(this);
    qCDebug(PLUGIN_SVN) << "Committing locations:" << localLocations;
    job->setUrls(localLocations);
    job->setCommitMessage(message) ;
    job->setRecursive((recursion == KDevelop::IBasicVersionControl::Recursive));
    return job;
}

KDevelop::VcsJob* KDevSvnPlugin::diff(const QUrl &fileOrDirectory,
                                      const KDevelop::VcsRevision& srcRevision,
                                      const KDevelop::VcsRevision& dstRevision,
                                      KDevelop::IBasicVersionControl::RecursionMode recurse)
{
    KDevelop::VcsLocation loc(fileOrDirectory);
    return diff2(loc, loc, srcRevision, dstRevision, recurse);
}

KDevelop::VcsJob* KDevSvnPlugin::diff2(const KDevelop::VcsLocation& src,
                                       const KDevelop::VcsLocation& dst,
                                       const KDevelop::VcsRevision& srcRevision,
                                       const KDevelop::VcsRevision& dstRevision,
                                       KDevelop::IBasicVersionControl::RecursionMode recurse)
{
    auto* job = new SvnDiffJob(this);
    job->setSource(src);
    job->setDestination(dst);
    job->setSrcRevision(srcRevision);
    job->setDstRevision(dstRevision);
    job->setRecursive((recurse == KDevelop::IBasicVersionControl::Recursive));
    return job;
}

KDevelop::VcsJob* KDevSvnPlugin::log(const QUrl &localLocation, const KDevelop::VcsRevision& rev, unsigned long limit)
{
    auto* job = new SvnLogJob(this);
    job->setLocation(localLocation);
    job->setStartRevision(rev);
    job->setLimit(limit);
    return job;
}

KDevelop::VcsJob* KDevSvnPlugin::log(const QUrl &localLocation,
                                     const KDevelop::VcsRevision& startRev,
                                     const KDevelop::VcsRevision& endRev)
{
    auto* job = new SvnLogJob(this);
    job->setLocation(localLocation);
    job->setStartRevision(startRev);
    job->setEndRevision(endRev);
    return job;
}

KDevelop::VcsJob* KDevSvnPlugin::annotate(const QUrl &localLocation,
        const KDevelop::VcsRevision& rev)
{
    auto* job = new SvnBlameJob(this);
    job->setLocation(localLocation);
    job->setEndRevision(rev);
    return job;
}

KDevelop::VcsJob* KDevSvnPlugin::merge(const KDevelop::VcsLocation& localOrRepoLocationSrc,
                                       const KDevelop::VcsLocation& localOrRepoLocationDst,
                                       const KDevelop::VcsRevision& srcRevision,
                                       const KDevelop::VcsRevision& dstRevision,
                                       const QUrl &localLocation)
{
    // TODO implement merge
    Q_UNUSED(localOrRepoLocationSrc)
    Q_UNUSED(localOrRepoLocationDst)
    Q_UNUSED(srcRevision)
    Q_UNUSED(dstRevision)
    Q_UNUSED(localLocation)
    return nullptr;
}

KDevelop::VcsJob* KDevSvnPlugin::resolve(const QList<QUrl>& /*localLocations*/,
        KDevelop::IBasicVersionControl::RecursionMode /*recursion*/)
{
    return nullptr;
}

KDevelop::VcsJob* KDevSvnPlugin::import(const QString & commitMessage, const QUrl &sourceDirectory, const KDevelop::VcsLocation & destinationRepository)
{
    auto* job = new SvnImportJob(this);
    job->setMapping(sourceDirectory, destinationRepository);
    job->setMessage(commitMessage);
    return job;
}

KDevelop::VcsJob* KDevSvnPlugin::createWorkingCopy(const KDevelop::VcsLocation & sourceRepository, const QUrl &destinationDirectory, KDevelop::IBasicVersionControl::RecursionMode recursion)
{
    auto* job = new SvnCheckoutJob(this);
    job->setMapping(sourceRepository, destinationDirectory, recursion);
    return job;
}


KDevelop::ContextMenuExtension KDevSvnPlugin::contextMenuExtension(KDevelop::Context* context, QWidget* parent)
{
    m_common->setupFromContext(context);

    const QList<QUrl> & ctxUrlList  = m_common->contextUrlList();

    bool hasVersionControlledEntries = false;
    for (const QUrl& url : ctxUrlList) {
        if (isVersionControlled(url) || isVersionControlled(KIO::upUrl(url))) {
            hasVersionControlledEntries = true;
            break;
        }
    }

    qCDebug(PLUGIN_SVN) << "version controlled?" << hasVersionControlledEntries;

    if (!hasVersionControlledEntries)
        return IPlugin::contextMenuExtension(context, parent);


    QMenu* svnmenu = m_common->commonActions(parent);
    svnmenu->addSeparator();

    if( !copy_action )
    {
        copy_action = new QAction(i18nc("@action:inmenu", "Copy..."), this);
        connect(copy_action, &QAction::triggered, this, &KDevSvnPlugin::ctxCopy);
    }
    svnmenu->addAction(copy_action);

    if( !move_action )
    {
        move_action = new QAction(i18nc("@action:inmenu", "Move..."), this);
        connect(move_action, &QAction::triggered, this, &KDevSvnPlugin::ctxMove);
    }
    svnmenu->addAction(move_action);

    KDevelop::ContextMenuExtension menuExt;
    menuExt.addAction(KDevelop::ContextMenuExtension::VcsGroup, svnmenu->menuAction());

    return menuExt;
}

void KDevSvnPlugin::ctxCopy()
{
    QList<QUrl> const & ctxUrlList = m_common->contextUrlList();
    if (ctxUrlList.count() > 1) {
        KMessageBox::error(nullptr, i18n("Please select only one item for this operation"));
        return;
    }

    QUrl source = ctxUrlList.first();

    if (source.isLocalFile()) {
        QUrl dir = source;
        bool isFile = QFileInfo(source.toLocalFile()).isFile();

        if (isFile) {
            dir = dir.adjusted(QUrl::RemoveFilename|QUrl::StripTrailingSlash);
        }

        KUrlRequesterDialog dlg(dir, i18nc("@label", "Destination file/directory"), nullptr);

        if (isFile) {
            dlg.urlRequester()->setMode(KFile::File | KFile::Directory | KFile::LocalOnly);
        } else {
            dlg.urlRequester()->setMode(KFile::Directory | KFile::LocalOnly);
        }

        if (dlg.exec() == QDialog::Accepted) { // krazy:exclude=crashy
            KDevelop::ICore::self()->runController()->registerJob(copy(source, dlg.selectedUrl()));
        }
    } else {
        KMessageBox::error(nullptr, i18n("Copying only works on local files"));
        return;
    }

}

void KDevSvnPlugin::ctxMove()
{
    QList<QUrl> const & ctxUrlList = m_common->contextUrlList();
    if (ctxUrlList.count() != 1) {
        KMessageBox::error(nullptr, i18n("Please select only one item for this operation"));
        return;
    }

    QUrl source = ctxUrlList.first();

    if (source.isLocalFile()) {
        QUrl dir = source;
        bool isFile = QFileInfo(source.toLocalFile()).isFile();

        if (isFile) {
            dir = source.adjusted(QUrl::RemoveFilename|QUrl::StripTrailingSlash);
        }

        KUrlRequesterDialog dlg(dir, i18n("Destination file/directory"), nullptr);

        if (isFile) {
            dlg.urlRequester()->setMode(KFile::File | KFile::Directory | KFile::LocalOnly);
        } else {
            dlg.urlRequester()->setMode(KFile::Directory | KFile::LocalOnly);
        }

        if (dlg.exec() == QDialog::Accepted) { // krazy:exclude=crashy
            KDevelop::ICore::self()->runController()->registerJob(move(source, dlg.selectedUrl()));
        }
    } else {
        KMessageBox::error(nullptr, i18n("Moving only works on local files/dirs"));
        return;
    }
}

QString KDevSvnPlugin::name() const
{
    return i18n("Subversion");
}

KDevelop::VcsImportMetadataWidget* KDevSvnPlugin::createImportMetadataWidget(QWidget* parent)
{
    return new SvnImportMetadataWidget(parent);
}

KDevelop::VcsLocationWidget* KDevSvnPlugin::vcsLocation(QWidget* parent) const
{
    return new SvnLocationWidget(parent);
}

ThreadWeaver::Queue* KDevSvnPlugin::jobQueue() const
{
    return m_jobQueue;
}

#include "kdevsvnplugin.moc"
#include "moc_kdevsvnplugin.cpp"
