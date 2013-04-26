/* KDevPlatform Kross Support
 *
 * Copyright 2008 Aleix Pol <aleixpol@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#ifndef KDEVPLATFORM_KROSSDISTRIBUTEDVERSIONCONTROL_H
#define KDEVPLATFORM_KROSSDISTRIBUTEDVERSIONCONTROL_H

#include <QObject>
#include <QList>

#include <vcs/vcsjob.h>
#include <vcs/interfaces/idistributedversioncontrol.h>

namespace Kross { class Action; }
namespace KDevelop { class Context; }


class KrossDistributedVersionControl : public KDevelop::IDistributedVersionControl
{
public:
    explicit KrossDistributedVersionControl(KDevelop::IPlugin* plugin);
    virtual ~KrossDistributedVersionControl();

    QString name() const;
    KDevelop::VcsImportMetadataWidget* createImportMetadataWidget(QWidget* parent);
    bool isVersionControlled(const KUrl& url);
    KDevelop::VcsJob* add(const KUrl::List& files, KDevelop::IBasicVersionControl::RecursionMode mode);
    KDevelop::VcsJob* remove(const KUrl::List& files);
    KDevelop::VcsJob* copy(const KUrl& file, const KUrl& newfile);
    KDevelop::VcsJob* move(const KUrl& file, const KUrl& newfile);
    KDevelop::VcsJob* status(const KUrl::List& files, KDevelop::IBasicVersionControl::RecursionMode mode);
    KDevelop::VcsJob* revert(const KUrl::List& files, KDevelop::IBasicVersionControl::RecursionMode mode);
    KDevelop::VcsJob* update( const KUrl::List& localLocations,
                                const KDevelop::VcsRevision& rev,
                                KDevelop::IBasicVersionControl::RecursionMode recursion = KDevelop::IBasicVersionControl::Recursive );
    KDevelop::VcsJob* commit(const QString& message, const KUrl::List& files, KDevelop::IBasicVersionControl::RecursionMode mode);
    KDevelop::VcsJob* diff(const KUrl::List& files, KDevelop::VcsRevision& rev, KDevelop::IBasicVersionControl::RecursionMode mode);
    KDevelop::VcsJob* repositoryLocation(const KUrl::List& files);
    KDevelop::VcsJob* repositoryLocation( const KUrl& localLocation );
    
    KDevelop::VcsJob* diff( const KUrl& fileOrDirectory,
                          const KDevelop::VcsRevision& srcRevision,
                          const KDevelop::VcsRevision& dstRevision,
                          KDevelop::VcsDiff::Type = KDevelop::VcsDiff::DiffUnified,
                          KDevelop::IBasicVersionControl::RecursionMode recursion
                                       = KDevelop::IBasicVersionControl::Recursive);
    KDevelop::VcsJob* log(const KUrl& file, const KDevelop::VcsRevision& rev, long unsigned int limit);
    KDevelop::VcsJob* log(const KUrl& file, const KDevelop::VcsRevision&, const KDevelop::VcsRevision&);
    KDevelop::VcsJob* annotate(const KUrl& file, const KDevelop::VcsRevision& rev);

    KDevelop::VcsJob* merge( const KDevelop::VcsLocation& localOrRepoLocationSrc,
                           const KDevelop::VcsLocation& localOrRepoLocationDst,
                           const KDevelop::VcsRevision& srcRevision,
                           const KDevelop::VcsRevision& dstRevision,
                           const KUrl& localLocation );
    KDevelop::VcsJob* resolve( const KUrl::List& localLocations, KDevelop::IBasicVersionControl::RecursionMode recursion );
    KDevelop::VcsJob* createWorkingCopy(const KDevelop::VcsLocation & sourceRepository, const KUrl & destinationDirectory, KDevelop::IBasicVersionControl::RecursionMode recursion = KDevelop::IBasicVersionControl::Recursive );

    //DVCS
    KDevelop::VcsJob* init(const KUrl& location);
    KDevelop::VcsJob* push(const KUrl& localRepositoryLocation, const KDevelop::VcsLocation& localOrRepoLocationDst);
    KDevelop::VcsJob* pull(const KDevelop::VcsLocation& localOrRepoLocationSrc, const KUrl& localRepositoryLocation);
    KDevelop::VcsJob* reset(const KUrl &repository, const QStringList &args, const KUrl::List &files);

    void setActionDistributed(Kross::Action* anAction);
    virtual KDevelop::VcsLocationWidget* vcsLocation(QWidget* parent) const;
private:
    Kross::Action *action;
    KDevelop::IPlugin* m_plugin;

    KUrl::List m_ctxUrls;
};


#endif
