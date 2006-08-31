/*
 * KDevelop C++ Code Completion Support
 *
 * Copyright (c) 2006 Hamish Rodda <rodda@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "cppcodecompletionmodel.h"

#include "declaration.h"
#include "cpptypes.h"
#include "classfunctiondeclaration.h"
#include "ducontext.h"

using namespace KTextEditor;

CppCodeCompletionModel::CppCodeCompletionModel( QObject * parent )
  : CodeCompletionModel(parent)
{
}

CppCodeCompletionModel::~CppCodeCompletionModel()
{
}

QVariant CppCodeCompletionModel::data(const QModelIndex& index, int role) const
{
  Declaration* dec = static_cast<Declaration*>(index.internalPointer());

  switch (role) {
    case Qt::DisplayRole:
      switch (index.column()) {
        case Prefix:
          if (dec->abstractType()) {
            if (CppFunctionType::Ptr functionType = dec->type<CppFunctionType>()) {
              if (functionType->returnType())
                return functionType->returnType()->toString();
              else
                return "<incomplete type>";

            } else {
              return dec->abstractType()->toString();
            }

          } else {
            return "<incomplete type>";
          }

        case Scope: {
          QualifiedIdentifier id = dec->qualifiedIdentifier();
          if (id.isEmpty())
            return QVariant();
          id.pop();
          return id.toString() + "::";
        }

        case Name:
          return dec->identifier().toString();

        case Arguments:
          if (CppFunctionType::Ptr functionType = dec->type<CppFunctionType>()) {
            QString ret = "(";
            bool first = true;
            foreach (const AbstractType::Ptr& argument, functionType->arguments()) {
              if (first)
                first = false;
              else
                ret += ", ";

              if (argument)
                ret += argument->toString();
              else
                ret += "<incomplete type>";
            }
            ret += ")";
            return ret;
          }
          break;

        case Postfix:
          if (CppFunctionType::Ptr functionType = dec->type<CppFunctionType>()) {
            return functionType->cvString();
          }
          break;
      }
      break;

    case Qt::DecorationRole:
      break;

    case CompletionRole: {
      CompletionProperties p;
      if (ClassMemberDeclaration* member = dynamic_cast<ClassMemberDeclaration*>(dec)) {
        switch (member->accessPolicy()) {
          case Cpp::Public:
            p |= Public;
            break;
          case Cpp::Protected:
            p |= Protected;
            break;
          case Cpp::Private:
            p |= Private;
            break;
        }

        if (member->isStatic())
          p |= Static;
        if (member->isAuto())
          ;//TODO
        if (member->isFriend())
          p |= Friend;
        if (member->isRegister())
          ;//TODO
        if (member->isExtern())
          ;//TODO
        if (member->isMutable())
          ;//TODO
      }

      if (ClassFunctionDeclaration* function = dynamic_cast<ClassFunctionDeclaration*>(dec)) {
        if (function->isVirtual())
          p |= Virtual;
        if (function->isInline())
          p |= Inline;
        if (function->isExplicit())
          ;//TODO
      }

      if (dec->abstractType()) {
        if (CppCVType* cv = dynamic_cast<CppCVType*>(dec->abstractType().data())) {
          if (cv->isConstant())
            p |= Const;
          if (cv->isVolatile())
            ;//TODO
          }

        switch (dec->abstractType()->whichType()) {
          case AbstractType::TypeIntegral:
            if (dec->type<CppEnumerationType>())
              p |= Enum;
            else
              p |= Variable;
            break;
          case AbstractType::TypePointer:
            p |= Variable;
            break;
          case AbstractType::TypeReference:
            p |= Variable;
            break;
          case AbstractType::TypeFunction:
            p |= Function;
            break;
          case AbstractType::TypeStructure:
            if (CppClassType::Ptr classType =  dec->type<CppClassType>())
              switch (classType->classType()) {
                case CppClassType::Class:
                  p |= Class;
                  break;
                case CppClassType::Struct:
                  p |= Struct;
                  break;
                case CppClassType::Union:
                  p |= Union;
                  break;
              }
          case AbstractType::TypeArray:
            p |= Variable;
            break;
          case AbstractType::TypeAbstract:
            // TODO
            break;
        }
      }

      /*
      LocalScope      = 0x80000,
      NamespaceScope  = 0x100000,
      GlobalScope     = 0x200000,
      */

      return (int)p;
    }

    case ScopeIndex:
      return static_cast<int>(reinterpret_cast<long>(dec->context()));
  }

  return QVariant();
}

QModelIndex CppCodeCompletionModel::index(int row, int column, const QModelIndex& parent) const
{
  if (row < 0 || row >= m_declarations.count() || column < 0 || column >= ColumnCount || parent.isValid())
    return QModelIndex();

  return createIndex(row, column, m_declarations.at(row));
}

int CppCodeCompletionModel::rowCount ( const QModelIndex & parent ) const
{
  if (parent.isValid())
    return 0;

  return m_declarations.count();
}

void CppCodeCompletionModel::setContext(DUContext * context, const KTextEditor::Cursor& position)
{
  m_context = context;
  Q_ASSERT(m_context);

  m_declarations = m_context->allDeclarations(position).values();
}

#include "cppcodecompletionmodel.moc"

// kate: space-indent on; indent-width 2; replace-tabs on
