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
#include <memory>

class QMenu;
class KUrl;
class SvnStatusHolder;
class KTempDir;
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
    bool isVersionControlled(const KUrl& localLocation);

    KDevelop::VcsJob* repositoryLocation(const KUrl& localLocation);

    KDevelop::VcsJob* status(const KUrl::List& localLocations,
                             KDevelop::IBasicVersionControl::RecursionMode recursion
                             = KDevelop::IBasicVersionControl::Recursive);

    KDevelop::VcsJob* add(const KUrl::List& localLocations,
                          KDevelop::IBasicVersionControl::RecursionMode recursion
                          = KDevelop::IBasicVersionControl::Recursive);

    KDevelop::VcsJob* remove(const KUrl::List& localLocations);

    KDevelop::VcsJob* copy(const KUrl& localLocationSrc,
                           const KUrl& localLocationDstn);

    KDevelop::VcsJob* move(const KUrl& localLocationSrc,
                           const KUrl& localLocationDst);

    KDevelop::VcsJob* revert(const KUrl::List& localLocations,
                             KDevelop::IBasicVersionControl::RecursionMode recursion
                             = KDevelop::IBasicVersionControl::Recursive);

    KDevelop::VcsJob* update(const KUrl::List& localLocations,
                             const KDevelop::VcsRevision& rev,
                             KDevelop::IBasicVersionControl::RecursionMode recursion
                             = KDevelop::IBasicVersionControl::Recursive);

    KDevelop::VcsJob* commit(const QString& message,
                             const KUrl::List& localLocations,
                             KDevelop::IBasicVersionControl::RecursionMode recursion
                             = KDevelop::IBasicVersionControl::Recursive);

    KDevelop::VcsJob* diff(const KUrl& fileOrDirectory,
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

    KDevelop::VcsJob* log(const KUrl& localLocation,
                          const KDevelop::VcsRevision& rev,
                          unsigned long limit);

    KDevelop::VcsJob* log(const KUrl& localLocation,
                          const KDevelop::VcsRevision& rev,
                          const KDevelop::VcsRevision& limit);

    KDevelop::VcsJob* annotate(const KUrl& localLocation,
                               const KDevelop::VcsRevision& rev);

    KDevelop::VcsJob* merge(const KDevelop::VcsLocation& localOrRepoLocationSrc,
                            const KDevelop::VcsLocation& localOrRepoLocationDst,
                            const KDevelop::VcsRevision& srcRevision,
                            const KDevelop::VcsRevision& dstRevision,
                            const KUrl& localLocation);

    KDevelop::VcsJob* resolve(const KUrl::List& localLocations,
                              KDevelop::IBasicVersionControl::RecursionMode recursion);
    KDevelop::VcsLocationWidget* vcsLocation(QWidget* parent) const;
    // End:  KDevelop::IBasicVersionControl

    // Begin:  KDevelop::ICentralizedVersionControl
    KDevelop::VcsJob* import(const QString & commitMessage, const KUrl & sourceDirectory, const KDevelop::VcsLocation & destinationRepository);

    KDevelop::VcsJob* createWorkingCopy(const KDevelop::VcsLocation & sourceRepository, const KUrl & destinationDirectory, KDevelop::IBasicVersionControl::RecursionMode recursion = KDevelop::IBasicVersionControl::Recursive);

    KDevelop::VcsJob* edit(const KUrl& localLocation);

    KDevelop::VcsJob* unedit(const KUrl& localLocation);

    KDevelop::VcsJob* localRevision(const KUrl& localLocation,
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
    std::auto_ptr<KDevelop::VcsPluginHelper> m_common;
    QMenu* svnmenu;
    QAction* copy_action;
    QAction* move_action;
};
#endif

