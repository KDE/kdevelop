/* This file is part of KDevelop
 *
 * Copyright 2018 Amish K. Naidu <amhndu@gmail.com>
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

#ifndef SCRATCHPADJOB_H
#define SCRATCHPADJOB_H

#include <outputview/outputjob.h>

#include <QProcess>

namespace KDevelop {
class OutputModel;
class ProcessLineMaker;
}
class KProcess;

class ScratchpadJob
    : public KDevelop::OutputJob
{
    Q_OBJECT

public:
    ScratchpadJob(const QString& command, const QString& title, QObject* parent);

    void start() override;
    bool doKill() override;

private Q_SLOTS:
    void processFinished(int exitCode, QProcess::ExitStatus status);
    void processError(QProcess::ProcessError error);

private:
    KDevelop::OutputModel* outputModel() const;

    KProcess* m_process;
    KDevelop::ProcessLineMaker* m_lineMaker;
};

#endif // SCRATCHPADJOB_H
