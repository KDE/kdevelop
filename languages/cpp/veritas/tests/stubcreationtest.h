/*
 * KDevelop xUnit test support
 *
 * Copyright 2008 Manuel Breugelmans
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef QTEST_STUBCREATIONTEST_H_INCLUDED
#define QTEST_STUBCREATIONTEST_H_INCLUDED

#include <QtCore/QObject>
#include "../classskeleton.h"

namespace Veritas { class StubConstructor; }

namespace Veritas { namespace Test {

class DeclarationFactory;
class StubCreationTest : public QObject
{
Q_OBJECT
private slots:
    void init();
    void cleanup();

    void appendStubToClassName();
    void basicConstructor();
    void basicDestructor();
    void basicInterface();
    void nonVoidReturnType();
    void nonVirtualMethod();
    void privateMethod();
    void signal();

private:
    inline Veritas::ClassSkeleton createStubClassFrom(const QByteArray&);
    void kompareMethods(const MethodSkeleton& expected, const MethodSkeleton& actual);
 
private:
    DeclarationFactory* m_factory;
    Veritas::StubConstructor* m_constructor;
};

}}

#endif // QTEST_STUBCREATIONTEST_H_INCLUDED
