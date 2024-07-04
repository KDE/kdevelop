/*
    SPDX-FileCopyrightText: 2007 Dukju Ahn <dukjuahn@gmail.com>
    SPDX-FileCopyrightText: 2008 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_PLUGIN_KDEVSVNPLUGIN_H
#define KDEVPLATFORM_PLUGIN_KDEVSVNPLUGIN_H

#include <vcs/interfaces/icentralizedversioncontrol.h>
#include <vcs/vcsdiff.h>
#include <vcs/vcslocation.h>
#include <interfaces/iplugin.h>

class QUrl;
class SvnStatusHolder;
class SvnCommitDialog;

namespace ThreadWeaver
{
    class Queue;
}

namespace KDevelop
{
class VcsPluginHelper;
}

class KDevSvnPlugin: public KDevelop::IPlugin, public KDevelop::ICentralizedVersionControl
{
    Q_OBJECT
    Q_INTERFACES(KDevelop::IBasicVersionControl KDevelop::ICentralizedVersionControl)
public:
    explicit KDevSvnPlugin(QObject* parent, const KPluginMetaData& metaData, const QVariantList& = QVariantList());
    ~KDevSvnPlugin() override;

    QString name() const override;
    KDevelop::VcsImportMetadataWidget* createImportMetadataWidget(QWidget* parent) override;

    // Begin:  KDevelop::IBasicVersionControl
    bool isValidRemoteRepositoryUrl(const QUrl& remoteLocation) override;
    bool isVersionControlled(const QUrl &localLocation) override;

    KDevelop::VcsJob* repositoryLocation(const QUrl &localLocation) override;

    KDevelop::VcsJob* status(const QList<QUrl>& localLocations,
                             KDevelop::IBasicVersionControl::RecursionMode recursion
                             = KDevelop::IBasicVersionControl::Recursive) override;

    KDevelop::VcsJob* add(const QList<QUrl>& localLocations,
                          KDevelop::IBasicVersionControl::RecursionMode recursion
                          = KDevelop::IBasicVersionControl::Recursive) override;

    KDevelop::VcsJob* remove(const QList<QUrl>& localLocations) override;

    KDevelop::VcsJob* copy(const QUrl &localLocationSrc,
                           const QUrl &localLocationDstn) override;

    KDevelop::VcsJob* move(const QUrl &localLocationSrc,
                           const QUrl &localLocationDst) override;

    KDevelop::VcsJob* revert(const QList<QUrl>& localLocations,
                             KDevelop::IBasicVersionControl::RecursionMode recursion
                             = KDevelop::IBasicVersionControl::Recursive) override;

    KDevelop::VcsJob* update(const QList<QUrl>& localLocations,
                             const KDevelop::VcsRevision& rev,
                             KDevelop::IBasicVersionControl::RecursionMode recursion
                             = KDevelop::IBasicVersionControl::Recursive) override;

    KDevelop::VcsJob* commit(const QString& message,
                             const QList<QUrl>& localLocations,
                             KDevelop::IBasicVersionControl::RecursionMode recursion
                             = KDevelop::IBasicVersionControl::Recursive) override;

    KDevelop::VcsJob* diff(const QUrl &fileOrDirectory,
                           const KDevelop::VcsRevision& srcRevision,
                           const KDevelop::VcsRevision& dstRevision,
                           KDevelop::IBasicVersionControl::RecursionMode recursion
                           = KDevelop::IBasicVersionControl::Recursive) override;

    /**
     * Retrieves a diff between the two locations at the given revisions
     *
     * The diff is in unified diff format for text files by default
     */
    KDevelop::VcsJob* diff2(const KDevelop::VcsLocation& localOrRepoLocationSrc,
                            const KDevelop::VcsLocation& localOrRepoLocationDst,
                            const KDevelop::VcsRevision& srcRevision,
                            const KDevelop::VcsRevision& dstRevision,
                            KDevelop::IBasicVersionControl::RecursionMode = KDevelop::IBasicVersionControl::Recursive);

    KDevelop::VcsJob* log(const QUrl &localLocation,
                          const KDevelop::VcsRevision& rev,
                          unsigned long limit) override;

    KDevelop::VcsJob* log(const QUrl &localLocation,
                          const KDevelop::VcsRevision& rev,
                          const KDevelop::VcsRevision& limit) override;

    KDevelop::VcsJob* annotate(const QUrl &localLocation,
                               const KDevelop::VcsRevision& rev) override;

    KDevelop::VcsJob* merge(const KDevelop::VcsLocation& localOrRepoLocationSrc,
                            const KDevelop::VcsLocation& localOrRepoLocationDst,
                            const KDevelop::VcsRevision& srcRevision,
                            const KDevelop::VcsRevision& dstRevision,
                            const QUrl &localLocation);

    KDevelop::VcsJob* resolve(const QList<QUrl>& localLocations,
                              KDevelop::IBasicVersionControl::RecursionMode recursion) override;
    KDevelop::VcsLocationWidget* vcsLocation(QWidget* parent) const override;
    // End:  KDevelop::IBasicVersionControl

    // Begin:  KDevelop::ICentralizedVersionControl
    KDevelop::VcsJob* import(const QString & commitMessage, const QUrl &sourceDirectory, const KDevelop::VcsLocation & destinationRepository) override;

    KDevelop::VcsJob* createWorkingCopy(const KDevelop::VcsLocation & sourceRepository, const QUrl &destinationDirectory, KDevelop::IBasicVersionControl::RecursionMode recursion = KDevelop::IBasicVersionControl::Recursive) override;

    KDevelop::VcsJob* edit(const QUrl &localLocation) override;

    KDevelop::VcsJob* unedit(const QUrl &localLocation) override;

    KDevelop::VcsJob* localRevision(const QUrl &localLocation,
                                    KDevelop::VcsRevision::RevisionType) override;
    // End:  KDevelop::ICentralizedVersionControl

    KDevelop::ContextMenuExtension contextMenuExtension(KDevelop::Context* context, QWidget* parent) override;

    ThreadWeaver::Queue* jobQueue() const;

public Q_SLOTS:

    // invoked by context-menu
    void ctxCopy();
    void ctxMove();
private:
    QScopedPointer<KDevelop::VcsPluginHelper> m_common;
    QAction* copy_action;
    QAction* move_action;
    ThreadWeaver::Queue* m_jobQueue;
};
#endif

