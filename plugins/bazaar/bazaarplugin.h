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
    ~BazaarPlugin() override;

    QString name() const override;

    bool isValidRemoteRepositoryUrl(const QUrl& remoteLocation) override;

    KDevelop::VcsJob* add(const QList<QUrl>& localLocations, RecursionMode recursion=Recursive) override;
    KDevelop::VcsJob* annotate(const QUrl& localLocation, const KDevelop::VcsRevision& rev) override;
    KDevelop::VcsJob* commit(const QString& message, const QList<QUrl>& localLocations, RecursionMode recursion=Recursive) override;
    KDevelop::VcsJob* copy(const QUrl& localLocationSrc, const QUrl& localLocationDstn) override;
    KDevelop::VcsImportMetadataWidget* createImportMetadataWidget(QWidget* parent) override;
    KDevelop::VcsJob* createWorkingCopy(const KDevelop::VcsLocation& sourceRepository, const QUrl& destinationDirectory, RecursionMode recursion=Recursive) override;
    KDevelop::VcsJob* diff(const QUrl& fileOrDirectory, const KDevelop::VcsRevision& srcRevision, const KDevelop::VcsRevision& dstRevision, KDevelop::VcsDiff::Type, RecursionMode recursion=Recursive) override;
    KDevelop::VcsJob* init(const QUrl& localRepositoryRoot) override;
    bool isVersionControlled(const QUrl& localLocation) override;
    KDevelop::VcsJob* log(const QUrl& localLocation, const KDevelop::VcsRevision& rev, long unsigned int limit) override;
    KDevelop::VcsJob* log(const QUrl& localLocation, const KDevelop::VcsRevision& rev, const KDevelop::VcsRevision& limit) override;
    KDevelop::VcsJob* move(const QUrl& localLocationSrc, const QUrl& localLocationDst) override;
    KDevelop::VcsJob* pull(const KDevelop::VcsLocation& localOrRepoLocationSrc, const QUrl& localRepositoryLocation) override;
    KDevelop::VcsJob* push(const QUrl& localRepositoryLocation, const KDevelop::VcsLocation& localOrRepoLocationDst) override;
    KDevelop::VcsJob* remove(const QList<QUrl>& localLocations) override;
    KDevelop::VcsJob* repositoryLocation(const QUrl& localLocation) override;
    KDevelop::VcsJob* resolve(const QList<QUrl>& localLocations, RecursionMode recursio=Recursive) override;
    KDevelop::VcsJob* revert(const QList<QUrl>& localLocations, RecursionMode recursion=Recursive) override;
    KDevelop::VcsJob* status(const QList<QUrl>& localLocations, RecursionMode recursion=Recursive) override;
    KDevelop::VcsJob* update(const QList<QUrl>& localLocations, const KDevelop::VcsRevision& rev, RecursionMode recursion=Recursive) override;
    KDevelop::VcsLocationWidget* vcsLocation(QWidget* parent) const override;
    KDevelop::ContextMenuExtension contextMenuExtension(KDevelop::Context* context) override;

private slots:
    void parseBzrStatus(KDevelop::DVcsJob* job);
    void parseBzrLog(KDevelop::DVcsJob* job);
    void parseBzrRoot(KDevelop::DVcsJob* job);

private:
    KDevelop::VcsPluginHelper* m_vcsPluginHelper;
};

#endif // BAZAAR_BAZAARPLUGIN_H
