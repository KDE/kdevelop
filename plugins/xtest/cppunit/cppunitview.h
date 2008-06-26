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

#ifndef CPPUNIT_CPPUNITPLUGIN_H
#define CPPUNIT_CPPUNITPLUGIN_H

#include <iplugin.h>
#include <QVariantList>

class CppUnitViewFactory;
namespace Veritas
{
class Test;
};

class CppUnitView : public KDevelop::IPlugin
{
    Q_OBJECT
public:
    explicit CppUnitView(QObject* parent, const QVariantList & = QVariantList());
    virtual ~CppUnitView();
    QWidget* spawn();

private:
    Veritas::Test* registerTests(const QString& exe);
    QString fetchExe();

private:
    CppUnitViewFactory* m_factory;
};

#endif // CPPUNIT_CPPUNITPLUGIN_H
