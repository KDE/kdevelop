/* This file is part of KDevelop
   Copyright 2012 Olivier de Gaalon <olivier.jg@gmail.com>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

// "internalContext" : { "childCount" : 3 }
template<class T>
struct TestEverything
{
public:
    /* "type" : { "toString" : "T" },
     * "useCount" : 0,
     * "instantiations" : {
     *     "<int>::<>" : {
     *         "type" : { "toString" : "int" },
     *         "instantiatedFrom" : { "qualifiedIdentifier" : "TestEverything::member" },
     *         "useCount" : 1
     *     }
     * }
     */
    T member;
    /* "type" : { "toString" : "function T ()" },
     * "returnType" : { "toString" : "T" },
     * "useCount" : 0,
     * "instantiations" : {
     *     "<int>::<>" : {
     *         "returnType" : { "toString" : "int" },
     *         "instantiatedFrom" : { "qualifiedIdentifier" : "TestEverything::memberFunc" },
     *         "useCount" : 1
     *     }
     * }
     */
    T memberFunc();
    /* "type" : { "toString" : "function T (X)" },
     * "returnType" : { "toString" : "T" },
     * "useCount" : 0,
     * "instantiations" : {
     *     "<int>::<>" : {
     *         "returnType" : { "toString" : "int" },
     *         "instantiatedFrom" : { "qualifiedIdentifier" : "TestEverything::templateMemberFunc" },
     *         "useCount" : 0
     *     },
     *     "<int>::<TestEverything< int >>" : {} #This instantiation/specialization is tested below
     * }
     */
    template<class X> T templateMemberFunc(X foo);
};

/* "type" : { "toString"  : "function int (TestEverything< int >)" },
 * "returnType" : { "toString"  : "int" },
 * "internalFunctionContext" : { "findDeclarations" : { "foo" : { "type" : { "toString" : "TestEverything< int >" } } } },
 * "specializedFrom" : { "qualifiedIdentifier" : "TestEverything::templateMemberFunc" },
 * "instantiatedFrom" : { "qualifiedIdentifier" : "TestEverything::templateMemberFunc" },
 * "qualifiedIdentifier" : "TestEverything< int >::templateMemberFunc< TestEverything< int > >",
 * "useCount" : 1
 */
template<>
template<>
int TestEverything<int>::templateMemberFunc< TestEverything<int> >(TestEverything<int> foo) { }

/* "type" : { "toString" : "constint", "isConst" : false },
 * "unaliasedType" : { "toString" : "const int", "isConst" : true },
 * "realType" : { "toString" : "const int", "isConst" : true },
 * "targetType" : { "toString" : "const int", "isConst" : true },
 * "shortenedType" : { "toString" : "constint", "isConst" : false }
 */
typedef const int constint;
/* "type" : { "toString" : "intptr", "isConst" : false },
 * "unaliasedType" : { "toString" : "int*", "isConst" : false },
 * "realType" : { "toString" : "int*", "isConst" : false },
 * "targetType" : { "toString" : "int", "isConst" : false },
 * "shortenedType" : { "toString" : "intptr", "isConst" : false }
 */
typedef int* intptr;
/* "type" : { "toString" : "constintptr", "isConst" : false },
 * # FIXME: unaliasedType can't see through other types (ie, PointerType), which leads to expected results
 * "unaliasedType" : {
 *      "EXPECT_FAIL" : { "toString" : "unaliasedType shows up as 'constint*', but constint is an alias" },
 *      "toString" : "const int*", "isConst" : false
 * },
 * "realType" : { "toString" : "constint*", "isConst" : false },
 * "targetType" : { "toString" : "const int", "isConst" : true },
 * "shortenedType" : { "toString" : "constintptr", "isConst" : false }
 */
typedef constint* constintptr;

int main()
{
    //"type" : { "toString" : "TestEverything< int >", "isConst" : false },
    //"identifiedTypeQid" : "TestEverything< int >"
    TestEverything<int> mytest;
    mytest.member;
    mytest.memberFunc();
    mytest.templateMemberFunc(mytest);
}
