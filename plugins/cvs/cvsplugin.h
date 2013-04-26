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

#include <KUrl>
#include <KJob>

#include <vcs/interfaces/icentralizedversioncontrol.h>
#include <vcs/vcslocation.h>
#include <interfaces/iplugin.h>
#include <qobject.h>
#include <memory>

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
    virtual ~CvsPlugin();
    
    virtual void unload();

    virtual QString name() const;
    virtual KDevelop::VcsImportMetadataWidget* createImportMetadataWidget(QWidget* parent);

    // From KDevelop::IPlugin
    KDevelop::ContextMenuExtension contextMenuExtension(KDevelop::Context*);

    // Begin:  KDevelop::IBasicVersionControl
    virtual bool isVersionControlled(const KUrl& localLocation);
    virtual KDevelop::VcsJob* repositoryLocation(const KUrl& localLocation);
    virtual KDevelop::VcsJob* add(const KUrl::List& localLocations,
                                  KDevelop::IBasicVersionControl::RecursionMode recursion);
    virtual KDevelop::VcsJob* remove(const KUrl::List& localLocations);
    virtual KDevelop::VcsJob* copy(const KUrl& localLocationSrc,
                                   const KUrl& localLocationDstn);
    virtual KDevelop::VcsJob* move(const KUrl& localLocationSrc,
                                   const KUrl& localLocationDst);
    virtual KDevelop::VcsJob* status(const KUrl::List& localLocations,
                                     KDevelop::IBasicVersionControl::RecursionMode recursion);
    virtual KDevelop::VcsJob* revert(const KUrl::List& localLocations,
                                     KDevelop::IBasicVersionControl::RecursionMode recursion);
    virtual KDevelop::VcsJob* update(const KUrl::List& localLocations,
                                     const KDevelop::VcsRevision& rev,
                                     KDevelop::IBasicVersionControl::RecursionMode recursion);
    virtual KDevelop::VcsJob* commit(const QString& message,
                                     const KUrl::List& localLocations,
                                     KDevelop::IBasicVersionControl::RecursionMode recursion);
    virtual KDevelop::VcsJob* diff(const KUrl& fileOrDirectory,
                                   const KDevelop::VcsRevision& srcRevision,
                                   const KDevelop::VcsRevision& dstRevision,
                                   KDevelop::VcsDiff::Type,
                                   KDevelop::IBasicVersionControl::RecursionMode = KDevelop::IBasicVersionControl::Recursive);
    virtual KDevelop::VcsJob* log(const KUrl& localLocation,
                                  const KDevelop::VcsRevision& rev,
                                  unsigned long limit);
    virtual KDevelop::VcsJob* log(const KUrl& localLocation,
                                  const KDevelop::VcsRevision& rev,
                                  const KDevelop::VcsRevision& limit);
    virtual KDevelop::VcsJob* annotate(const KUrl& localLocation,
                                       const KDevelop::VcsRevision& rev);
    virtual KDevelop::VcsJob* resolve(const KUrl::List& localLocations,
                                      KDevelop::IBasicVersionControl::RecursionMode recursion);
    virtual KDevelop::VcsJob* createWorkingCopy(const KDevelop::VcsLocation & sourceRepository, const KUrl & destinationDirectory, KDevelop::IBasicVersionControl::RecursionMode recursion = KDevelop::IBasicVersionControl::Recursive);
    // End:  KDevelop::IBasicVersionControl

    // Begin:  KDevelop::ICentralizedVersionControl
    virtual KDevelop::VcsJob* edit(const KUrl& localLocation);
    virtual KDevelop::VcsJob* unedit(const KUrl& localLocation);
    virtual KDevelop::VcsJob* localRevision(const KUrl& localLocation,
                                            KDevelop::VcsRevision::RevisionType);
    virtual KDevelop::VcsJob* import(const QString& commitMessage, const KUrl& sourceDirectory, const KDevelop::VcsLocation& destinationRepository);
// End:  KDevelop::ICentralizedVersionControl

    CvsProxy* proxy();

    const KUrl urlFocusedDocument() const;
    virtual KDevelop::VcsLocationWidget* vcsLocation(QWidget* parent) const;
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
    std::auto_ptr<CvsPluginPrivate> d;
    QString findWorkingDir(const KUrl& location);
};

#endif
