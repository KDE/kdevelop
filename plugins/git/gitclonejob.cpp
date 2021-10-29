/*
    SPDX-FileCopyrightText: 2010 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "gitclonejob.h"

GitCloneJob::GitCloneJob(const QDir& d, KDevelop::IPlugin* parent, OutputJobVerbosity verbosity)
    : GitJob(d, parent, verbosity)
    , m_steps(0)
{
    connect(this, &GitCloneJob::resultsReady, this, &GitCloneJob::processResult);
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

        const QString info = QString::fromUtf8(out.mid(start+1, end-start-1));
        emit infoMessage(this, info);
    }
}
