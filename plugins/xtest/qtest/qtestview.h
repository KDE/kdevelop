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

#include <iplugin.h>
#include <QVariantList>

class QTestViewFactory;
namespace Veritas { class Test; }

class QTestView : public KDevelop::IPlugin
{
Q_OBJECT

public:
    explicit QTestView(QObject* parent, const QVariantList & = QVariantList());
    virtual ~QTestView();
    QWidget* spawn();

private:
    QString fetchBuildRoot();
    QString fetchRegXML();
    Veritas::Test* registerTests(const QString& regXML, const QString& rootDir);

private:
    QTestViewFactory* m_factory;
};

#endif // QTEST_QTESTPLUGIN_H
