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

#include "qtestviewdata.h"
#include "xmlregister.h"
#include "kdevregister.h"
#include "qtestsuite.h"
#include "qtestsettings.h"
#include "qtestoutputjob.h"
#include "qtestoutputdelegate.h"

#include <KSharedConfig>
#include <KConfigGroup>

#include <interfaces/context.h>
#include <interfaces/contextmenuextension.h>
#include <interfaces/idocument.h>
#include <interfaces/iproject.h>
#include <interfaces/iprojectcontroller.h>
#include <interfaces/iuicontroller.h>
#include <outputview/ioutputview.h>
#include <project/projectmodel.h>
#include <project/interfaces/ibuildsystemmanager.h>
#include <project/interfaces/iprojectfilemanager.h>
#include <shell/core.h>
#include <shell/documentcontroller.h>
#include <veritas/test.h>

using namespace Veritas;
using namespace KDevelop;
using namespace QTest;

QTestViewData::QTestViewData(ITestFramework* framework)
    : Veritas::ITestRunner(framework),
      m_settings(0),
      m_lock(false),
      m_delegate(new QTestOutputDelegate(this))
{
}

QTestViewData::~QTestViewData()
{
    if (m_settings) delete m_settings;
}

void QTestViewData::registerTests()
{
/*    kDebug() << "Loading test registration XML: " << fetchRegXML();
    QFile* testXML = new QFile(fetchRegXML());
    XmlRegister reg;
    reg.setSettings(new Settings(project()));
    reg.setRootDir(fetchBuildRoot());
    reg.setSource(testXML);
    reg.reload();*/

    kDebug() << "";
    if (!project()) {
        kDebug() << "!project()";
        return;
    }
    if (m_lock) {
        kDebug() << "currently buzzy";
        return;
    }
    m_lock = true;

    if (m_settings) delete m_settings;
    m_settings = new Settings(project());
    KDevRegister* reg = new KDevRegister();
    reg->setProject(project());
    reg->setSettings(m_settings);
    connect(reg, SIGNAL(reloadFinished(Veritas::Test*)),
            SIGNAL(registerFinished(Veritas::Test*)));
    connect(reg, SIGNAL(reloadFinished(Veritas::Test*)),
            SLOT(resetLock()));
    connect(reg, SIGNAL(reloadFailed()), SLOT(resetLock()));
    reg->reload();
}


void QTestViewData::resetLock()
{
    m_lock = false;
    sender()->deleteLater();
}

// should be moved to xmlregister
QString QTestViewData::fetchBuildRoot()
{
    if (project() == 0) {
        return "";
    }
    IBuildSystemManager* man = project()->buildSystemManager();
    ProjectFolderItem* pfi = project()->projectItem();
    return man->buildDirectory(pfi).pathOrUrl();
}

// should be moved to xmlregister
QString QTestViewData::fetchRegXML()
{
    if (project() == 0) {
        return "";
    }
    KSharedConfig::Ptr cfg = project()->projectConfiguration();
    KConfigGroup group(cfg.data(), "QTest");
    return KUrl(group.readEntry("Test Registration")).pathOrUrl();
}

void QTestViewData::openVerbose(Veritas::Test* t)
{
    QTestCase* caze = dynamic_cast<QTestCase*>(t);
    if (!caze) return;
    QTestOutputJob* job = new QTestOutputJob(m_delegate, caze);
    ICore::self()->runController()->registerJob(job);
}

#include "qtestviewdata.moc"
