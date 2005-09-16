#include "QtTest/QtTest"
#include "parser.h"
#include "control.h"
#include "dumptree.h"
#include "tokens.h"


#include <QByteArray>
#include <QDataStream>
#include <QFile>

#include <iostream>

bool hasKind(AST*, AST::NODE_KIND);
AST* getAST(AST*, AST::NODE_KIND, int num = 0);

class TestParser : public QObject
{
  Q_OBJECT

  Control control;
  DumpTree dumper;

public:
  TestParser()
  {
  }

private slots:

  void initTestCase()
  {
  }

  void testParser()
  {
    QByteArray clazz("struct A { int i; A() : i(5) { } virtual void test() = 0; };");
    pool mem_pool;
    TranslationUnitAST* ast = parse(clazz, &mem_pool);
    VERIFY(ast != 0);
    VERIFY(ast->declarations != 0);
  }

  void testParseMethod()
  {
    QByteArray method("void A::test() {  }");
    VERIFY(!control.skipFunctionBody());
    pool mem_pool;
    TranslationUnitAST* ast = parse(method, &mem_pool);
    VERIFY(ast != 0);
    VERIFY(hasKind(ast, AST::Kind_FunctionDefinition));
  }

  void testMethodArgs()
  {
    QByteArray method("int A::test(int primitive, B* pointer) { return primitive; }");
    pool mem_pool;
    Parser parser(&control);
    TranslationUnitAST* ast = parser.parse(method.constData(), 
					   method.size() + 1, &mem_pool);
    // return type
    SimpleTypeSpecifierAST* retType = static_cast<SimpleTypeSpecifierAST*>
      (getAST(ast, AST::Kind_SimpleTypeSpecifier));
    COMPARE((TOKEN_KIND)parser.token_stream.kind(retType->start_token),
	    Token_int);

    // first param
    ParameterDeclarationAST* param = static_cast<ParameterDeclarationAST*>
      (getAST(ast, AST::Kind_ParameterDeclaration));
    SimpleTypeSpecifierAST* paramType = static_cast<SimpleTypeSpecifierAST*>
      (getAST(param, AST::Kind_SimpleTypeSpecifier));
    COMPARE((TOKEN_KIND)parser.token_stream.kind(paramType->start_token),
	    Token_int);
    UnqualifiedNameAST* argName  = static_cast<UnqualifiedNameAST*>
      (getAST(param, AST::Kind_UnqualifiedName));
    COMPARE(parser.token_stream.symbol(argName->id)->as_string(), 
	    QString("primitive"));
    
    // second param
    param = static_cast<ParameterDeclarationAST*>
      (getAST(ast, AST::Kind_ParameterDeclaration, 1));
    UnqualifiedNameAST* argType = static_cast<UnqualifiedNameAST*>
      (getAST(param, AST::Kind_UnqualifiedName));
    COMPARE(parser.token_stream.symbol(argType->id)->as_string(), 
	    QString("B"));

    // pointer operator
    VERIFY(hasKind(param, AST::Kind_PtrOperator));

    argName = static_cast<UnqualifiedNameAST*>
      (getAST(param, AST::Kind_UnqualifiedName, 1));
    COMPARE(parser.token_stream.symbol(argName->id)->as_string(), 
	    QString("pointer"));
    
  }

  void testForStatements()
  {
    QByteArray method("void A::t() { for (int i = 0; i < 10; i++) { ; }}");
    pool mem_pool;
    TranslationUnitAST* ast = parse(method, &mem_pool);

    VERIFY(ast != 0);
    VERIFY(hasKind(ast, AST::Kind_ForStatement));
    VERIFY(hasKind(ast, AST::Kind_Condition));
    VERIFY(hasKind(ast, AST::Kind_IncrDecrExpression));
    VERIFY(hasKind(ast, AST::Kind_SimpleDeclaration));

    QByteArray emptyFor("void A::t() { for (;;) { } }");
    ast = parse(emptyFor, &mem_pool);
    VERIFY(ast != 0);
    VERIFY(hasKind(ast, AST::Kind_ForStatement));
    VERIFY(!hasKind(ast, AST::Kind_Condition));
    VERIFY(!hasKind(ast, AST::Kind_SimpleDeclaration));
  }
  
  void testIfStatements()
  {
    QByteArray method("void A::t() { if (1 < 2) { } }");
    pool mem_pool;
    TranslationUnitAST* ast = parse(method, &mem_pool);
    VERIFY(hasKind(ast, AST::Kind_Condition));
    VERIFY(hasKind(ast, AST::Kind_BinaryExpression));
  }

  void testParseFile()
  {
     QFile file("test_parser.cpp");
     VERIFY(file.open(QFile::ReadOnly));
     QByteArray contents = file.readAll();
     file.close();
     pool mem_pool;
     Parser parser(&control);     
     TranslationUnitAST* ast = parser.parse(contents.constData(), 
					    contents.size(), &mem_pool);
     VERIFY(ast != 0);
     VERIFY(ast->declarations != 0);
   }

private:

  TranslationUnitAST* parse(const QByteArray& unit, pool* mem_pool)
  {
    Parser parser(&control);
    return  parser.parse(unit.constData(), unit.size() + 1, mem_pool);
  }

  

};

struct HasKindVisitor : protected DefaultVisitor
{

  AST::NODE_KIND kind;
  AST* ast;
  int num;

  HasKindVisitor(AST::NODE_KIND kind, int num = 0)
    : kind(kind), ast(0), num(num)
  {
  }

  bool hasKind() const
  {
    return ast != 0;
  }

  void visit(AST* node)
  {
    if (!ast && node) {
      if (node->kind == kind && --num < 0) {
	ast = node;
      }
      else {
	DefaultVisitor::visit(node);
      }
    }
  }
};

bool hasKind(AST* ast, AST::NODE_KIND kind)
{
  HasKindVisitor visitor(kind);
  visitor.visit(ast);
  return visitor.hasKind();
}

AST* getAST(AST* ast, AST::NODE_KIND kind, int num)
{
  HasKindVisitor visitor(kind, num);
  visitor.visit(ast);
  return visitor.ast;
}

QTTEST_MAIN(TestParser)
#include "test_parser.moc"
