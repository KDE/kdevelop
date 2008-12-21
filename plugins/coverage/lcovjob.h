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

#ifndef VERITAS_COVERAGE_LCOVJOB_H
#define VERITAS_COVERAGE_LCOVJOB_H

#include <interfaces/iplugin.h>
#include <outputview/outputjob.h>
#include "coverageexport.h"

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
class LcovInfoParser;

/*! Executes the lcov coverage script in an outputview */
class VERITAS_COVERAGE_EXPORT LcovJob : public KDevelop::OutputJob
{
    Q_OBJECT

public:
    explicit LcovJob(const KUrl& root, QObject* parent=0);
    virtual ~LcovJob();
    void setProcess(KProcess *proc); // takes ownership.
    void setParser(LcovInfoParser* parser);
    void setDelegate(CovOutputDelegate* delegate);
    virtual void start();

protected:
    CovOutputModel* model() const;

private Q_SLOTS:
    void slotFinished();
    void slotError(QProcess::ProcessError error);

private:
    void initProcess();
    void initOutputView();
    void initParser();

private:
    KDevelop::ProcessLineMaker* m_lineMaker;
    KProcess* m_lcov;
    KUrl m_root;
    QString m_tmpPath;
    LcovInfoParser* m_parser;
};

}

#endif // VERITAS_COVERAGE_LCOVJOB_H
