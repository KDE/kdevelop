/***************************************************************************
 *   Copyright 2007 Dukju Ahn <dukjuahn@gmail.com>                         *
 *   Copyright 2008 Andreas Pakulat <apaku@gmx.de>                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KDEVPLATFORM_PLUGIN_KDEVSVNPLUGIN_H
#define KDEVPLATFORM_PLUGIN_KDEVSVNPLUGIN_H

#include <vcs/interfaces/icentralizedversioncontrol.h>
#include <vcs/vcsdiff.h>
#include <vcs/vcslocation.h>
#include <interfaces/iplugin.h>

class QMenu;
class QUrl;
class SvnStatusHolder;
class QTemporaryDir;
class SvnCommitDialog;

namespace KDevelop
{
class VcsCommitDialog;
class ContextMenuExtension;
class VcsPluginHelper;
}

class KDevSvnPlugin: public KDevelop::IPlugin, public KDevelop::ICentralizedVersionControl
{
    Q_OBJECT
    Q_INTERFACES(KDevelop::IBasicVersionControl KDevelop::ICentralizedVersionControl)
public:
    explicit KDevSvnPlugin(QObject *parent, const QVariantList & = QVariantList());
    virtual ~KDevSvnPlugin();

    virtual QString name() const;
    virtual KDevelop::VcsImportMetadataWidget* createImportMetadataWidget(QWidget* parent);

    // Begin:  KDevelop::IBasicVersionControl
    bool isVersionControlled(const QUrl &localLocation);

    KDevelop::VcsJob* repositoryLocation(const QUrl &localLocation);

    KDevelop::VcsJob* status(const QList<QUrl>& localLocations,
                             KDevelop::IBasicVersionControl::RecursionMode recursion
                             = KDevelop::IBasicVersionControl::Recursive);

    KDevelop::VcsJob* add(const QList<QUrl>& localLocations,
                          KDevelop::IBasicVersionControl::RecursionMode recursion
                          = KDevelop::IBasicVersionControl::Recursive);

    KDevelop::VcsJob* remove(const QList<QUrl>& localLocations);

    KDevelop::VcsJob* copy(const QUrl &localLocationSrc,
                           const QUrl &localLocationDstn);

    KDevelop::VcsJob* move(const QUrl &localLocationSrc,
                           const QUrl &localLocationDst);

    KDevelop::VcsJob* revert(const QList<QUrl>& localLocations,
                             KDevelop::IBasicVersionControl::RecursionMode recursion
                             = KDevelop::IBasicVersionControl::Recursive);

    KDevelop::VcsJob* update(const QList<QUrl>& localLocations,
                             const KDevelop::VcsRevision& rev,
                             KDevelop::IBasicVersionControl::RecursionMode recursion
                             = KDevelop::IBasicVersionControl::Recursive);

    KDevelop::VcsJob* commit(const QString& message,
                             const QList<QUrl>& localLocations,
                             KDevelop::IBasicVersionControl::RecursionMode recursion
                             = KDevelop::IBasicVersionControl::Recursive);

    KDevelop::VcsJob* diff(const QUrl &fileOrDirectory,
                           const KDevelop::VcsRevision& srcRevision,
                           const KDevelop::VcsRevision& dstRevision,
                           KDevelop::VcsDiff::Type = KDevelop::VcsDiff::DiffUnified,
                           KDevelop::IBasicVersionControl::RecursionMode recursion
                           = KDevelop::IBasicVersionControl::Recursive);

    /**
     * Retrieves a diff between the two locations at the given revisions
     *
     * The diff is in unified diff format for text files by default
     */
    KDevelop::VcsJob* diff2(const KDevelop::VcsLocation& localOrRepoLocationSrc,
                            const KDevelop::VcsLocation& localOrRepoLocationDst,
                            const KDevelop::VcsRevision& srcRevision,
                            const KDevelop::VcsRevision& dstRevision,
                            KDevelop::VcsDiff::Type = KDevelop::VcsDiff::DiffDontCare,
                            KDevelop::IBasicVersionControl::RecursionMode = KDevelop::IBasicVersionControl::Recursive);

    KDevelop::VcsJob* log(const QUrl &localLocation,
                          const KDevelop::VcsRevision& rev,
                          unsigned long limit);

    KDevelop::VcsJob* log(const QUrl &localLocation,
                          const KDevelop::VcsRevision& rev,
                          const KDevelop::VcsRevision& limit);

    KDevelop::VcsJob* annotate(const QUrl &localLocation,
                               const KDevelop::VcsRevision& rev);

    KDevelop::VcsJob* merge(const KDevelop::VcsLocation& localOrRepoLocationSrc,
                            const KDevelop::VcsLocation& localOrRepoLocationDst,
                            const KDevelop::VcsRevision& srcRevision,
                            const KDevelop::VcsRevision& dstRevision,
                            const QUrl &localLocation);

    KDevelop::VcsJob* resolve(const QList<QUrl>& localLocations,
                              KDevelop::IBasicVersionControl::RecursionMode recursion);
    KDevelop::VcsLocationWidget* vcsLocation(QWidget* parent) const;
    // End:  KDevelop::IBasicVersionControl

    // Begin:  KDevelop::ICentralizedVersionControl
    KDevelop::VcsJob* import(const QString & commitMessage, const QUrl &sourceDirectory, const KDevelop::VcsLocation & destinationRepository);

    KDevelop::VcsJob* createWorkingCopy(const KDevelop::VcsLocation & sourceRepository, const QUrl &destinationDirectory, KDevelop::IBasicVersionControl::RecursionMode recursion = KDevelop::IBasicVersionControl::Recursive);

    KDevelop::VcsJob* edit(const QUrl &localLocation);

    KDevelop::VcsJob* unedit(const QUrl &localLocation);

    KDevelop::VcsJob* localRevision(const QUrl &localLocation,
                                    KDevelop::VcsRevision::RevisionType);
    // End:  KDevelop::ICentralizedVersionControl

    KDevelop::ContextMenuExtension contextMenuExtension(KDevelop::Context*);

public Q_SLOTS:

    // invoked by context-menu
    void ctxInfo();
    void ctxStatus();
    void ctxCopy();
    void ctxMove();
    void ctxCat();
    void ctxImport();
    void ctxCheckout();
private:
    QScopedPointer<KDevelop::VcsPluginHelper> m_common;
    QAction* copy_action;
    QAction* move_action;
};
#endif

