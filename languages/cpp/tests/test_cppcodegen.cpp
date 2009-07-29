/*
   This file is part of KDevelop
   Copyright 2009 Ramón Zarazúa <killerfox512+kde@gmail.com>
   
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

#include "test_cppcodegen.h"

#include "parsesession.h"
#include "ast.h"
#include "astutilities.h"
#include "codegen/cppnewclass.h"

#include <language/backgroundparser/backgroundparser.h>
#include <language/codegen/coderepresentation.h>
#include <language/codegen/documentchangeset.h>
#include <language/duchain/duchain.h>  
#include <language/duchain/duchainlock.h>

#include <interfaces/ilanguagecontroller.h>

#include <QtTest/QTest>
#include <shell/shellextension.h>
#include <tests/autotestshell.h>
#include <tests/testcore.h>
#include <qtest_kde.h>

///gets the top context of artificial test code by just specifying the file name, more efficient and safe than calling contextForUrl
//Needs to be a macro because QVERIFY2 only works directly on the test function
#define GET_CONTEXT(file, top)  IndexedString str_(CodeRepresentation::artificialUrl(file));\
                                QVERIFY2(m_contexts[str_], QString("Requested artificial file not found: %1").arg(str_.str()).toUtf8());\
                                top = m_contexts[str_].data()

QTEST_KDEMAIN(TestCppCodegen, GUI )

using namespace KDevelop;

void TestCppCodegen::initTestCase()
{
  //TODO Have some problem with CppLanguageSuppor having an invalid component return from its factory
  //Initialize KDevelop components
  AutoTestShell::init();
  Core::initialize(KDevelop::Core::NoUi);
  
  //Insert all the test data as a string representation
  //NOTE: When adding new test artificial code, remember to update cppcodegen_snippets.cpp, and if possible update related tests
  //NOTE: To #include another artificial Code Representation, it must be as an absolute path
  addArtificialCode(IndexedString(CodeRepresentation::artificialUrl("ClassA.h")), "class ClassA { public: ClassA(); private: int i;  float f, j;\
                                                       struct ContainedStruct { int i; ClassA * p;  } structVar; };");
  addArtificialCode(IndexedString(CodeRepresentation::artificialUrl("ClassA.cpp")), "#include</ClassA.h> \n ClassA::ClassA() : i(0), j(0.0) {structVar.i = 0; }");
  
  addArtificialCode(IndexedString(CodeRepresentation::artificialUrl("AbstractClass.h")), "class AbstractClass { public: virtual ~AbstractClass();\
                                                       virtual void pureVirtual() = 0; virtual const int constPure(const int &) const = 0; \
                                                       virtual void regularVirtual(); virtual const int constVirtual(const int &) const; int data; };");
  
  parseArtificialCode();
}


void TestCppCodegen::cleanupTestCase()
{
  Core::self()->cleanup();
  qDeleteAll(m_artificialCode);
}

void TestCppCodegen::init()
{
  resetArtificialCode();
  parseArtificialCode();
}

void TestCppCodegen::testAstDuChainMapping()
{
  
  DUChainReadLocker lock(DUChain::lock());
  
  ContextContainer::Iterator it = m_contexts.begin();
  
  //----ClassA.h----
  ParseSession::Ptr session = ParseSession::Ptr::dynamicCast<IAstContainer>(it->data()->ast());
  QVERIFY(session);
  TranslationUnitAST * ast = session->topAstNode();
  QVERIFY(ast);
  QVERIFY(ast->declarations);
  QCOMPARE(ast->declarations->count(), 1);
  
  QVERIFY(AstUtils::childNode<SimpleDeclarationAST>(ast, 0));
  QCOMPARE(AstUtils::childNode<SimpleDeclarationAST>(ast, 0)->type_specifier,
            session->astNodeFromDeclaration(it->data()->localDeclarations()[0]));
  //ClassA
  {
    ClassSpecifierAST * classAst = AstUtils::node_cast<ClassSpecifierAST>
                                  (AstUtils::childNode<SimpleDeclarationAST>(ast, 0)->type_specifier);
    QVERIFY(classAst);
    
    DUContext * cont = it->data()->localDeclarations()[0]->internalContext();
    QVERIFY(cont);
    
    QCOMPARE(classAst->member_specs->count(), 6);
    QCOMPARE(cont->localDeclarations().size(), 6);
    
    kDebug() << "Declaration mapped: " << session->declarationFromAstNode(AstUtils::childNode<SimpleDeclarationAST>(classAst, 3))->identifier();
    //kDebug() << "Actual Declaration: " << cont->localDeclarations()[6]->identifier();
    QCOMPARE(AstUtils::childNode<SimpleDeclarationAST>(classAst, 1),
             session->astNodeFromDeclaration(cont->localDeclarations()[0]));
             
/*    QCOMPARE(AstUtils::childNode<SimpleDeclarationAST>(classAst, 3)->type_specifier,
             session->astNodeFromDeclaration(cont->localDeclarations()[2]));*/
             
//    QCOMPARE(AstUtils::childNode<FunctionDefinitionAST>(classAst, 4),
//             session->astNodeFromDeclaration(cont->localDeclarations()[2]));

    
    QCOMPARE(AstUtils::childNode<SimpleDeclarationAST>(classAst, 5)->type_specifier,
             session->astNodeFromDeclaration(cont->localDeclarations()[4]));
  }
  ++it;
 
  
  //----ClassA.cpp----
  QVERIFY(session = ParseSession::Ptr::dynamicCast<IAstContainer>(it->data()->ast()));
  QVERIFY(ast = session->topAstNode());
  QVERIFY(ast->declarations);
  QCOMPARE(ast->declarations->count(), 1);
  
  QVERIFY(AstUtils::childNode<FunctionDefinitionAST>(ast, 0));
  QCOMPARE(AstUtils::childNode<FunctionDefinitionAST>(ast, 0),
            session->astNodeFromDeclaration(KDevelop::DeclarationPointer(it->data()->localDeclarations()[0])));
  QCOMPARE(it->data()->localDeclarations()[0]->context()->importedParentContexts().size(), 1);
  QVERIFY(it->data()->localDeclarations()[0]->context()->importedParentContexts()[0].context(it->data()) != it->data());
}

#include <codegen/simplerefactoring.h>
void TestCppCodegen::testClassGeneration()
{
  TopDUContext * top = 0;
  GET_CONTEXT("AbstractClass.h", top);
  
  //DUChain::self()->
  
  CppNewClass newClass;
  newClass.identifier("GeneratedClass");
  newClass.addBaseClass("AbstractClass");
  newClass.setHeaderUrl(CodeRepresentation::artificialUrl("GeneratedClass.h"));
  newClass.setImplementationUrl(CodeRepresentation::artificialUrl("GeneratedClass.cpp"));
  
  DocumentChangeSet changes = newClass.generate();
  changes.applyAllToTemp();
  QCOMPARE(changes.tempNamesForAll().size(), 2);
/*  parseFile(changes.tempNamesForAll()[0].second);
  parseFile(changes.tempNamesForAll()[1].second);*/
}

void TestCppCodegen::testPrivateImplementation()
{
}

void TestCppCodegen::parseArtificialCode()
{
  //Update the context for all the representations, and save their contexts in the contexts map
  foreach(IndexedString file, m_artificialCodeNames)
  {
    CodeRepresentation::Ptr code = createCodeRepresentation(file);
    
    parseFile(file);
  }
}

void TestCppCodegen::addArtificialCode ( IndexedString fileName, const QString & code )
{
  m_artificialCode << new InsertArtificialCodeRepresentation(fileName, code);
  m_artificialCodeNames << fileName;
}

void TestCppCodegen::resetArtificialCode(void)
{
}

void TestCppCodegen::parseFile(IndexedString file)
{
    Core::self()->languageController()->backgroundParser()->addDocument(file.toUrl(), static_cast<TopDUContext::Features>(TopDUContext::AllDeclarationsAndContexts | TopDUContext::AST));
    ///TODO maybe only needs to be done once
    Q_ASSERT(m_contexts[file] = DUChain::self()->waitForUpdate(file, static_cast<TopDUContext::Features>(TopDUContext::AllDeclarationsAndContexts | TopDUContext::AST)));
}

#include "test_cppcodegen.moc"
