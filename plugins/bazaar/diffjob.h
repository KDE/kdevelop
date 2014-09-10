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

#ifndef BAZAAR_DIFFJOB_H
#define BAZAAR_DIFFJOB_H

#include <QtCore/QVariant>

#include <vcs/vcsjob.h>

namespace KDevelop
{
class DVcsJob;
}

class QDir;
class BazaarPlugin;

class DiffJob : public KDevelop::VcsJob
{
    Q_OBJECT

public:
    DiffJob(const QDir& workingDir, const QString& revisionSpecRange, const QUrl& fileOrDirectory, BazaarPlugin* parent = 0, OutputJobVerbosity verbosity = OutputJob::Silent);

    virtual KDevelop::IPlugin* vcsPlugin() const override;
    virtual KDevelop::VcsJob::JobStatus status() const override;
    virtual QVariant fetchResults() override;
    virtual void start() override;

protected:
    virtual bool doKill() override;

private slots:
    void prepareResult(KJob*);

private:
    BazaarPlugin* m_plugin;
    QVariant m_result;

    JobStatus m_status;
    QPointer<KDevelop::DVcsJob> m_job;
};

#endif // BAZAAR_DIFFJOB_H
