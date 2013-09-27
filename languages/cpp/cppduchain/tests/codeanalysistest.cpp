/* This file is part of KDevelop
    Copyright 2010 Aleix Pol Gonzalez <aleixpol@kde.org>

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

#include "codeanalysistest.h"
#include <QTest>
#include <language/duchain/duchain.h>
#include <language/checks/controlflowgraph.h>
#include <language/checks/controlflownode.h>
#include <language/duchain/declaration.h>

QTEST_MAIN(CodeAnalysisTest)

using namespace KDevelop;
using namespace Cpp;

CodeAnalysisTest::CodeAnalysisTest(QObject* parent): QObject(parent)
{
  initShell();
}

void CodeAnalysisTest::initTestCase()
{}

void CodeAnalysisTest::cleanupTestCase()
{}

void CodeAnalysisTest::testUseReadWrite()
{
  QFETCH(QString, code);
  QFETCH(QVariantList, modFlags);
  
  LockedTopDUContext top = parse(code.toUtf8(), DumpNone, 0, true);
  
  DataAccessRepository* repo = &m_modifications;
  
//   foreach(DataAccess* f, repo->modifications()) {
//     qDebug() << "lalala" << f->flags();
//   }
  
  QCOMPARE(repo->modifications().count(), modFlags.size());
  
  int i=0;
  foreach(const QVariant& f, modFlags) {
//     qDebug() << "flags" << repo->modifications().at(i)->flags() << f;
    KDevelop::DataAccess::DataAccessFlags p(f.toUInt());
    DataAccess* da=repo->modifications().at(i);
    QCOMPARE(da->isRead(), bool(p&DataAccess::Read));
    QCOMPARE(da->isWrite(), bool(p&DataAccess::Write));
    
    DUContext* ctx=top->findContextAt(da->pos());
    int u=ctx->findUseAt(da->pos());
    Declaration* d=ctx->findDeclarationAt(da->pos());
    QVERIFY(u>=0 || d);
    i++;
  }
}
void CodeAnalysisTest::testUseReadWrite_data()
{
  QTest::addColumn<QString>("code");
  QTest::addColumn<QVariantList>("modFlags");
  
  QTest::newRow("def") << "int a; void main() { a=3; }" << (QVariantList() << uint(DataAccess::Write));
  QTest::newRow("int==") << "int a; int main() { a==3; }" << (QVariantList() << uint(DataAccess::Read));
  
  QTest::newRow("callr") << "int f(int e, int a) { f(a,e); }" << (QVariantList() << uint(DataAccess::Read) << uint(DataAccess::Read) << uint(DataAccess::Read));
  QTest::newRow("callrw") << "int p,w; int f(int&, int) { f(p,w); }" << (QVariantList() << uint(DataAccess::Write|DataAccess::Read) << uint(DataAccess::Read) << uint(DataAccess::Read));
  QTest::newRow("initandcopy") << "void f() { int a=1, b; a=b; }"
                                  << (QVariantList() << uint(DataAccess::Write) << uint(DataAccess::Write) << uint(DataAccess::Read));
  QTest::newRow("oper==") << "class C { bool operator==(const C&) const; }; bool f(const C& e) { return e==e; }"
                                  << (QVariantList() << uint(DataAccess::Read) << uint(DataAccess::Read));
  QTest::newRow("++oper") << "struct C { bool operator++(); }; C e; bool f() { ++e; }"
                                  << (QVariantList() << uint(DataAccess::Write|DataAccess::Read));
  QTest::newRow("oper++") << "struct C { bool operator++(int); }; C e; bool f() { e++; }"
                                  << (QVariantList() << uint(DataAccess::Write|DataAccess::Read));
  QTest::newRow("int++") << "void f() { int e; e++; }"
                                  << (QVariantList() << uint(DataAccess::Write|DataAccess::Read));
  QTest::newRow("member-dot") << "struct C { C boh(); }; C e; bool f() { e.boh().boh().boh(); }"
                                  << (QVariantList() << uint(DataAccess::Write|DataAccess::Read) << uint(DataAccess::Call|DataAccess::Read|DataAccess::Write) << uint(DataAccess::Call|DataAccess::Read|DataAccess::Write) << uint(DataAccess::Call|DataAccess::Read|DataAccess::Write));
  QTest::newRow("member-arrow") << "struct C { C boh(); }; C* e; bool f() { e->boh(); }"
                                  << (QVariantList() << uint(DataAccess::Read) << uint(DataAccess::Call|DataAccess::Read));
  QTest::newRow("forloop") << "int i; bool f() { for(i=0; i<33; ++i) {} }"
                                  << (QVariantList() << uint(DataAccess::Write) << uint(DataAccess::Read) << uint(DataAccess::Read|DataAccess::Write));
  QTest::newRow("if-nocond") << "int i; bool f() { if(i) {} }"
                                  << (QVariantList() << uint(DataAccess::Read));
  QTest::newRow("if-unary") << "int i; bool f() { if(!i) {} }"
                                  << (QVariantList() << uint(DataAccess::Read));
  QTest::newRow("ternary-op") << "int i; bool f() { return i ? i : i; }" //TODO: do the same when ref parameter
                                  << (QVariantList() << uint(DataAccess::Read) << uint(DataAccess::Read) << uint(DataAccess::Read));
  QTest::newRow("initializer") << "struct C {int i,j; C() : i(0),j(i) {} };"
                                  << (QVariantList() << uint(DataAccess::Write) << uint(DataAccess::Write) << uint(DataAccess::Read));
  QTest::newRow("new-delete") << "int *a,b; void f() { a = new int(b); delete a; }"
                                  << (QVariantList() << uint(DataAccess::Write) << uint(DataAccess::Read) << uint(DataAccess::Read));
  QTest::newRow("local new-delete") << "int b; void f() { int *a = new int(b); delete a; }"
                                  << (QVariantList() << uint(DataAccess::Write) << uint(DataAccess::Read) << uint(DataAccess::Read));
  QTest::newRow("new-define") << "void f() { int b=2, *a = new int(b); }"
                                  << (QVariantList() << uint(DataAccess::Write) << uint(DataAccess::Write) << uint(DataAccess::Read));
  QTest::newRow("return") << "int a; int f() { return a; }"
                                  << (QVariantList() << uint(DataAccess::Read));
  QTest::newRow("init") << "int f() { int a=3; }"
                                  << (QVariantList() << uint(DataAccess::Write));
  QTest::newRow("init2") << "int f() { int a=3; int b=a; }"
                                  << (QVariantList() << uint(DataAccess::Write) << uint(DataAccess::Write) << uint(DataAccess::Read));
  QTest::newRow("switch") << "int f(int a) { switch(a) { case 3: break;} }"
                                  << (QVariantList() << uint(DataAccess::Read));
  QTest::newRow("constructor") << "class C { C(int,int&); };  void f(int a) { int b; C* c=new C(a,b); }"
                                  << (QVariantList() << uint(DataAccess::Write) << uint(DataAccess::Read) << uint(DataAccess::Read|DataAccess::Write));
  QTest::newRow("empty constructor") << "class C { C(); };  void f() { new C; }"
                                  << QVariantList();
  QTest::newRow("function call, different parameter count") << "void f(int) { f(3,4); }"
                                  << (QVariantList() << uint(DataAccess::Read));
  QTest::newRow("method call, different parameter count") << "class C { void f(int,int&); };  void f(int x) { C c; c.f(1,x); }"
                                  << (QVariantList() << uint(DataAccess::Read|DataAccess::Write) << uint(DataAccess::Read|DataAccess::Write) << uint(DataAccess::Call|DataAccess::Read|DataAccess::Write));
  QTest::newRow("casts") << "class C { C m(); }; class D : public C {}; void f(D* a) { static_cast<C*>(a)->m(); }"
                                  << (QVariantList() << uint(DataAccess::Read) << uint(DataAccess::Read) << uint(DataAccess::Call|DataAccess::Read));
  QTest::newRow("ptr") << "void f() { int* a=new int; (*a)=3; }"
                                  << (QVariantList() << uint(DataAccess::Write) << uint(DataAccess::Read));
}

static void walkNodesRecursively(ControlFlowNode* node, QSet<ControlFlowNode*>& visited)
{
  if(!visited.contains(node)) {
    visited.insert(node);
    
    if(node->next())
      walkNodesRecursively(node->next(), visited);
    if(node->alternative())
      walkNodesRecursively(node->alternative(), visited);
  }
}

class ControlFlowToDot
{
  public:
    ControlFlowToDot(QTextStream* dev, const QByteArray& sources) : m_dev(dev), m_sources(sources)
    {}
    
    static QString escapeQuotes(const QString& _s) { QString s(_s); s.replace('\"', "\\\""); return s;}
    
    bool exportGraph(const QByteArray&, const ControlFlowGraph* graph)
    {
      bool r = true;
      int i=0;
      *m_dev << "digraph G {\n";
      *m_dev << "  label = \""+escapeQuotes(m_sources)+"\";\n";
      QList<ControlFlowNode*> n=graph->rootNodes();
      for(QList<ControlFlowNode*>::const_iterator it=n.constBegin(), itEnd=n.constEnd(); it!=itEnd; ++it, ++i) {
        *m_dev << "  subgraph cluster_" << i << "  {\n\tcolor=black;\n";
        Declaration* d=declarationForNode(graph, *it);
        if(d)
          *m_dev << "\tlabel=\""+d->toString()+"\"\n";
        r &= exportNode(*it);
        *m_dev << "  }\n";
      }
      
      QVector< ControlFlowNode* > deadNodes = graph->deadNodes();
      if(!deadNodes.isEmpty()) {
        *m_dev << "  subgraph cluster_"<< i <<"  {\n";
        *m_dev << "\tlabel = \"Dead Nodes\";\n";
        foreach(ControlFlowNode* node, deadNodes) {
          *m_dev << '\t' << nodesName(node) << " [color=green]\n\n";
        }
        *m_dev << "  }\n";
      }
      
      *m_dev << "}\n";
      
      return r;
    }
    
    Declaration* declarationForNode(const ControlFlowGraph* graph, const ControlFlowNode* node) const {
      foreach(Declaration* d, graph->declarations()) {
        if(graph->nodeForDeclaration(d)==node)
          return d;
      }
      return 0;
    }
    
private:
    bool exportNode(const ControlFlowNode* node)
    {
      if(!node)
        return false;
      
      QHash< const ControlFlowNode*, QString >::iterator initialIt = m_names.find(node);
      bool isfirst = m_names.isEmpty();
      
      if(initialIt==m_names.end()) {
        initialIt = m_names.insert(node, nodesName(node));
      } else
        return true;
      
      QString name = initialIt.value();
      if(isfirst)
        *m_dev << '\t' << name << " [shape=doubleoctagon]\n\n";
      if(!node->next() && !node->alternative())
        *m_dev << '\t' << name << " [color=red]\n\n";
      
      if(exportNode(node->next()))        *m_dev << '\t' << name << " -> " << m_names.value(node->next()) << " [color=blue];\n";
      if(exportNode(node->alternative())) *m_dev << '\t' << name << " -> " << m_names.value(node->alternative()) << " [color=red];\n";
      
      return true;
    }
    
    QString nodesName(const ControlFlowNode* node) const
    {
      RangeInRevision range = node->nodeRange();
      if(!node->next() && !node->alternative())
        return "Exit";
      else if(range.isEmpty()) {
        static int uniqueId=0;
        return QString("dummy_%1").arg(uniqueId++);
      } else {
        int a=cursorToPos(range.start), b=cursorToPos(range.end);
        QString ret=m_sources.mid(a, b-a);
        if(node->type()==ControlFlowNode::Conditional) {
          RangeInRevision crange = node->conditionRange();
          int ca=cursorToPos(crange.start), cb=cursorToPos(crange.end);
          ret += " - condition: "+m_sources.mid(ca, cb-ca);
        }
        
        return "\""+ret+"\"";
      }
    }
    
    int cursorToPos(const CursorInRevision& cursor) const
    {
      int ret=0;
      int line=cursor.line, col=cursor.column;
      for(; line>0 && ret>0; line--)
        ret = m_sources.indexOf('\n', ret);
      
      if(ret<0)
        return -1;
      
      return ret + col;
    }
    
    QTextStream* m_dev;
    QHash<const ControlFlowNode*, QString> m_names;
    QByteArray m_sources;
};

void CodeAnalysisTest::testControlFlowCreation()
{
  QFETCH(QString, code);
  QFETCH(int, nodeCount);
  
  LockedTopDUContext top = parse(code.toUtf8(), DumpNone);
  
  ControlFlowGraph* graph = &m_ctlflowGraph;
  
  QList<ControlFlowNode*> n=graph->rootNodes();
  QList<ControlFlowNode*>::const_iterator it=n.constBegin(), itEnd=n.constEnd();
  QSet<ControlFlowNode*> visited;
  int entries=0;
  for(; it!=itEnd; ++it, ++entries)
    walkNodesRecursively(*it, visited);
  
  {//Graph exporting
    QFile file(QString(QTest::currentDataTag()).replace(' ', '_')+".dot");
    QVERIFY(file.open(QFile::WriteOnly));
    QTextStream st(&file);
    ControlFlowToDot exporter(&st, code.toUtf8());
    exporter.exportGraph(QTest::currentDataTag(), graph);
  }
  
  QCOMPARE(visited.size(), nodeCount);
  
  foreach(ControlFlowNode* n, visited) {
    RangeInRevision crange=n->conditionRange();
    QVERIFY(!crange.isValid() || crange.end>=crange.start);
  }
}

void CodeAnalysisTest::testControlFlowCreation_data()
{
  QTest::addColumn<QString>("code");
  QTest::addColumn<int>("nodeCount");
  
  QTest::newRow("empty") << "void f() {}" << 2;
  QTest::newRow("sequence") << "int f(int a) { return a+1; }" << 2;
  QTest::newRow("conditional") << "int f(int a) { if(a) a+=1; return a+3; }" << 4;
  QTest::newRow("conditional_else") << "int f(int a) { if(a) a+=1; else a-=1; return a+3; }" << 5;
  QTest::newRow("different exits") << "int f(int a) { if(a) return a; else return b; }" << 4;
  QTest::newRow("conditional_inlined") << "int f(int a) { a=a?a+1 : a-1; return a+3; }" << 5;
  QTest::newRow("while") << "int f(int q) { while(q) {q--} return q; }" << 5;
  QTest::newRow("for") << "int f(int a) { for(int i=0; i<a; i++) {i+=54;} return 0; }" << 6;
  QTest::newRow("forinf") << "void f() { for(;;) {} }" << 6;
  QTest::newRow("switch") << "void f(int a) { switch(a) { case 1: f(1); break; case 2: f(2); return; default: f(3); break; } f(4); }" << 9;
  QTest::newRow("switch2") << "void f(int a) { switch(a) { case 1: f(-1); case 2: f(1); break; case 3: f(2); break; } f(666); }" << 9;
  QTest::newRow("switch0") << "void f(int a) { switch(a) {} f(666); }" << 3;
  QTest::newRow("do-while") << "void f(int a) { do {a--; } while(a); }" << 5;
  
  QTest::newRow("loopbreak") << "void f(int i) { while(i) { if(i>20) break; } f(666); }" << 7;
  QTest::newRow("loopconti") << "void f(int i) { while(i) { if(i>20) continue; } f(666); }" << 7;
  
  QTest::newRow("goto") << "void f(int i) { f(0); tag: f(1); if(i) goto tag; f(2); f(1); }" << 5;
  QTest::newRow("goto2") << "void f(int i) { f(0); goto tag; f(1); if(i) f(3); tag: f(2); }" << 3;
  
  QTest::newRow("outside") << "enum {Result = 2 ? 1 : 3 };" << 4;
  QTest::newRow("class") << "struct Peu { public: Peu() { int x = 3?4:2; } int thing(int x) { return x ? x+1 : x-1; } };" << 10;
  QTest::newRow("lecture") << "int f(int x) { int i; if(x==0) i=3; else i=4; return i; }" << 5;
}

