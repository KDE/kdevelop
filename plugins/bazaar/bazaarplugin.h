/***************************************************************************
 *   Copyright 2013-2014 Maciej Poleski                                    *
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

#ifndef BAZAAR_BAZAARPLUGIN_H
#define BAZAAR_BAZAARPLUGIN_H

#include <QtCore/QVariantList>

#include <interfaces/iplugin.h>
#include <vcs/interfaces/idistributedversioncontrol.h>
#include <vcs/vcspluginhelper.h>

namespace KDevelop
{
class DVcsJob;
}

class BazaarPlugin : public KDevelop::IPlugin, public KDevelop::IDistributedVersionControl
{
    Q_OBJECT
    Q_INTERFACES(KDevelop::IBasicVersionControl KDevelop::IDistributedVersionControl)
public:
    explicit BazaarPlugin(QObject* parent, const QVariantList& args = QVariantList());
    virtual ~BazaarPlugin();

    virtual bool hasError() const override;
    virtual QString errorDescription() const override;

    virtual QString name() const override;

    virtual KDevelop::VcsJob* add(const KUrl::List& localLocations, RecursionMode recursion=Recursive) override;
    virtual KDevelop::VcsJob* annotate(const KUrl& localLocation, const KDevelop::VcsRevision& rev) override;
    virtual KDevelop::VcsJob* commit(const QString& message, const KUrl::List& localLocations, RecursionMode recursion=Recursive) override;
    virtual KDevelop::VcsJob* copy(const KUrl& localLocationSrc, const KUrl& localLocationDstn) override;
    virtual KDevelop::VcsImportMetadataWidget* createImportMetadataWidget(QWidget* parent) override;
    virtual KDevelop::VcsJob* createWorkingCopy(const KDevelop::VcsLocation& sourceRepository, const KUrl& destinationDirectory, RecursionMode recursion=Recursive) override;
    virtual KDevelop::VcsJob* diff(const KUrl& fileOrDirectory, const KDevelop::VcsRevision& srcRevision, const KDevelop::VcsRevision& dstRevision, KDevelop::VcsDiff::Type, RecursionMode recursion=Recursive) override;
    virtual KDevelop::VcsJob* init(const KUrl& localRepositoryRoot) override;
    virtual bool isVersionControlled(const KUrl& localLocation) override;
    virtual KDevelop::VcsJob* log(const KUrl& localLocation, const KDevelop::VcsRevision& rev, long unsigned int limit) override;
    virtual KDevelop::VcsJob* log(const KUrl& localLocation, const KDevelop::VcsRevision& rev, const KDevelop::VcsRevision& limit) override;
    virtual KDevelop::VcsJob* move(const KUrl& localLocationSrc, const KUrl& localLocationDst) override;
    virtual KDevelop::VcsJob* pull(const KDevelop::VcsLocation& localOrRepoLocationSrc, const KUrl& localRepositoryLocation) override;
    virtual KDevelop::VcsJob* push(const KUrl& localRepositoryLocation, const KDevelop::VcsLocation& localOrRepoLocationDst) override;
    virtual KDevelop::VcsJob* remove(const KUrl::List& localLocations) override;
    virtual KDevelop::VcsJob* repositoryLocation(const KUrl& localLocation) override;
    virtual KDevelop::VcsJob* resolve(const KUrl::List& localLocations, RecursionMode recursio=Recursive) override;
    virtual KDevelop::VcsJob* revert(const KUrl::List& localLocations, RecursionMode recursion=Recursive) override;
    virtual KDevelop::VcsJob* status(const KUrl::List& localLocations, RecursionMode recursion=Recursive) override;
    virtual KDevelop::VcsJob* update(const KUrl::List& localLocations, const KDevelop::VcsRevision& rev, RecursionMode recursion=Recursive) override;
    virtual KDevelop::VcsLocationWidget* vcsLocation(QWidget* parent) const override;
    virtual KDevelop::ContextMenuExtension contextMenuExtension(KDevelop::Context* context) override;

private slots:
    void parseBzrStatus(KDevelop::DVcsJob* job);
    void parseBzrLog(KDevelop::DVcsJob* job);
    void parseBzrRoot(KDevelop::DVcsJob* job);

private:
    KDevelop::VcsPluginHelper* m_vcsPluginHelper;

    bool m_hasError;
    QString m_errorDescription;
};

#endif // BAZAAR_BAZAARPLUGIN_H
