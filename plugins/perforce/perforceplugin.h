/***************************************************************************
 *   Copyright 2010  Morten Danielsen Volden                               *
 *                                                                         *
 *   This program is free software: you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation, either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 ***************************************************************************/

#ifndef KDEVPERFORCEPLUGIN_H
#define KDEVPERFORCEPLUGIN_H

#include <vcs/interfaces/icentralizedversioncontrol.h>
#include <vcs/vcsstatusinfo.h>
#include <interfaces/iplugin.h>
#include <outputview/outputjob.h>


#include <QVariantList>
#include <QString>

#include <memory>

class QMenu;
class QFileInfo;
class QDir;


namespace KDevelop
{
class ContextMenuExtension;
class VcsPluginHelper;
class DVcsJob;
}


class PerforcePlugin : public KDevelop::IPlugin, public KDevelop::ICentralizedVersionControl
{
    Q_OBJECT
    Q_INTERFACES(KDevelop::IBasicVersionControl KDevelop::ICentralizedVersionControl)

    friend class PerforcePluginTest;
public:
    explicit PerforcePlugin(QObject* parent, const QVariantList & = QVariantList());
    ~PerforcePlugin() override;

    //@{
    /** Methods inherited from KDevelop::IBasicVersionControl */
    QString name() const override;

    KDevelop::VcsImportMetadataWidget* createImportMetadataWidget(QWidget* parent) override;

    bool isValidRemoteRepositoryUrl(const QUrl& remoteLocation) override;
    bool isVersionControlled(const QUrl& localLocation) override;

    KDevelop::VcsJob* repositoryLocation(const QUrl& localLocation) override;

    KDevelop::VcsJob* add(const QList<QUrl>& localLocations,
                          RecursionMode recursion = IBasicVersionControl::Recursive) override;
    KDevelop::VcsJob* remove(const QList<QUrl>& localLocations) override;

    KDevelop::VcsJob* copy(const QUrl& localLocationSrc,
                           const QUrl& localLocationDstn) override;
    KDevelop::VcsJob* move(const QUrl& localLocationSrc,
                           const QUrl& localLocationDst) override;
    KDevelop::VcsJob* status(const QList<QUrl>& localLocations,
                             RecursionMode recursion = IBasicVersionControl::Recursive) override;

    KDevelop::VcsJob* revert(const QList<QUrl>& localLocations,
                             RecursionMode recursion = IBasicVersionControl::Recursive) override;

    KDevelop::VcsJob* update(const QList<QUrl>& localLocations,
                             const KDevelop::VcsRevision& rev = KDevelop::VcsRevision::createSpecialRevision(KDevelop::VcsRevision::Head),
                             KDevelop::IBasicVersionControl::RecursionMode recursion = KDevelop::IBasicVersionControl::Recursive) override;

    KDevelop::VcsJob* commit(const QString& message,
                             const QList<QUrl>& localLocations,
                             KDevelop::IBasicVersionControl::RecursionMode recursion = KDevelop::IBasicVersionControl::Recursive) override;

    KDevelop::VcsJob* diff(const QUrl& fileOrDirectory,
                           const KDevelop::VcsRevision& srcRevision,
                           const KDevelop::VcsRevision& dstRevision,
                           KDevelop::VcsDiff::Type = KDevelop::VcsDiff::DiffUnified,
                           KDevelop::IBasicVersionControl::RecursionMode recursion = KDevelop::IBasicVersionControl::Recursive) override;

    KDevelop::VcsJob* log(const QUrl& localLocation,
                          const KDevelop::VcsRevision& rev,
                          unsigned long limit = 0) override;

    KDevelop::VcsJob* log(const QUrl& localLocation,
                          const KDevelop::VcsRevision& rev = KDevelop::VcsRevision::createSpecialRevision(KDevelop::VcsRevision::Base),
                          const KDevelop::VcsRevision& limit = KDevelop::VcsRevision::createSpecialRevision(KDevelop::VcsRevision::Start)) override;

    KDevelop::VcsJob* annotate(const QUrl& localLocation,
                               const KDevelop::VcsRevision& rev = KDevelop::VcsRevision::createSpecialRevision(KDevelop::VcsRevision::Head)) override;

    KDevelop::VcsJob* resolve(const QList<QUrl>& localLocations,
                              KDevelop::IBasicVersionControl::RecursionMode recursion) override;

    KDevelop::VcsJob* createWorkingCopy(const  KDevelop::VcsLocation & sourceRepository,
                                        const QUrl & destinationDirectory,
                                        KDevelop::IBasicVersionControl::RecursionMode recursion = IBasicVersionControl::Recursive) override;


    KDevelop::VcsLocationWidget* vcsLocation(QWidget* parent) const override;
    //@}

    //@{
    /** Methods inherited from KDevelop::ICentralizedVersionControl  */
    KDevelop::VcsJob* edit(const QUrl& localLocation) override;

    KDevelop::VcsJob* unedit(const QUrl& localLocation) override;

    KDevelop::VcsJob* localRevision(const QUrl& localLocation,
                                    KDevelop::VcsRevision::RevisionType) override;

    KDevelop::VcsJob* import(const QString & commitMessage,
                             const QUrl & sourceDirectory,
                             const  KDevelop::VcsLocation & destinationRepository) override;
    //@}

    /// This plugin implements its own edit method
    KDevelop::VcsJob* edit(const QList<QUrl>& localLocations);


    KDevelop::ContextMenuExtension contextMenuExtension(KDevelop::Context* context) override;


public Q_SLOTS:

    /// invoked by context-menu
    void ctxEdit();
//   void ctxUnedit();
//   void ctxLocalRevision();
//   void ctxImport();

private slots:
    void parseP4StatusOutput(KDevelop::DVcsJob* job);
    void parseP4DiffOutput(KDevelop::DVcsJob* job);
    void parseP4LogOutput(KDevelop::DVcsJob* job);
    void parseP4AnnotateOutput(KDevelop::DVcsJob* job);



private:
    bool isValidDirectory(const QUrl & dirPath);
    KDevelop::DVcsJob* p4fstatJob(const QFileInfo& curFile,
                                  KDevelop::OutputJob::OutputJobVerbosity verbosity = KDevelop::OutputJob::Verbose);

    bool parseP4fstat(const QFileInfo& curFile,
                      KDevelop::OutputJob::OutputJobVerbosity verbosity = KDevelop::OutputJob::Verbose);

    KDevelop::VcsJob* errorsFound(const QString& error,
                                  KDevelop::OutputJob::OutputJobVerbosity verbosity = KDevelop::OutputJob::Verbose);

    QString getRepositoryName(const QFileInfo& curFile);


    void setEnvironmentForJob(KDevelop::DVcsJob* job, QFileInfo const& fsObject);
    QList<QVariant> getQvariantFromLogOutput(QStringList const& outputLines);

    std::unique_ptr<KDevelop::VcsPluginHelper> m_common;
    QMenu* m_perforcemenu;
    QString m_perforceConfigName;
    QString m_perforceExecutable;
    QAction* m_edit_action;
};

#endif // PERFORCEPLUGIN_H
