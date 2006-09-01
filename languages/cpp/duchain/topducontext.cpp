/* This  is part of KDevelop
    Copyright (C) 2006 Hamish Rodda <rodda@kde.org>

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

#include "topducontext.h"

#include "symboltable.h"
#include "declaration.h"

using namespace KTextEditor;

TopDUContext::TopDUContext(KTextEditor::Range* range)
  : DUContext(range)
  , m_hasUses(false)
{
}

TopDUContext::~TopDUContext( )
{
}

void TopDUContext::setHasUses(bool hasUses)
{
  m_hasUses = hasUses;
}

bool TopDUContext::hasUses() const
{
  return m_hasUses;
}

Declaration* TopDUContext::findDeclarationInternal(const QualifiedIdentifier& identifier, const KTextEditor::Cursor& position, const AbstractType::Ptr& dataType, QList<UsingNS*>* usingNS, bool inImportedContext) const
{
  QList<Declaration*> found = checkDeclarations(SymbolTable::self()->findDeclarations(identifier), position, dataType);

  if (found.count() == 1)
    return found.first();

  if (found.count() > 1) {
    /*kWarning() << k_funcinfo << "Multiple definitions for " << identifier.toString() << endl;
    foreach (Declaration* dec, found)
      kDebug() << "Found at " << dec->textRange() << " in document " << dec->url() << endl;*/

    return found.first();
  }

  foreach (UsingNS* ns, usingNamespaces())
    if (ns->textCursor() <= position)
      usingNS->append(ns);

  foreach (UsingNS* ns, *usingNS) {
    QualifiedIdentifier id = identifier.merge(ns->nsIdentifier);

    // FIXME nested using definitions

    found = checkDeclarations(SymbolTable::self()->findDeclarations(id), position, dataType);

    if (found.count() == 1)
      return found.first();

    if (found.count() > 1) {
      /*kWarning() << k_funcinfo << "Multiple definitions for " << identifier.toString() << endl;
      foreach (Declaration* dec, found)
        kDebug() << "Found at " << dec->textRange() << " in document " << dec->url() << endl;*/

      return found.first();
    }
  }

  return 0;
}

bool TopDUContext::imports(TopDUContext * origin, int depth) const
{
  if (depth == 100) {
    kWarning() << k_funcinfo << "Imported context list too deep!" << endl;
    return false;
  }

  foreach (DUContext* context, importedParentContexts()) {
    Q_ASSERT(dynamic_cast<TopDUContext*>(context));
    TopDUContext* top = static_cast<TopDUContext*>(context);
    if (top == origin)
      return true;
    if (top->imports(origin, depth + 1))
      return true;
  }

  return false;
}

QList<Declaration*> TopDUContext::checkDeclarations(const QList<Declaration*>& declarations, const KTextEditor::Cursor& position, const AbstractType::Ptr& dataType) const
{
  QList<Declaration*> found;

  foreach (Declaration* dec, declarations) {
    if (dataType && dec->abstractType() != dataType)
      continue;

    TopDUContext* top = dec->context()->topContext();
    if (top != this) {
      // Make sure that this declaration is accessible
      // TODO when import location available, use that too
      if (!imports(top))
        continue;

    } else {
      if (dec->textRange().start() > position)
        continue;
    }

    found.append(dec);
  }

  return found;
}

// kate: indent-width 2;
