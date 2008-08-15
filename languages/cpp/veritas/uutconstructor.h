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

#ifndef VERITASCPP_UUTCONSTRUCTOR_H
#define VERITASCPP_UUTCONSTRUCTOR_H

#include "veritascppexport.h"
#include "classskeleton.h"
#include "methodskeleton.h"
#include <language/editor/simplecursor.h>

namespace KDevelop { class Declaration; class Use; class DUContext; class SimpleCursor; }

namespace Veritas
{

class DocumentAccess;
/*!
Generate a skeleton for unit under test from its implementation 
in a test. Fetches all method uses and construct method skeletons
for those.

input:
@code
void MyTest::test() {
    MyUnitUnderTest uut;
    uut.rockOn();
}
@endcode
=>
output:
@code
class MyUnitUnderTest {
public:
    void rockOn();
};
@endcode
@unittest Veritas::Test::UUTCreationTest
*/
class VERITASCPP_EXPORT UUTConstructor
{
public:
    /*! Generate a class for an unresolved variable and all it's Use's */
    virtual ClassSkeleton morph(KDevelop::Declaration* variable);

    UUTConstructor();
    virtual ~UUTConstructor();
    virtual void setDocumentAccess(DocumentAccess*);

private:
    /*! Create a method skeleton from a @param use of @param clazz
    eg: Foo f; f.bar(); => method Foo::bar() */
    MethodSkeleton createMethod(KDevelop::Declaration* clazz, const KDevelop::Use* use, KDevelop::DUContext*);
    QString enoughText(const KUrl& url, const KDevelop::SimpleCursor&) const;
    void printUseInfo(int useId, const KDevelop::Use* use, KDevelop::DUContext* ctx);
    void constructMethodsFor(KDevelop::DUContext* ctx, KDevelop::Declaration* variable, ClassSkeleton& cs);

private:
    DocumentAccess* m_docAccess;
};

}

#endif // VERITASCPP_UUTCONSTRUCTOR_H
