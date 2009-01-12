/***************************************************************************
 *   Copyright 2008 Evgeniy Ivanov <powerfox@kde.ru>                       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU General Public License as        *
 *   published by the Free Software Foundation; either version 2 of        *
 *   the License or (at your option) version 3 or any later version        *
 *   accepted by the membership of KDE e.V. (or its successor approved     *
 *   by the membership of KDE e.V.), which shall act as a proxy            *
 *   defined in Section 14 of version 3 of the license.                    *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 ***************************************************************************/

#ifndef DVCS_PLUGIN_H
#define DVCS_PLUGIN_H

#include <KDE/KUrl>
#include <KDE/KJob>
#include <KDE/KComponentData>

#include <QtCore/QObject>

#include <vcs/interfaces/idistributedversioncontrol.h>
#include <interfaces/iuicontroller.h>
#include <interfaces/iplugin.h>

#include "../vcsexport.h"

#include "idvcsexecutor.h"

class QString;
class KDevDVCSViewFactory;

struct DistributedVersionControlPluginPrivate {
    KDevDVCSViewFactory* m_factory;
    KDevelop::IDVCSexecutor* m_exec;
    KUrl::List m_ctxUrlList;
};


namespace KDevelop
{
class VcsJob;
class ContextMenuExtension;

/**
 * DistributedVersionControlPlugin is a base class for git/hg/bzr plugins. This class implements
 * KDevelop::IBasicVersionControl, KDevelop::IDistributedVersionControl and KDevelop::IPlugin (contextMenuExtension).
 * DistributedVersionControlPlugin class uses IDVCSexecutor to get all jobs
 * from real DVCS plugins like Git. It is based on KDevelop's CVS plugin (also looks like svn plugin is it's relative too).
 * @note Create only special items in contextMenuExtension, all standart menu items are created in vcscommon plugin!
 */
class KDEVPLATFORMVCS_EXPORT DistributedVersionControlPlugin : public IPlugin, public IDistributedVersionControl
{
    Q_OBJECT
    Q_INTERFACES(KDevelop::IBasicVersionControl KDevelop::IDistributedVersionControl)
public:

    DistributedVersionControlPlugin(QObject *parent, KComponentData compData);
    virtual ~DistributedVersionControlPlugin(){}

    // Begin: KDevelop::IBasicVersionControl
    virtual QString name() const;
    virtual bool isVersionControlled(const KUrl& localLocation);
    virtual VcsJob* repositoryLocation(const KUrl& localLocation);
    virtual VcsJob* add(const KUrl::List& localLocations,
                        IBasicVersionControl::RecursionMode recursion  = IBasicVersionControl::Recursive);
    virtual VcsJob* remove(const KUrl::List& localLocations);
    virtual VcsJob* status(const KUrl::List& localLocations,
                           IBasicVersionControl::RecursionMode recursion);
    virtual VcsJob* copy(const KUrl& localLocationSrc,
                         const KUrl& localLocationDstn); ///Not used in DVCS;
    virtual VcsJob* move(const KUrl& localLocationSrc,
                         const KUrl& localLocationDst); ///Not used in DVCS;
    virtual VcsJob* revert(const KUrl::List& localLocations,
                           IBasicVersionControl::RecursionMode recursion);
    virtual VcsJob* update(const KUrl::List& localLocations,
                           const VcsRevision& rev,
                           IBasicVersionControl::RecursionMode recursion);
    virtual VcsJob* commit(const QString& message,
                           const KUrl::List& localLocations,
                           IBasicVersionControl::RecursionMode recursion  = IBasicVersionControl::Recursive);
    virtual VcsJob* diff(const VcsLocation& localOrRepoLocationSrc,
                         const VcsLocation& localOrRepoLocationDst,
                         const VcsRevision& srcRevision,
                         const VcsRevision& dstRevision,
                         VcsDiff::Type,
                         IBasicVersionControl::RecursionMode = IBasicVersionControl::Recursive);
    virtual VcsJob* log(const KUrl& localLocation,
                        const VcsRevision& rev,
                        unsigned long limit);
    virtual VcsJob* log(const KUrl& localLocation,
                        const VcsRevision& rev,
                        const VcsRevision& limit);
    virtual VcsJob* annotate(const KUrl& localLocation,
                             const VcsRevision& rev);
    virtual VcsJob* merge(const VcsLocation& localOrRepoLocationSrc,
                          const VcsLocation& localOrRepoLocationDst,
                          const VcsRevision& srcRevision,
                          const VcsRevision& dstRevision,
                          const KUrl& localLocation);
    virtual VcsJob* resolve(const KUrl::List& localLocations,
                            IBasicVersionControl::RecursionMode recursion);
    virtual VcsJob* checkout(const VcsMapping &mapping);
    // End:  KDevelop::IBasicVersionControl

    // Begin:  KDevelop::IDistributedVersionControl
    virtual VcsJob* add_dvcs(const KUrl &repository, const KUrl::List& localLocations);
    virtual VcsJob* init(const KUrl& localRepositoryRoot);
    virtual VcsJob* clone(const VcsLocation& localOrRepoLocationSrc,
                          const KUrl& localRepositoryRoot);
    virtual VcsJob* push(const KUrl& localRepositoryLocation,
                         const VcsLocation& localOrRepoLocationDst);
    virtual VcsJob* pull(const VcsLocation& localOrRepoLocationSrc,
                         const KUrl& localRepositoryLocation);
    virtual VcsJob* reset(const KUrl& repository, 
                          const QStringList &args, const KUrl::List& files);
    // End:  KDevelop::IDistributedVersionControl

    /** Returns pointer to IDVCSexecutor used in DistributedVersionControlPlugin */
    IDVCSexecutor* proxy();

    /** Used in KDevelop's appwizardplugin (creates import widget) */
    virtual VcsImportMetadataWidget* createImportMetadataWidget(QWidget* parent);

    // From KDevelop::IPlugin
    /** Creates context menu
     * @note Create only special items here (like checkout), all standart menu items are created in vcscommon plugin!
     */
    virtual ContextMenuExtension contextMenuExtension(Context*);

public Q_SLOTS:
    //slots for context menu
    void ctxCommit();
    void ctxAdd();
    void ctxRemove();
    void ctxCheckout();
    void ctxLog();
    void ctxStatus();
    void ctxRevHistory();

    // slots for menu
    void slotInit();

    /**
     * Updates project state after checkout (simply reloads it now)
     */
    void checkoutFinished(KJob*);

Q_SIGNALS:
    /**
     * Some actions like commit, add, remove... will connect the job's
     * result() signal to this signal. Anybody, like for instance the
     * DVCSMainView class, that is interested in getting notified about
     * jobs that finished can connect to this signal.
     * @see class GitMainView
     */
    void jobFinished(KJob* job);

    /**
     * Gets emmited when a job like log, editors... was created.
     * GitPlugin will connect the newly created view to the result() signal
     * of a job. So the new view will show the output of that job as
     * soon as it has finished.
     */
    void addNewTabToMainView(QWidget* tab, QString label);

protected:
    const KUrl urlFocusedDocument() const;

protected:
    DistributedVersionControlPluginPrivate* const d;
};

}

class KDevDVCSViewFactory: public KDevelop::IToolViewFactory{
public:
    KDevDVCSViewFactory(KDevelop::DistributedVersionControlPlugin *plugin): m_plugin(plugin) {}
    virtual QWidget* create(QWidget *parent = 0);
    virtual Qt::DockWidgetArea defaultPosition();
    virtual QString id() const;
private:
    KDevelop::DistributedVersionControlPlugin *m_plugin;
};

#endif
