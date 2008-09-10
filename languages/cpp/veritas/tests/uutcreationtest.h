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

#ifndef QTEST_UUTCREATIONTEST_H_INCLUDED
#define QTEST_UUTCREATIONTEST_H_INCLUDED

#include <QtCore/QObject>

namespace Veritas
{
class UUTConstructor;
class MethodSkeleton;
class ClassSkeleton;

namespace Test
{

class DocumentAccessStub;
class DeclarationFactory;

/*! @unitundertest Veritas::UUTConstructor */
class UUTCreationTest : public QObject
{
Q_OBJECT
private slots:
    void init();
    void cleanup();

    void unresolvedVariable();
    void resolvedVariables();
    void singleUse();
    void tdd_nonVoidReturn();
    void singleArgument();
    void tdd_pointerUse();
    void multipleUses();
    void useInIfConstruct();
    void spacesBeforeDot();
    void spacesAfterDot();
    void methodOnNextLine();
    void dataMemberUse();
    void multipleParameters();
    void classParameter();
    void threeParameters();
    void functionUseWithIdenticalName();
    void multipleParametersSmearedOverMultipleLines();
    void nestedBraces();
    void noSemicolon();

private:
    void assertReturnsVoid(const MethodSkeleton& ms);
    void assertNoArguments(const MethodSkeleton& ms);
    void assertNamed(const QString& name, const MethodSkeleton&);
    void assertNamed(const QString& name, const ClassSkeleton&);
    void assertDefaultBody(const MethodSkeleton&);
    ClassSkeleton classFromImplementation(const QByteArray& text);
    void assertSimpleFooMethod(const ClassSkeleton& cs);

private:
    DeclarationFactory* m_factory;
    DocumentAccessStub* m_document;
    UUTConstructor* m_constructor;
};

}}


#endif // QTEST_UUTCREATIONTEST_H_INCLUDED
