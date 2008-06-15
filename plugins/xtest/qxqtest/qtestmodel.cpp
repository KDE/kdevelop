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

#include "qtestmodel.h"
#include "qtestcase.h"
#include "qtestsuite.h"
#include "qtestcommand.h"
#include "qtestregister.h"
#include "qtestbase.h"
#include <QIODevice>
#include <klocalizedstring.h>

using QxQTest::QTestModel;
using QxQTest::QTestBase;
using QxQTest::QTestSuite;
using QxQTest::QTestCase;
using QxQTest::QTestBase;
using QxQTest::QTestCommand;
using QxRunner::RunnerModel;

QTestModel::QTestModel(QObject* parent)
        : RunnerModel(parent), m_root("")
{
    setExpectedResults(QxRunner::RunWarning | QxRunner::RunError);
}

QTestModel::~QTestModel()
{
}

void QTestModel::setRoot(const QString& root)
{
    m_root = root;
}

QString QTestModel::name() const
{
    return tr("QxQTest");
}

QString QTestModel::about() const
{
    QString version("4.4");
    QString aboutModel = i18n("for QTestLib") + ' ' + version;
    return aboutModel;
}

void QTestModel::readTests(QIODevice* dev)
{
    QTestRegister reg;
    reg.setRootDir(m_root);
    reg.addFromXml(dev);
    setRootItem(reg.rootItem());
}

#include "qtestmodel.moc"
