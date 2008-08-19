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

#ifndef QTEST_QTESTVIEW_H
#define QTEST_QTESTVIEW_H

#include <KUrl>
#include <QMap>
#include <QVariantList>

#include <interfaces/iplugin.h>

class QTestRunnerViewFactory;
class QTestOutputDelegate;

namespace KDevelop
{
class ContextMenuExtension;
class Context;
class ProjectFolderItem;
class IProject;
}

namespace Sublime { class View; }
namespace Veritas { class Test; }

/*! Makes the QTestRunner toolview available */
class QTestPlugin : public KDevelop::IPlugin
{
Q_OBJECT

public:
    explicit QTestPlugin(QObject* parent, const QVariantList& = QVariantList());
    virtual ~QTestPlugin();
    KDevelop::ContextMenuExtension contextMenuExtension(KDevelop::Context* context);
    QMap<Sublime::View*, int> m_tools;

private slots:
    void newQTest();
    void openVerbose(Veritas::Test*);
    void maybeRemoveResultsView(Sublime::View*);
    void fixMovedResultsView(Sublime::View*);

private:
    void removeAllResultsViews();
    void removeResultsView(const QString& docId);

private:
    QTestRunnerViewFactory* m_factory;
    KDevelop::ProjectFolderItem* m_dir;
    QTestOutputDelegate* m_delegate;
    KDevelop::IProject* m_proj;
};

#endif // QTEST_QTESTVIEW_H
