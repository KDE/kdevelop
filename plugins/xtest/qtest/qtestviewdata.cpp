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
#include "qtestregister.h"
#include "qtestsuite.h"
#include "config/qtestsettings.h"

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

QTestViewData::QTestViewData(QObject* parent) : Veritas::TestViewData(parent)
{
    m_id = QTestViewData::id;
    QTestViewData::id += 1;
}

QTestViewData::~QTestViewData() {}

Test* QTestViewData::registerTests()
{
    kDebug() << "Loading test registration XML: " << fetchRegXML();
    QFile* testXML = new QFile(fetchRegXML());
    QTestRegister reg;
    reg.setSettings(new Settings(project()));
    reg.setRootDir(fetchBuildRoot());
    reg.addFromXml(testXML);
    return reg.rootItem();
}

QString QTestViewData::fetchBuildRoot()
{
    if (project() == 0) {
        return "";
    }
    IBuildSystemManager* man = project()->buildSystemManager();
    ProjectFolderItem* pfi = project()->projectItem();
    return man->buildDirectory(pfi).pathOrUrl();
}

QString QTestViewData::resultsViewId()
{
    return QString("org.kdevelop.QTestResultsView") + QString::number(m_id);
}

QString QTestViewData::fetchRegXML()
{
    if (project() == 0) {
        return "";
    }
    KSharedConfig::Ptr cfg = project()->projectConfiguration();
    KConfigGroup group(cfg.data(), "QTest");
    return KUrl(group.readEntry("Test Registration")).pathOrUrl();
}

int QTestViewData::id = 0;

#include "qtestviewdata.moc"
