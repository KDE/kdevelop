/***************************************************************************
 *   This file was partly taken from KDevelop's cvs plugin                 *
 *   Copyright 2007 Robert Gruber <rgruber@users.sourceforge.net>          *
 *                                                                         *
 *   Adapted for Git                                                       *
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

#ifndef GIT_PLUGIN_H
#define GIT_PLUGIN_H

#include <KUrl>
#include <KJob>

#include <vcs/interfaces/idistributedversioncontrol.h>
#include <vcs/vcslocation.h>
#include <interfaces/iplugin.h>
#include <qobject.h>

class GitProxy;

namespace KDevelop
{
    class ContextMenuExtension;
}

/**
 * This is the main class of KDevelop's Git plugin.
 *
 * It implements the IBasicVersionControl and IDistributedVersionControl interfaces.
 * @author Robert Gruber <rgruber@users.sourceforge.net>
 * @author Evgeniy Ivanov <powerfox@kde.ru>
 */
class GitPlugin: public KDevelop::IPlugin, public KDevelop::IDistributedVersionControl
{
    Q_OBJECT
    Q_INTERFACES(KDevelop::IBasicVersionControl KDevelop::IDistributedVersionControl)

friend class GitProxy;

public:
    GitPlugin( QObject *parent, const QVariantList & args = QVariantList() );
    ~GitPlugin();

    // From KDevelop::IPlugin
    KDevelop::ContextMenuExtension contextMenuExtension( KDevelop::Context* );

    // Begin: KDevelop::IBasicVersionControl
    QString name() const;
    KDevelop::VcsImportMetadataWidget* createImportMetadataWidget( QWidget* parent );
    bool isVersionControlled( const KUrl& localLocation );
    KDevelop::VcsJob* repositoryLocation( const KUrl& localLocation );
    KDevelop::VcsJob* add( const KUrl::List& localLocations,
                           KDevelop::IBasicVersionControl::RecursionMode recursion );
    KDevelop::VcsJob* remove( const KUrl::List& localLocations );
    KDevelop::VcsJob* status( const KUrl::List& localLocations,
                                      KDevelop::IBasicVersionControl::RecursionMode recursion );
    KDevelop::VcsJob* copy( const KUrl& localLocationSrc,
                                    const KUrl& localLocationDstn ); ///Not used in DVCS;
    KDevelop::VcsJob* move( const KUrl& localLocationSrc,
                                    const KUrl& localLocationDst ); ///Not used in DVCS;
    KDevelop::VcsJob* revert( const KUrl::List& localLocations,
                                      KDevelop::IBasicVersionControl::RecursionMode recursion );
    KDevelop::VcsJob* update( const KUrl::List& localLocations,
                                      const KDevelop::VcsRevision& rev,
                                      KDevelop::IBasicVersionControl::RecursionMode recursion );
    KDevelop::VcsJob* commit( const QString& message,
                                      const KUrl::List& localLocations,
                                      KDevelop::IBasicVersionControl::RecursionMode recursion );
    KDevelop::VcsJob* diff( const KDevelop::VcsLocation& localOrRepoLocationSrc,
                                    const KDevelop::VcsLocation& localOrRepoLocationDst,
                                    const KDevelop::VcsRevision& srcRevision,
                                    const KDevelop::VcsRevision& dstRevision,
                                    KDevelop::VcsDiff::Type,
                                    KDevelop::IBasicVersionControl::RecursionMode = KDevelop::IBasicVersionControl::Recursive );
    KDevelop::VcsJob* log( const KUrl& localLocation,
                                   const KDevelop::VcsRevision& rev,
                                   unsigned long limit );
    KDevelop::VcsJob* log( const KUrl& localLocation,
                                   const KDevelop::VcsRevision& rev,
                                   const KDevelop::VcsRevision& limit );
    KDevelop::VcsJob* annotate( const KUrl& localLocation,
                                        const KDevelop::VcsRevision& rev );
    KDevelop::VcsJob* merge( const KDevelop::VcsLocation& localOrRepoLocationSrc,
                                     const KDevelop::VcsLocation& localOrRepoLocationDst,
                                     const KDevelop::VcsRevision& srcRevision,
                                     const KDevelop::VcsRevision& dstRevision,
                                     const KUrl& localLocation );
    KDevelop::VcsJob* resolve( const KUrl::List& localLocations,
                                       KDevelop::IBasicVersionControl::RecursionMode recursion );
    // End:  KDevelop::IBasicVersionControl


    // Begin:  KDevelop::IDistributedVersionControl
    KDevelop::VcsJob* init( const KUrl& localRepositoryRoot );
    KDevelop::VcsJob* clone( const QString& repositoryLocationSrc,
                   const KUrl& localRepositoryRoot );
    KDevelop::VcsJob* push( const KUrl& localRepositoryLocation,
                  const QString& repositoryLocation );
    KDevelop::VcsJob* pull( const QString& repositoryLocation,
                  const KUrl& localRepositoryLocation );
    // End:  KDevelop::IDistributedVersionControl

    GitProxy* proxy();

    const KUrl urlFocusedDocument() const;

public slots:
    // slots for context menu
    void ctxCommit();
    void ctxAdd();
    void ctxRemove();
//     void ctxUpdate();
//     void ctxLog();
//     void ctxAnnotate();
//     void ctxRevert();
//     void ctxDiff();
//     void ctxEditors();

    // slots for menu
    void slotInit();
//     void slotCheckout();
//     void slotStatus();

signals:
    /**
     * Some actions like commit, add, remove... will connect the job's
     * result() signal to this signal. Anybody, like for instance the
     * GitMainView class, that is interested in getting notified about
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

private:
    class GitPluginPrivate* d;

    void setupActions();
};

#endif
