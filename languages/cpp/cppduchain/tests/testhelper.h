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

#ifndef TESTHELPER_H
#define TESTHELPER_H

#include <QByteArray>

#include "dumpchain.h"
#include <templatedeclaration.h>
#include "control.h"
#include <language/duchain/duchainlock.h>
#include <language/checks/dataaccessrepository.h>
#include <language/checks/controlflowgraph.h>

namespace KDevelop {
class TopDUContext;
}

namespace Cpp {

void release(KDevelop::TopDUContext* top);

/// Object that locks the duchain for writing and destroys its TopDUContext on destruction
struct LockedTopDUContext
{
  LockedTopDUContext(KDevelop::TopDUContext* top) ;

  ~LockedTopDUContext() ;
  LockedTopDUContext& operator=(KDevelop::TopDUContext* ctx);
  KDevelop::TopDUContext* operator->() const;
  operator KDevelop::TopDUContext*() const;

  KDevelop::TopDUContext* m_top;
  KDevelop::DUChainWriteLocker m_writeLock;
};

class TestHelper {
public:

  enum DumpArea {
    DumpNone = 0,
    DumpAST = 1,
    DumpDUChain = 2,
    DumpType = 4,
    DumpAll = 7
  };
  Q_DECLARE_FLAGS(DumpAreas, DumpArea)

  void initShell();

  KDevelop::TopDUContext* parse(const QByteArray& unit,
                                DumpAreas dump = static_cast<DumpAreas>(DumpAST | DumpDUChain | DumpType),
                                KDevelop::TopDUContext* update = 0, bool keepAst = false);
protected:
  KDevelop::DataAccessRepository m_modifications;
  KDevelop::ControlFlowGraph m_ctlflowGraph;

private:
  // Parser
  Control control;
  DumpChain cppDumper;

};

void dump(const TemplateDeclaration::InstantiationsHash& instantiations);

/**
 * Read contents of file @p file in cppduchain/tests/data directory.
 */
QByteArray readCodeFile(const QString& file);

}

#endif // TESTHELPER_H
