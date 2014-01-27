/***************************************************************************
 *   Copyright 2008 Evgeniy Ivanov <powerfox@kde.ru>                       *
 *   Copyright 2009 Hugo Parente Lima <hugo.pl@gmail.com>                  *
 *   Copyright 2010 Aleix Pol Gonzalez <aleixpol@kde.org>                  *
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

#ifndef GITPLUGINCHECKINREPOSITORYJOB_H
#define GITPLUGINCHECKINREPOSITORYJOB_H

#include <vcs/interfaces/icontentawareversioncontrol.h>
#include <QProcess>

class GitPluginCheckInRepositoryJob : public KDevelop::CheckInRepositoryJob
{
    Q_OBJECT
public:
    GitPluginCheckInRepositoryJob(KTextEditor::Document* document, const QString& rootDirectory);
    virtual ~GitPluginCheckInRepositoryJob();
    virtual void start();

private slots:
    void repositoryQueryFinished(int);
    void processFailed(QProcess::ProcessError);

private:
    QProcess* m_hashjob;
    QProcess* m_findjob;
    QString m_rootDirectory;
};

#endif // GITPLUGINCHECKINREPOSITORYJOB_H
