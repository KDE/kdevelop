/* KDevelop xUnit plugin
 *
 * Copyright 2008 Manuel Breugelmans <mbr.nxi@gmail.com>
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

#include "lcovjobtest.h"
#include "../lcovjob.h"
#include "../covoutputdelegate.h"
#include "plugincontrollerstub.h"
#include "corestub.h"
#include "pluginstub.h"
#include <KProcess>
#include <QtTest/QTest>
#include <qtest_kde.h>

using Veritas::CovOutputDelegate;
using Veritas::LcovJob;
using Veritas::LcovJobTest;

namespace
{
class MyKProcess : public KProcess
{
public:
    MyKProcess() : m_startCalled(false) {}
    virtual ~MyKProcess() {}
    virtual void start() { m_startCalled = true; }
    bool m_startCalled;
};
}

void LcovJobTest::initTestCase()
{
    m_core = new TestStubs::Core;
    Q_ASSERT(m_core);
    m_plugCtrl = new TestStubs::PluginController;
    m_plugCtrl->m_pluginForExtension = new TestStubs::Plugin(KGlobal::mainComponent(), 0);
    m_core->m_pluginController = m_plugCtrl;
}

void LcovJobTest::init()
{
}

void LcovJobTest::cleanup()
{
}

void LcovJobTest::basicRun()
{
    MyKProcess* proc = new MyKProcess;
    CovOutputDelegate* del = new CovOutputDelegate;
    LcovJob* m_job = new LcovJob(KUrl("/my/root/dir"));
    m_job->setProcess(proc);
    m_job->setDelegate(del);
    m_job->start();
}

QTEST_KDEMAIN( LcovJobTest, NoGUI)
#include "lcovjobtest.moc"
