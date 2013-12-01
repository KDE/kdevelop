/* This file is part of KDevelop
    Copyright 2006 Hamish Rodda <rodda@kde.org>
    Copyright 2007-2009 David Nolden <david.nolden.kdevelop@art-master.de>

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

#include "testhelper.h"

#include "parsesession.h"
#include "parser.h"
#include "rpp/preprocessor.h"
#include "rpp/pp-engine.h"
#include "declarationbuilder.h"
#include "usebuilder.h"

#include <language/duchain/topducontext.h>
#include <language/duchain/dumpchain.h>
#include <language/codegen/coderepresentation.h>
#include <tests/autotestshell.h>
#include <tests/testcore.h>
#include <usedecoratorvisitor.h>
#include <controlflowgraphbuilder.h>
#include <environmentmanager.h>

using namespace KDevelop;

using namespace Cpp;

void Cpp::release( TopDUContext* top )
{
  //KDevelop::EditorIntegrator::releaseTopRange(top->textRangePtr());

  TopDUContextPointer tp(top);
  DUChain::self()->removeDocumentChain(static_cast<TopDUContext*>(top));
  Q_ASSERT(!tp);
}

//BEGIN LockedTopDUContext

LockedTopDUContext::LockedTopDUContext( TopDUContext* top )
  : m_top( top )
{
}

LockedTopDUContext::operator KDevelop::TopDUContext*() const
{
  return m_top;
}

TopDUContext* LockedTopDUContext::operator->() const
{
  return m_top;
}

LockedTopDUContext& LockedTopDUContext::operator=( TopDUContext * ctx )
{
  m_top = ctx;
  return *this;
}

LockedTopDUContext::~LockedTopDUContext()
{
  DUChainWriteLocker lock;
  release( m_top );
}

//BEGIN TestHelper

void TestHelper::initShell()
{
  AutoTestShell::init(QStringList() << "kdevcppsupport");
  TestCore* core = new TestCore();
  core->initialize(KDevelop::Core::NoUi);
  EnvironmentManager::init();

  DUChain::self()->disablePersistentStorage();
  KDevelop::CodeRepresentation::setDiskChangesForbidden(true);
}

TopDUContext* TestHelper::parse(const QByteArray& unit, DumpAreas dump, TopDUContext* update, bool keepAst)
{
  m_modifications.clear();
  m_ctlflowGraph.clear();

  if (dump)
    kDebug(9007) << "==== Beginning new test case...:" << endl << unit;

  //If the AST flag is set, then the parse session needs to be owned by a shared pointer
  ParseSession::Ptr session(new ParseSession());

  rpp::Preprocessor preprocessor;
  rpp::pp pp(&preprocessor);

  session->setContentsAndGenerateLocationTable(pp.processFile("anonymous", unit));

  Parser parser(&control);
  TranslationUnitAST* ast = parser.parse(session.data());
  ast->session = session.data();

  if (dump & DumpAST) {
    kDebug(9007) << "===== AST:";
    cppDumper.dump(ast, session.data());
  }

  static int testNumber = 0;
  IndexedString url(QString("/internal/%1").arg(testNumber++));

  DeclarationBuilder definitionBuilder(session.data());
  Cpp::EnvironmentFilePointer file( new Cpp::EnvironmentFile( url, 0 ) );

  //HACK Should actually use DUChain::updateContextForUrl
  kDebug() << "update->features & TopDUContext::AST: " << (update ? update->features() & TopDUContext::AST : 0);
  if(keepAst) {
    definitionBuilder.setMapAst(true);
    if (update) {
      DUChainWriteLocker lock(DUChain::lock());
      update->setAst( IAstContainer::Ptr( session.data() ) );
    }
  }
  TopDUContext* top = definitionBuilder.buildDeclarations(file, ast, 0, ReferencedTopDUContext(update));
  if(update) {
    Q_ASSERT(top == update);
  }
  if (top && keepAst) {
    DUChainWriteLocker lock(DUChain::lock());
    top->setAst( IAstContainer::Ptr( session.data() ) );
  }

  UseBuilder useBuilder(session.data());
  useBuilder.setMapAst(keepAst);
  useBuilder.buildUses(ast);
  
  UseDecoratorVisitor visit(session.data(), &m_modifications);
  visit.run(ast);
  
  ControlFlowGraphBuilder flowvisitor(ReferencedTopDUContext(top), session.data(), &m_ctlflowGraph);
  flowvisitor.run(ast);

  if (dump & DumpDUChain) {
    kDebug(9007) << "===== DUChain:";

    DUChainWriteLocker lock(DUChain::lock());
    dumpDUContext(top);
  }

  if (dump)
    kDebug(9007) << "===== Finished test case.";

  return top;
}

namespace Cpp {
void dump(const TemplateDeclaration::InstantiationsHash& instantiations)
{
  TemplateDeclaration::InstantiationsHash::const_iterator it = instantiations.constBegin();
  while(it != instantiations.constEnd()) {
    qDebug() << it.key().information().toString();
    ++it;
  }
}

QByteArray readCodeFile(const QString& file)
{
  QFile f(QFileInfo(__FILE__).absolutePath() + QLatin1String("/data/") + file);
  qDebug() << f.fileName();
  if (!f.open(QIODevice::ReadOnly)) {
    kWarning() << "Could not open test file" << file;
    return QByteArray();
  }
  return f.readAll();
}
}
