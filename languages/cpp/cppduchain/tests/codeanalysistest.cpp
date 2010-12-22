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
#include <flowgraph.h>
#include <flownode.h>

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
  
  LockedTopDUContext top = parse(code.toUtf8(), DumpNone);
  
  DataAccessRepository* repo = &m_modifications;
  
  QCOMPARE(repo->count(), modFlags.size());
  
  int i=0;
  foreach(const QVariant& f, modFlags) {
    qDebug() << "flags" << repo->at(i).m_flags << f;
    QCOMPARE(repo->at(i).m_flags, f.toUInt());
    
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
  QTest::newRow("oper==") << "class C { bool operator==(const C&) const; }; bool f(const C& e) { return e==e; }"
                                  << (QVariantList() << uint(DataAccess::Read) << uint(DataAccess::Read));
  QTest::newRow("++oper") << "struct C { bool operator++(); }; C e; bool f() { ++e; }"
                                  << (QVariantList() << uint(DataAccess::Write|DataAccess::Read));
  QTest::newRow("oper++") << "struct C { bool operator++(int); }; C e; bool f() { e++; }"
                                  << (QVariantList() << uint(DataAccess::Write|DataAccess::Read));
  QTest::newRow("member-dot") << "struct C { C boh(); }; C e; bool f() { e.boh().boh().boh(); }"
                                  << (QVariantList() << uint(DataAccess::Write|DataAccess::Read));
  QTest::newRow("member-arrow") << "struct C { C boh(); }; C* e; bool f() { e->boh(); }"
                                  << (QVariantList() << uint(DataAccess::Read));
  QTest::newRow("forloop") << "int i; bool f() { for(i=0; i<33; ++i) {} }"
                                  << (QVariantList() << uint(DataAccess::Write) << uint(DataAccess::Read) << uint(DataAccess::Read|DataAccess::Write));
  QTest::newRow("if-nocond") << "int i; bool f() { if(i) {} }"
                                  << (QVariantList() << uint(DataAccess::Read));
  QTest::newRow("if-unary") << "int i; bool f() { if(!i) {} }"
                                  << (QVariantList() << uint(DataAccess::Read));
  QTest::newRow("ternary-op") << "int i; bool f() { i ? i : i; }"
                                  << (QVariantList() << uint(DataAccess::Read) << uint(DataAccess::Read) << uint(DataAccess::Read));
  QTest::newRow("initializer") << "struct C {int i,j; C() : i(0),j(i) {} };" //todo: the use on j(>i<) is repeated
                                  << (QVariantList() << uint(DataAccess::Write) << uint(DataAccess::Write) << uint(DataAccess::Read));
  QTest::newRow("new-delete") << "int *a,b; void f() { a = new int(b); delete a; }"
                                  << (QVariantList() << uint(DataAccess::Write) << uint(DataAccess::Read) << uint(DataAccess::Read));
  QTest::newRow("return") << "int a; int f() { return a; }"
                                  << (QVariantList() << uint(DataAccess::Read));
  QTest::newRow("init") << "int f() { int a=3; }"
                                  << (QVariantList() << uint(DataAccess::Write));
  QTest::newRow("switch") << "int f(int a) { switch(a) { case 3: break;} }"
                                  << (QVariantList() << uint(DataAccess::Read));
}

static void walkNodesRecursively(ControlFlowNode* node, QSet<ControlFlowNode*>& visited)
{
  if(!visited.contains(node)) {
    visited.insert(node);
    
    if(node->m_next)
      walkNodesRecursively(node->m_next, visited);
    if(node->m_alternative)
      walkNodesRecursively(node->m_alternative, visited);
  }
}

class ControlFlowToDot
{
  public:
    ControlFlowToDot(QTextStream* dev, const QByteArray& sources) : m_dev(dev), m_sources(sources)
    {}
    
    bool exportGraph(const QByteArray& name, const ControlFlowNode* initial)
    {
      *m_dev << "digraph " << name << "{\n";
      bool r = exportNode(initial);
      *m_dev << "}\n";
      return r;
    }
    
private:
    bool exportNode(const ControlFlowNode* node)
    {
      if(!node)
        return false;
      
      QHash< const ControlFlowNode*, QString >::iterator initialIt = m_names.find(node);
      
      if(initialIt==m_names.end()) {
        initialIt = m_names.insert(node, nodesName(node));
      } else
        return true;
      
      QString name = initialIt.value();
      
      if(exportNode(node->m_next))        *m_dev << '\t' << name << " -> " << m_names.value(node->m_next) << ";\n";
      if(exportNode(node->m_alternative)) *m_dev << '\t' << name << " -> " << m_names.value(node->m_alternative) << ";\n";
      
      return true;
    }
    
    QString nodesName(const ControlFlowNode* node) const
    {
      if(!node->m_next && !node->m_alternative)
        return "Exit";
      else {
        RangeInRevision range = node->m_nodeRange;
        int a=cursorToPos(range.start), b=cursorToPos(range.end);
        return "\""+m_sources.mid(a, b-a)+"\"";
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
  
  LockedTopDUContext top = parse(code.toUtf8(), DumpAST);
  
  ControlFlowGraph* graph = &m_ctlflowGraph;
  
  KDevelop::ControlFlowGraph::const_iterator it=graph->constBegin(), itEnd=graph->constEnd();
  QSet<ControlFlowNode*> visited;
  int entries=0;
  for(; it!=itEnd; ++it, ++entries)
    walkNodesRecursively(*it, visited);
  
  QCOMPARE(entries, 1);
  QCOMPARE(visited.size(), nodeCount);
  
  QFile file(QString(QTest::currentDataTag())+".dot");
  QVERIFY(file.open(QFile::WriteOnly));
  QTextStream st(&file);
  ControlFlowToDot exporter(&st, code.toUtf8());
  
  for(KDevelop::ControlFlowGraph::const_iterator it=graph->constBegin(), itEnd=graph->constEnd(); it!=itEnd; ++it)
    exporter.exportGraph(QTest::currentDataTag(), *it);
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
  QTest::newRow("switch") << "void f(int a) { switch(a) { case 1: case 2: break; case 3; break;} }" << 5;
  QTest::newRow("do-while") << "void f(int a) { do {a--; } while(a); }" << 5;
  
  QTest::newRow("loopbreak") << "void f(int i) { while(i) { if(i>20) break; } i++; }" << 7;
  QTest::newRow("loopconti") << "void f(int i) { while(i) { if(i>20) continue; } i++; }" << 7;
}
