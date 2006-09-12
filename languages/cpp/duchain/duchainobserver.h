/* This file is part of KDevelop
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

#ifndef DUCHAINOBSERVER_H
#define DUCHAINOBSERVER_H

class DUChainBase;
class DUContext;
class Declaration;
class Definition;
class Use;

/**
 * Abstract class for observers of the definition-use chain to receive
 * feedback on changes.
 */
class DUChainObserver
{
public:
  virtual ~DUChainObserver() {};

  enum Modification {
    Addition,
    Removal,
    Change,
    Deletion
  };

  enum Relationship {
    // Context
    ParentContext,
    ChildContexts,
    ImportedParentContexts,
    ImportedChildContexts,
    LocalDeclarations,
    LocalDefinitions,
    UsingNamespaces,
    ContextType,

    // Context + Declaration
    Uses,
    Identifier,

    // Declaration
    DataType,
    ForwardDeclarations,

    // Declaration + Definition + Use
    Context,

    // Declaration + Definition
    DefinitionRelationship,

    // Use
    DeclarationRelationship,

    // All
    NotApplicable
  };

  virtual void contextChanged(DUContext* context, Modification change, Relationship relationship, DUChainBase* relatedObject = 0) = 0;

  virtual void declarationChanged(Declaration* declaration, Modification change, Relationship relationship, DUChainBase* relatedObject = 0) = 0;

  virtual void definitionChanged(Definition* definition, Modification change, Relationship relationship, DUChainBase* relatedObject = 0) = 0;

  virtual void useChanged(Use* use, Modification change, Relationship relationship, DUChainBase* relatedObject = 0) = 0;
};

#endif // DUCHAINOBSERVER_H

// kate: indent-width 2;
