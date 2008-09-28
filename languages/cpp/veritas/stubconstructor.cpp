/*
 * KDevelop xUnit test support
 *
 * Copyright 2008 Manuel Breugelmans <mbr.nxi@gmail.com>
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

#include "stubconstructor.h"

// kdevplatform
#include <interfaces/idocument.h>
#include <language/duchain/duchainutils.h>
#include <language/duchain/duchain.h>
#include <language/duchain/duchainlock.h>
#include <language/duchain/ducontext.h>
#include <language/duchain/classfunctiondeclaration.h>

// kdevelop
#include "languages/cpp/cppduchain/classdeclaration.h"

using Veritas::ClassSkeleton;
using Veritas::MethodSkeleton;
using Veritas::ConstructorSkeleton;
using Veritas::StubConstructor;
using namespace KDevelop;
using namespace Cpp;

namespace
{

/*! Returns true if @param decl is a member function with the following properties:
      - no constructor nor destructor
      - virtual
      - not private
      - not a Qt meta function */
QStringList blackList(QString("metaObject,qt_metacast,qt_metacall").split(','));
ClassFunctionDeclaration* filterMemberFunction(Declaration* decl)
{
    ClassFunctionDeclaration* fdcl = dynamic_cast<ClassFunctionDeclaration*>(decl);
    if (fdcl) {
        if (fdcl->isDestructor() || fdcl->isConstructor() || 
           !fdcl->isVirtual()    || fdcl->accessPolicy() == Declaration::Private ||
            blackList.contains(fdcl->identifier().toString())) {
            return 0;
        }
    }
    return fdcl;
}

MethodSkeleton fillMethod(ClassFunctionDeclaration* function)
{
    MethodSkeleton ms;
    FunctionType::Ptr type = function->type<FunctionType>();
    if (!type) { kDebug() << "Erm type == 0"; return ms; }
    ms.setName(function->identifier().toString());
    ms.setFunctionType(type);
    return ms;
}

QString stubNameFor(ClassDeclaration* clazz)
{
    Identifier clzId = clazz->identifier();
    return clzId.toString() + "Stub";
}

/*! add a data member to the class and return this in method ms.
    eg: class FooStub { int m_foo; int foo() { return m_foo; } }; */
void insertFakeMember(const QString& member, MethodSkeleton& ms, ClassSkeleton& cs, ConstructorSkeleton& constr)
{
    ms.setBody("return " + member + ';');
    cs.addMember(ms.returnType() + ' ' + member);
    constr.addInitializer(member + "(0)");
}

} // end anonymous namespace

StubConstructor::StubConstructor()
{}

StubConstructor::~StubConstructor()
{}

ClassSkeleton StubConstructor::morph(ClassDeclaration* clazz)
{
    Q_ASSERT(clazz);
    DUChainReadLocker lock(DUChain::lock());

    QString stubName = stubNameFor(clazz);
    ClassSkeleton cs; cs.setName(stubName);
    cs.addSuper(clazz->identifier().toString());
    ConstructorSkeleton constr; constr.setName(stubName);
    MethodSkeleton destr; destr.setName('~' + stubName);

    ClassFunctionDeclaration* function;
    foreach(Declaration* dcl, clazz->internalContext()->localDeclarations()) {
        function = filterMemberFunction(dcl);
        if (function) { // construct and add method to the stub
            MethodSkeleton ms = fillMethod(function);
            if (ms.returnType() != "void") {
                QString member = "m_" + ms.name();
                insertFakeMember(member, ms, cs, constr);
            }
            cs.addMethod(ms);
        }
    }

    cs.setConstructor(constr);
    cs.setDestructor(destr);
    return cs;
}

