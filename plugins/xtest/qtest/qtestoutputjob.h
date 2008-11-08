/* KDevelop xUnit plugin
 *    Copyright 1999-2001 Bernd Gehrmann and the KDevelop Team <bernd@kdevelop.org>
 *    Copyright 2008 by Hamish Rodda <rodda@kde.org>
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

#ifndef QTEST_QTESTOUTPUTJOB_H
#define QTEST_QTESTOUTPUTJOB_H

#include <outputview/outputjob.h>
#include "qxqtestexport.h"

namespace KDevelop
{
class IOutputView;
class IProject;
}

namespace QTest { class QTestCase; }

class KProcess;
class QTestOutputModel;
class QTestOutputDelegate;
class QTestView;

class QXQTEST_EXPORT QTestOutputJob : public KDevelop::OutputJob
{
    Q_OBJECT

public:
    QTestOutputJob(QTestOutputDelegate *parent, QTest::QTestCase*);
    virtual void start();

protected:
    QTestOutputModel* model() const;

private:
    void outputFile(const KUrl& path);

private:
    QTestOutputDelegate* delegate() const;
    QTest::QTestCase* m_caze;
};

#endif // QTEST_QTESTOUTPUTJOB_H
