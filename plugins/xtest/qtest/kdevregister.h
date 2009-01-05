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

#include <QList>
#include <KUrl>
#include <interfaces/istatus.h>
#include <veritas/testexecutableinfo.h>
#include <veritas/itesttreebuilder.h>

namespace Veritas { class Test; }
namespace KDevelop { class ProjectBaseItem; class IPlugin; }

class KJob;
class SuiteBuilderRunner;

namespace QTest
{
class ISettings;

/*! Constructs a QTest tree based on KDevelop information retrieved from the
 *  build system. */
class QXQTEST_EXPORT ModelBuilder : public Veritas::ITestTreeBuilder, public KDevelop::IStatus
{
Q_OBJECT
Q_INTERFACES( KDevelop::IStatus )

public:
    ModelBuilder();
    virtual ~ModelBuilder();

    virtual void reload(KDevelop::IProject*);
    virtual Veritas::Test* root() const;
    KDevelop::IProject* project() const;
    
// IStatus
    virtual QString statusName() const;

Q_SIGNALS:
    void clearMessage(KDevelop::IStatus*);
    void showMessage(KDevelop::IStatus*, const QString&, int timeout=0);
    void hideProgress(KDevelop::IStatus*);
    void showProgress(KDevelop::IStatus*, int minimum, int maximum, int value);
    void showErrorMessage(const QString&, int timeout);
// IStatus

private slots:
    void fetchTestCommands(KJob*);
    void suiteBuilderFinished();
    void slotShowProgress(int minimum, int maximum, int value);
    void doReload(KDevelop::ProjectBaseItem*);
    void connectBuilderPlugin(KDevelop::IPlugin* plugin);

private:
    KUrl buildRoot();
    Veritas::Test* m_root;
    QList<Veritas::TestExecutableInfo> m_testExes;
    SuiteBuilderRunner *m_runner;
    bool m_reloading;
    ISettings* m_settings;
    KDevelop::IProject* m_currentProject;

};

}

#endif // VERITAS_QTEST_MODELBUILDER_H
