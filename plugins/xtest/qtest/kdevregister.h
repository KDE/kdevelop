/*
* KDevelop xUnit integration
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

#ifndef VERITAS_QTEST_KDEVREGISTER_H
#define VERITAS_QTEST_KDEVREGISTER_H

#include "qxqtestexport.h"
#include "iregister.h"
#include <QList>
#include <KUrl>
#include <interfaces/istatus.h>

namespace Veritas { class Test; }
namespace KDevelop { class ProjectTestTargetItem; class IRunController; }
class KJob;
class SuiteBuilderRunner;

namespace QTest
{

/*!
Constructs a QTest tree based on KDevelop information retrieved from the
build system.
*/
class QXQTEST_EXPORT KDevRegister : public IRegister, public KDevelop::IStatus
{
Q_OBJECT
Q_INTERFACES( KDevelop::IStatus )

public:
    KDevRegister();
    virtual ~KDevRegister();

    virtual void reload();
    virtual Veritas::Test* root() const;

// IStatus
    virtual QString statusName() const;

Q_SIGNALS:
    void clearMessage();
    void showMessage(const QString&, int timeout=0);
    void hideProgress();
    void showProgress(int minimum, int maximum, int value);
    void showErrorMessage(QString, int timeout);
// IStatus

private slots:
    void fetchTestCommands(KJob*);
    void suiteBuilderFinished();

private:
    Veritas::Test* m_root;
    QList<KDevelop::ProjectTestTargetItem*> m_testTargets;
    QList<QString> m_testNames;
    SuiteBuilderRunner *m_runner;
    bool m_reloading;
};

}

#endif // VERITAS_QTEST_KDEVREGISTER_H
