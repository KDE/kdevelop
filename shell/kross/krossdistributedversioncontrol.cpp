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

#include "krossdistributedversioncontrol.h"

#include <KDebug>
#include <KLocale>

#include <QAction>
#include <QMenu>

#include <interfaces/iplugin.h>
#include <kross/core/action.h>
#include <interfaces/context.h>
#include <project/projectmodel.h>
#include "krossvcsjob.h"
#include "wrappers/krossvcsrevision.h"
#include "wrappers/krossvcslocation.h"
#include <vcs/widgets/vcsimportmetadatawidget.h>

using namespace KDevelop;

KrossDistributedVersionControl::KrossDistributedVersionControl(KDevelop::IPlugin* plugin)
    : action(0), m_plugin(plugin)
{
}

KrossDistributedVersionControl::~KrossDistributedVersionControl()
{
}

void KrossDistributedVersionControl::setActionDistributed(Kross::Action* anAction)
{
    action=anAction;
}

QString KrossDistributedVersionControl::name() const
{
    QVariantList args;
    QVariant result=action->callFunction("name", args );
    kDebug() << "retrieving name" << result.toString();
    return result.toString();
}

KDevelop::VcsImportMetadataWidget * KrossDistributedVersionControl::createImportMetadataWidget(QWidget * w)
{
    QVariantList args;
    args << qVariantFromValue<QObject*>(w);
    QVariant result=action->callFunction( "createImportMetadataWidget", args);
    return qobject_cast<KDevelop::VcsImportMetadataWidget *>(result.value<QObject*>());
}

bool KrossDistributedVersionControl::isVersionControlled(const KUrl & url)
{
    QVariantList args;
    args << QUrl(url);
    QVariant result=action->callFunction( "isVersionControlled", args);
    return result.toBool();
}

KDevelop::VcsJob * KrossDistributedVersionControl::add(const KUrl::List & files, KDevelop::IBasicVersionControl::RecursionMode mode = IBasicVersionControl::Recursive)
{
    QVariant param(files);
    QVariant param1(mode==Recursive);

    return new KrossVcsJob("add", QVariantList() << param << param1, action, m_plugin, action);
}

KDevelop::VcsJob * KrossDistributedVersionControl::remove(const KUrl::List & files)
{
    QVariant param(files);
    
    return new KrossVcsJob("remove", QVariantList() << param, action, m_plugin, action);
}

KDevelop::VcsJob * KrossDistributedVersionControl::copy(const KUrl & file, const KUrl & newfile)
{
    QVariant from(file);
    QVariant to(newfile);
    
    return new KrossVcsJob("copy", QVariantList() << from << to, action, m_plugin, action);
}

KDevelop::VcsJob * KrossDistributedVersionControl::move(const KUrl & file, const KUrl & newfile)
{
    QVariant from(file);
    QVariant to(newfile);
    
    return new KrossVcsJob("move", QVariantList() << from << to, action, m_plugin, action);
}

KDevelop::VcsJob * KrossDistributedVersionControl::status(const KUrl::List & files, KDevelop::IBasicVersionControl::RecursionMode mode)
{
    QVariant param(files);
    QVariant param1(mode);
    
    return new KrossVcsJob("status", QVariantList() << param << param1, action, m_plugin, action);
}

KDevelop::VcsJob * KrossDistributedVersionControl::revert(const KUrl::List & files, KDevelop::IBasicVersionControl::RecursionMode mode)
{
    QVariant param(files);
    QVariant param1(mode==Recursive);
    
    return new KrossVcsJob("revert", QVariantList() << param << param1, action, m_plugin, action);
}

KDevelop::VcsJob * KrossDistributedVersionControl::update(const KUrl::List & localLocations, const KDevelop::VcsRevision & rev,
                                                            KDevelop::IBasicVersionControl::RecursionMode rec)
{
    QVariant revision=Handlers::kDevelopVcsRevisionHandler(&rev);
    
    QVariantList parameters;
    parameters.append(QVariant(localLocations));
    parameters.append(revision);
    parameters.append(QVariant(rec));
    
    return new KrossVcsJob("update", parameters, action, m_plugin, action);
    return 0;
}

KDevelop::VcsJob * KrossDistributedVersionControl::commit(const QString & msg, const KUrl::List & files,
                                                          KDevelop::IBasicVersionControl::RecursionMode mode)
{
    QVariant message(msg);
    QVariant file(files);
    QVariant recursion(mode);
    
    return new KrossVcsJob("commit", QVariantList() << message << file << mode, action, m_plugin, action);
}

KDevelop::VcsJob * KrossDistributedVersionControl::diff(const KUrl::List & files, KDevelop::VcsRevision & rev,
                                                        KDevelop::IBasicVersionControl::RecursionMode mode)
{
    QVariant revision=Handlers::kDevelopVcsRevisionHandler(&rev);
    
    return new KrossVcsJob("diff", QVariantList() << files << revision << mode, action, m_plugin, action);
}

KDevelop::VcsJob * KrossDistributedVersionControl::repositoryLocation(const KUrl::List & files)
{
    return new KrossVcsJob("repositoryLocation", QVariantList() << files, action, m_plugin, action);
}

KDevelop::VcsJob * KrossDistributedVersionControl::repositoryLocation(const KUrl & localLocation)
{
    return new KrossVcsJob("repositoryLocation", QVariantList() << localLocation, action, m_plugin, action);
}

KDevelop::VcsJob * KrossDistributedVersionControl::diff(const KUrl & fileOrDirectory,
                                                        const KDevelop::VcsRevision & srcRevision,
                                                        const KDevelop::VcsRevision & dstRevision,
                                                        KDevelop::VcsDiff::Type t,
                                                        KDevelop::IBasicVersionControl::RecursionMode recursion)
{
    QVariant srcrev=Handlers::kDevelopVcsRevisionHandler(&srcRevision);
    QVariant dstrev=Handlers::kDevelopVcsRevisionHandler(&dstRevision);
    
    QVariant obj=qVariantFromValue(fileOrDirectory);
    
    return new KrossVcsJob("diff",
                                         QVariantList() << obj << srcrev << dstrev << QVariant(t) << QVariant(recursion), action, m_plugin, action);
    return 0;
}

KDevelop::VcsJob * KrossDistributedVersionControl::log(const KUrl & file, const KDevelop::VcsRevision & rev, long unsigned int limit)
{
    QVariant revision=Handlers::kDevelopVcsRevisionHandler(&rev);

    return new KrossVcsJob("log", QVariantList() << file << revision << qlonglong(limit), action, m_plugin, action);
}

VcsJob* KrossDistributedVersionControl::log(const KUrl& file, const VcsRevision& rev, const VcsRevision& lim)
{
    QVariant revision=Handlers::kDevelopVcsRevisionHandler(&rev);
    QVariant limit=Handlers::kDevelopVcsRevisionHandler(&lim);
    
    return new KrossVcsJob("log", QVariantList() << file << revision << limit, action, m_plugin, action);
}

VcsJob* KrossDistributedVersionControl::annotate(const KUrl& file, const VcsRevision& rev)
{
    QVariant revision=Handlers::kDevelopVcsRevisionHandler(&rev);
    
    return new KrossVcsJob("annotate", QVariantList() << file << revision, action, m_plugin, action);
}

VcsJob* KrossDistributedVersionControl::merge(const VcsLocation& localOrRepoLocationSrc,
                                              const VcsLocation& localOrRepoLocationDst,
                                              const VcsRevision& srcRevision,
                                              const VcsRevision& dstRevision,
                                              const KUrl& localLocation )
{
    QVariant srcrev=Handlers::kDevelopVcsRevisionHandler(&srcRevision);
    QVariant dstrev=Handlers::kDevelopVcsRevisionHandler(&dstRevision);
    
    QVariant srcloc=Handlers::kDevelopVcsLocationHandler(&localOrRepoLocationSrc);
    QVariant dstloc=Handlers::kDevelopVcsLocationHandler(&localOrRepoLocationDst);
    
    return new KrossVcsJob("merge", QVariantList() << srcloc << dstloc << srcrev << dstrev << localLocation, action, m_plugin, action);

}

VcsJob* KrossDistributedVersionControl::resolve( const KUrl::List& localLocations, KDevelop::IBasicVersionControl::RecursionMode recursion )
{
    return new KrossVcsJob("resolve", QVariantList() << localLocations << recursion, action, m_plugin, action);
}


VcsJob* KrossDistributedVersionControl::init(const KUrl& location)
{
    return new KrossVcsJob("init", QVariantList() << location, action, m_plugin, action);
}

VcsJob* KrossDistributedVersionControl::push(const KUrl& localRepositoryLocation, const KDevelop::VcsLocation& localOrRepoLocationDst)
{
    return new KrossVcsJob("push", QVariantList() << localRepositoryLocation << localOrRepoLocationDst.localUrl(), action, m_plugin, action);
}

VcsJob* KrossDistributedVersionControl::pull(const KDevelop::VcsLocation& localOrRepoLocationSrc, const KUrl& localRepositoryLocation)
{
    return new KrossVcsJob("pull", QVariantList() << localOrRepoLocationSrc.localUrl() << localRepositoryLocation, action, m_plugin, action);
}

VcsJob* KrossDistributedVersionControl::createWorkingCopy(const KDevelop::VcsLocation & sourceRepository, const KUrl & destinationDirectory, KDevelop::IBasicVersionControl::RecursionMode mode)
{
    QVariant srcloc = Handlers::kDevelopVcsLocationHandler(&sourceRepository);
    QVariant recursion(mode==Recursive);

    return new KrossVcsJob("createWorkingCopy", QVariantList() << srcloc << destinationDirectory << recursion, action, m_plugin, action);
}

VcsJob* KrossDistributedVersionControl::reset(const KUrl &repository, const QStringList &args, const KUrl::List &files)
{
    return new KrossVcsJob("reset", QVariantList() << repository << args << files, action, m_plugin, action);
}

KDevelop::VcsLocationWidget* KrossDistributedVersionControl::vcsLocation(QWidget* parent) const
{
    return 0;
}
