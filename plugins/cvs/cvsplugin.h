/***************************************************************************
 *   Copyright 2007 Robert Gruber <rgruber@users.sourceforge.net>          *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KDEVPLATFORM_PLUGIN_CVSPLUGIN_H
#define KDEVPLATFORM_PLUGIN_CVSPLUGIN_H

#include <QUrl>
#include <KJob>

#include <vcs/interfaces/icentralizedversioncontrol.h>
#include <vcs/vcslocation.h>
#include <interfaces/iplugin.h>
#include <qobject.h>

class CvsProxy;
namespace KDevelop
{
class ContextMenuExtension;
}
class CvsPluginPrivate;
/**
 * This is the main class of KDevelop's CVS plugin.
 *
 * It implements the IVersionControl interface.
 *
 * @author Robert Gruber <rgruber@users.sourceforge.net>
 */
class CvsPlugin : public KDevelop::IPlugin, public KDevelop::ICentralizedVersionControl
{
    Q_OBJECT
    Q_INTERFACES(KDevelop::IBasicVersionControl KDevelop::ICentralizedVersionControl)

    friend class CvsProxy;

public:
    explicit CvsPlugin(QObject *parent, const QVariantList & args = QVariantList());
    ~CvsPlugin() override;

    void unload() override;

    QString name() const override;
    KDevelop::VcsImportMetadataWidget* createImportMetadataWidget(QWidget* parent) override;

    // From KDevelop::IPlugin
    KDevelop::ContextMenuExtension contextMenuExtension(KDevelop::Context*) override;

    // Begin:  KDevelop::IBasicVersionControl
    bool isValidRemoteRepositoryUrl(const QUrl& remoteLocation) override;
    bool isVersionControlled(const QUrl& localLocation) override;
    KDevelop::VcsJob* repositoryLocation(const QUrl& localLocation) override;
    KDevelop::VcsJob* add(const QList<QUrl>& localLocations,
                                  KDevelop::IBasicVersionControl::RecursionMode recursion) override;
    KDevelop::VcsJob* remove(const QList<QUrl>& localLocations) override;
    KDevelop::VcsJob* copy(const QUrl& localLocationSrc,
                                   const QUrl& localLocationDstn) override;
    KDevelop::VcsJob* move(const QUrl& localLocationSrc,
                                   const QUrl& localLocationDst) override;
    KDevelop::VcsJob* status(const QList<QUrl>& localLocations,
                                     KDevelop::IBasicVersionControl::RecursionMode recursion) override;
    KDevelop::VcsJob* revert(const QList<QUrl>& localLocations,
                                     KDevelop::IBasicVersionControl::RecursionMode recursion) override;
    KDevelop::VcsJob* update(const QList<QUrl>& localLocations,
                                     const KDevelop::VcsRevision& rev,
                                     KDevelop::IBasicVersionControl::RecursionMode recursion) override;
    KDevelop::VcsJob* commit(const QString& message,
                                     const QList<QUrl>& localLocations,
                                     KDevelop::IBasicVersionControl::RecursionMode recursion) override;
    KDevelop::VcsJob* diff(const QUrl& fileOrDirectory,
                                   const KDevelop::VcsRevision& srcRevision,
                                   const KDevelop::VcsRevision& dstRevision,
                                   KDevelop::VcsDiff::Type,
                                   KDevelop::IBasicVersionControl::RecursionMode = KDevelop::IBasicVersionControl::Recursive) override;
    KDevelop::VcsJob* log(const QUrl& localLocation,
                                  const KDevelop::VcsRevision& rev,
                                  unsigned long limit) override;
    KDevelop::VcsJob* log(const QUrl& localLocation,
                                  const KDevelop::VcsRevision& rev,
                                  const KDevelop::VcsRevision& limit) override;
    KDevelop::VcsJob* annotate(const QUrl& localLocation,
                                       const KDevelop::VcsRevision& rev) override;
    KDevelop::VcsJob* resolve(const QList<QUrl>& localLocations,
                                      KDevelop::IBasicVersionControl::RecursionMode recursion) override;
    KDevelop::VcsJob* createWorkingCopy(const KDevelop::VcsLocation & sourceRepository, const QUrl & destinationDirectory, KDevelop::IBasicVersionControl::RecursionMode recursion = KDevelop::IBasicVersionControl::Recursive) override;
    // End:  KDevelop::IBasicVersionControl

    // Begin:  KDevelop::ICentralizedVersionControl
    KDevelop::VcsJob* edit(const QUrl& localLocation) override;
    KDevelop::VcsJob* unedit(const QUrl& localLocation) override;
    KDevelop::VcsJob* localRevision(const QUrl& localLocation,
                                            KDevelop::VcsRevision::RevisionType) override;
    KDevelop::VcsJob* import(const QString& commitMessage, const QUrl& sourceDirectory, const KDevelop::VcsLocation& destinationRepository) override;
// End:  KDevelop::ICentralizedVersionControl

    CvsProxy* proxy();

    const QUrl urlFocusedDocument() const;
    KDevelop::VcsLocationWidget* vcsLocation(QWidget* parent) const override;
public slots:
    // slots for context menu
    void ctxEdit();
    void ctxUnEdit();
    void ctxEditors();

    // slots for menu
    void slotImport();
    void slotCheckout();
    void slotStatus();

signals:
    /**
     * Some actions like commit, add, remove... will connect the job's
     * result() signal to this signal. Anybody, like for instance the
     * CvsMainView class, that is interested in getting notified about
     * jobs that finished can connect to this signal.
     * @see class CvsMainView
     */
    void jobFinished(KJob* job);

    /**
     * Gets emmited when a job like log, editors... was created.
     * CvsPlugin will connect the newly created view to the result() signal
     * of a job. So the new view will show the output of that job as
     * soon as it has finished.
     */
    void addNewTabToMainView(QWidget* tab, QString label);

private:
    void setupActions();
    QScopedPointer<CvsPluginPrivate> d;
    QString findWorkingDir(const QUrl& location);
};

#endif
