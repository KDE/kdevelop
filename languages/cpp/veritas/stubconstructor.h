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

#ifndef VERITAS_STUBCONSTRUCTOR_INCLUDED_H
#define VERITAS_STUBCONSTRUCTOR_INCLUDED_H

#include "veritascppexport.h"
#include "classskeleton.h"

namespace KDevelop { class IDocument; class SimpleCursor; }
namespace Cpp { class ClassDeclaration; }

namespace Veritas
{
class DUCreationHelper;

/*!
Constructs a test fake sub class implementation for a given class.

input:
@code
class Foo {
public:
    virtual int foo();
};
@endcode

=> output:
@code
class FooStub {
public:
    int m_foo;
    virtual int foo() { return m_foo; };
};
@endcode */
class VERITASCPP_EXPORT StubConstructor
{
public:
    StubConstructor();
    virtual ~StubConstructor();
    ClassSkeleton morph(Cpp::ClassDeclaration*);
};

}

#endif // VERITAS_STUBCONSTRUCTOR_INCLUDED_H
