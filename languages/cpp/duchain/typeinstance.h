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

#ifndef TYPEINSTANCE_H
#define TYPEINSTANCE_H

#include "identifier.h"
#include "kdevdocumentcursorobject.h"

class AbstractType;

class TypeInstance : public KDevDocumentCursorObject
{
public:
  TypeInstance(KTextEditor::Cursor* definitionPoint);

  const Identifier& identifier() const;
  void setIdentifier(const Identifier& identifier);

  AbstractType* type() const;
  void setType(AbstractType* type);

private:
  Identifier m_identifier;
  AbstractType* m_type;
};

class FunctionTypeInstance : public TypeInstance
{
public:
  FunctionTypeInstance(KTextEditor::Cursor* definitionPoint);
  virtual ~FunctionTypeInstance();

  const QList<TypeInstance*>& argumentInstances() const;
  void addArgumentInstance(TypeInstance* instance);
};

#endif // TYPEINSTANCE_H
