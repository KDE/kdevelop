/***************************************************************************
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

#include "gitclonejob.h"

GitCloneJob::GitCloneJob(const QDir& d, KDevelop::IPlugin* parent, OutputJobVerbosity verbosity)
    : DVcsJob(d, parent, verbosity)
    , m_steps(0)
{
    connect(this, SIGNAL(resultsReady(KDevelop::VcsJob*)), SLOT(processResult()));
}
void GitCloneJob::processResult()
{
    if (error()) {
        QByteArray out = errorOutput();
        if (out.contains('\n')) {
            m_steps+=out.count('\n');
            emitPercent(m_steps, 6); //I'm counting 6 lines so it's a way to provide some progress, probably not the best
        }

        int end = qMax(out.lastIndexOf('\n'), out.lastIndexOf('\r'));
        int start = qMax(qMax(out.lastIndexOf('\n', end-1), out.lastIndexOf('\r', end-1)), 0);

        QByteArray info=out.mid(start+1, end-start-1);
        emit infoMessage(this, info);
    }
}
