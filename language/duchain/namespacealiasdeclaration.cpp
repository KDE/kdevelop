/* This  is part of KDevelop
    Copyright 2007 David Nolden <david.nolden.kdevelop@art-master.de>

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

#include "namespacealiasdeclaration.h"

#include "ducontext.h"

namespace KDevelop
{

struct NamespaceAliasDeclarationPrivate
{
  QualifiedIdentifier m_importIdentifier; //The identifier that was imported
};

NamespaceAliasDeclaration::NamespaceAliasDeclaration(const NamespaceAliasDeclaration& rhs) : Declaration(rhs), d(new NamespaceAliasDeclarationPrivate(*rhs.d)) {
}

NamespaceAliasDeclaration::NamespaceAliasDeclaration(KTextEditor::Range * range, Scope scope, DUContext* context)
  : Declaration(range, scope, context), d(new NamespaceAliasDeclarationPrivate)
{
}

QualifiedIdentifier NamespaceAliasDeclaration::importIdentifier() const {
  return d->m_importIdentifier;
}

void NamespaceAliasDeclaration::setImportIdentifier(const QualifiedIdentifier& id) {
  d->m_importIdentifier = id;
}

NamespaceAliasDeclaration::~NamespaceAliasDeclaration()
{
}

Declaration* NamespaceAliasDeclaration::clone() const {
  return new NamespaceAliasDeclaration(*this);
}

void NamespaceAliasDeclaration::setAbstractType(AbstractType::Ptr type) {
  //A namespace-import does not have a type, so ignore any set type
}

QString NamespaceAliasDeclaration::toString() const {
  if( identifier() != globalImportIdentifier )
    return QString("Import %1 as %2").arg(d->m_importIdentifier.toString()).arg(identifier().toString());
  else
    return QString("Import %1").arg(d->m_importIdentifier.toString());
}

}
// kate: space-indent on; indent-width 2; tab-width 4; replace-tabs on; auto-insert-doxygen on
