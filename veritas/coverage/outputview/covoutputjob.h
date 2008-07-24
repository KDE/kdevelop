/* KDevelop xUnit plugin
 *    Copyright 2008 Manuel Breugelmans <mbr.nxi@gmail.com>
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



#ifndef VERITAS_COVERAGE_COVOUTPUTJOB_H
#define VERITAS_COVERAGE_COVOUTPUTJOB_H

#include <interfaces/iplugin.h>
#include <outputview/outputjob.h>

#include <QProcess>
#include <QString>
#include <KUrl>

namespace KDevelop
{
class IOutputView;
class IProject;
class ProcessLineMaker;
}

class KProcess;

namespace Veritas
{

class CovOutputModel;
class CovOutputDelegate;
class CovView;

class CovOutputJob : public KDevelop::OutputJob
{
    Q_OBJECT

public:
    CovOutputJob(CovOutputDelegate *parent, const KUrl& root);
    virtual void start();

protected:
    CovOutputModel* model() const;

private Q_SLOTS:
    void slotFinished();
    void slotError(QProcess::ProcessError error);

private:
    CovOutputDelegate* delegate() const;
    KDevelop::ProcessLineMaker* m_lineMaker;
    KProcess* m_lcov;
    KUrl m_root;
    QString m_tmpPath;
};

}

#endif // VERITAS_COVERAGE_COVOUTPUTJOB_H
