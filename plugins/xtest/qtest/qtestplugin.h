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

#ifndef QTEST_QTESTPLUGIN_H
#define QTEST_QTESTPLUGIN_H

#include <KUrl>
#include <QMap>
#include <QLabel>
#include <QVariantList>
#include <veritas/itestframework.h>
#include <interfaces/iplugin.h>

class QTestOutputDelegate;

namespace KDevelop
{
class ContextMenuExtension;
class Context;
class ProjectFolderItem;
class IProject;
}

namespace Veritas { class Test; }
namespace QTest { class KDevRegister; }

/*! Makes the QTestRunner toolview available */
class QTestPlugin : public KDevelop::IPlugin, public Veritas::ITestFramework
{
Q_OBJECT
Q_INTERFACES(Veritas::ITestFramework)

public:
    explicit QTestPlugin(QObject* parent, const QVariantList& = QVariantList());
    virtual ~QTestPlugin();
    KDevelop::ContextMenuExtension contextMenuExtension(KDevelop::Context* context);
    virtual QString name() const;
    virtual Veritas::TestRunner* createRunner();

    virtual QWidget* createConfigWidget();
    virtual KDevelop::ProjectConfigSkeleton* configSkeleton(const QVariantList& args);

private slots:
    void newQTest();

private:
    KDevelop::ProjectFolderItem* m_dir;
    QTestOutputDelegate* m_delegate;
    KDevelop::IProject* m_proj; // scratch variable
};

#endif // QTEST_QTESTPLUGIN_H
