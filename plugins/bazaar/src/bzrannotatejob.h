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

#ifndef BAZAAR_BZRANNOTATEJOB_H
#define BAZAAR_BZRANNOTATEJOB_H

#include <QtCore/QObject>
#include <QtCore/QStringList>
#include <QtCore/QHash>
#include <QtCore/QDir>

#include <KUrl>

#include <vcs/vcsevent.h>
#include <vcs/vcsjob.h>

class QDir;
namespace KDevelop
{
class DVcsJob;
}

class BzrAnnotateJob : public KDevelop::VcsJob
{
    Q_OBJECT
public:

    explicit BzrAnnotateJob(const QDir& workingDir, const QString& revisionSpec, const KUrl& localLocation, KDevelop::IPlugin* parent = 0, OutputJobVerbosity verbosity = OutputJob::Verbose);

    virtual QVariant fetchResults() override;
    virtual void start() override;
    virtual JobStatus status() const override;
    virtual KDevelop::IPlugin* vcsPlugin() const override;

protected:
    virtual bool doKill() override;

private slots:
    void parseBzrAnnotateOutput(KDevelop::DVcsJob* job);
    void parseNextLine();
    void prepareCommitInfo(std::size_t revision);
    void parseBzrLog(KDevelop::DVcsJob* job);

private:
    QDir m_workingDir;
    QString m_revisionSpec;
    KUrl m_localLocation;
    KDevelop::IPlugin* m_vcsPlugin;

    JobStatus m_status;
    QPointer<KJob> m_job;

    QStringList m_outputLines;
    int m_currentLine;
    QHash<int, KDevelop::VcsEvent> m_commits;
    QVariantList m_results;
};

#endif // BAZAAR_BZRANNOTATEJOB_H
