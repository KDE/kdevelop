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

#ifndef BAZAAR_COPYJOB_H
#define BAZAAR_COPYJOB_H

#include <KUrl>

#include <vcs/vcsjob.h>

#include <time.h>

namespace KIO
{
class Job;
}

class BazaarPlugin;

class CopyJob : public KDevelop::VcsJob
{
    Q_OBJECT

public:
    CopyJob(const KUrl& localLocationSrc, const KUrl& localLocationDstn, BazaarPlugin* parent = 0, OutputJobVerbosity verbosity = OutputJob::Verbose);

    virtual KDevelop::IPlugin* vcsPlugin() const override;
    virtual KDevelop::VcsJob::JobStatus status() const override;
    virtual QVariant fetchResults() override;
    virtual void start() override;

protected:
    virtual bool doKill() override;

private slots:
    void finish(KJob*);
    void addToVcs(KIO::Job* job, const KUrl& from, const KUrl& to, time_t mtime, bool directory, bool renamed);

private:
    BazaarPlugin* m_plugin;
    KUrl m_source;
    KUrl m_destination;

    JobStatus m_status;
    QPointer<KJob> m_job;
};

#endif // BAZAAR_COPYJOB_H
