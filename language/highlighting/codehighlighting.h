/*
 * This file is part of KDevelop
 *
 * Copyright 2007-2008 David Nolden <david.nolden.kdevelop@art-master.de>
 * Copyright 2006 Hamish Rodda <rodda@kde.org>
 * Copyright 2009 Milian Wolff <mail@milianw.de>
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

#ifndef KDEV_CODEHIGHLIGHTING_H
#define KDEV_CODEHIGHLIGHTING_H

#include <QtCore/QObject>
#include <QtCore/QHash>
#include <QtCore/QModelIndex>
#include <ktexteditor/attribute.h>
#include "../duchain/ducontext.h"
#include "../interfaces/icodehighlighting.h"

namespace KTextEditor { class SmartRange; }
namespace KDevelop
{
class DUContext;
class Declaration;

///@todo make the colorized highlighting work while smart-conversion

///The kate smart-mutex must be locked when this class is used. @todo Lock the smart-mutex selectively within CodeHighlighting, to reduce GUI-Lockups

typedef QVector<KDevelop::Declaration*> ColorMap;

class KDEVPLATFORMLANGUAGE_EXPORT CodeHighlighting : public QObject, public KDevelop::ICodeHighlighting
{
  Q_OBJECT
  Q_INTERFACES(KDevelop::ICodeHighlighting)

  public:
    enum Types {
      UnknownType,
      //Primary highlighting:
      LocalClassMemberType,
      InheritedClassMemberType,
      LocalVariableType,

      //Other highlighting:
      ClassType,
      FunctionType,
      ForwardDeclarationType,
      EnumType,
      EnumeratorType,
      TypeAliasType,

      //If none of the above match:
      MemberVariableType,
      NamespaceVariableType,
      GlobalVariableType,

      //Most of these are currently not used:
      ArgumentType,
      CodeType,
      FileType,
      NamespaceType,
      ScopeType,
      TemplateType,
      TemplateParameterType,
      FunctionVariableType,
      ErrorVariableType
    };

    enum Contexts {
      DefinitionContext,
      DeclarationContext,
      ReferenceContext
    };

    CodeHighlighting(QObject* parent);
    virtual ~CodeHighlighting();

    void highlightDUChain(KDevelop::TopDUContext* context) const;

    void deleteHighlighting(KDevelop::DUContext* context) const;

    virtual void highlightDeclaration(KDevelop::Declaration* declaration, const QColor &color) const;
    virtual void highlightUse(KDevelop::DUContext* context, int index, const QColor &color) const;
    virtual void highlightUses(KDevelop::DUContext* context) const;

    //color should be zero when undecided
    KTextEditor::Attribute::Ptr attributeForType(Types type, Contexts context, const QColor &color) const;
    KTextEditor::Attribute::Ptr attributeForDepth(int depth) const;

  private:
    void highlightDUChainSimple(KDevelop::DUContext* context) const;
    void highlightDUChain(KDevelop::DUContext* context, QHash<KDevelop::Declaration*, uint> colorsForDeclarations, ColorMap) const;
    void outputRange( KTextEditor::SmartRange * range ) const;

  KDevelop::Declaration* localClassFromCodeContext(KDevelop::DUContext* context) const;
    /**
     * @param context Should be the context from where the declaration is used, if a use is highlighted.
     * */
    Types typeForDeclaration(KDevelop::Declaration* dec, KDevelop::DUContext* context) const;

    mutable QHash<Types, KTextEditor::Attribute::Ptr> m_definitionAttributes;
    mutable QHash<Types, KTextEditor::Attribute::Ptr> m_declarationAttributes;
    mutable QHash<Types, KTextEditor::Attribute::Ptr> m_referenceAttributes;

    mutable QList<KTextEditor::Attribute::Ptr> m_depthAttributes;

    //A temporary hash for speedup
    mutable QHash<KDevelop::DUContext*, KDevelop::Declaration*> m_contextClasses;

    //Here the colors of function context are stored until they are merged into the function body
    mutable QMap<KDevelop::IndexedDUContext, QHash<KDevelop::Declaration*, uint> > m_functionColorsForDeclarations;
    mutable QMap<KDevelop::IndexedDUContext, ColorMap> m_functionDeclarationsForColors;

  //Should be used to enable/disable the colorization of local variables and their uses
  bool m_localColorization;
  mutable bool m_useClassCache;
};

}

#endif

// kate: space-indent on; indent-width 2; replace-trailing-space-save on; show-tabs on; tab-indents on; tab-width 2;
