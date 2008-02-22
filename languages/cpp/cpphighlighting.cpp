/*
 * KDevelop C++ Highlighting Support
 *
 * Copyright 2006 Hamish Rodda <rodda@kde.org>
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

#include "cpphighlighting.h"

#include <ktexteditor/smartrange.h>

#include <topducontext.h>
#include <declaration.h>
#include <definition.h>
#include <use.h>
#include "cpptypes.h"
#include <duchain.h>
#include <duchainlock.h>
#include "cpplanguagesupport.h"

using namespace KTextEditor;
using namespace KDevelop;

uint colors[] = {0x421919/* gray red */, 0x422f19 /* gray orange */,0x2a4219 /* gray green/orange */, 0x194219 /* gray pure green*/, 0x194239 /* gray green/blue */, 0x192942 /* gray blue */, 0x191c42 /* gray deep blue */, 0x321942 /* gray violet blue */, 0x42193b /* gray violet */, 0};

const uint numColors = 9;

CppHighlighting::CppHighlighting( QObject * parent )
  : QObject(parent), m_localColorization(true)
{
}

CppHighlighting::~CppHighlighting( )
{
}

KTextEditor::Attribute::Ptr CppHighlighting::attributeForType( Types type, Contexts context, uint color ) const
{
  KTextEditor::Attribute::Ptr a;
  switch (context) {
    case DefinitionContext:
      a = m_definitionAttributes[type];
      break;

    case DeclarationContext:
      a = m_declarationAttributes[type];
      break;

    case ReferenceContext:
      a = m_referenceAttributes[type];
      break;
  }

  if (!a || color ) {
    a = KTextEditor::Attribute::Ptr(new KTextEditor::Attribute());
    a->setBackgroundFillWhitespace(true);
    switch (context) {
      case DefinitionContext:
        if( !color )
          m_definitionAttributes.insert(type, a);
        break;

      case DeclarationContext:
        if( !color )
          m_declarationAttributes.insert(type, a);
        break;

      case ReferenceContext:
        if( !color )
          m_referenceAttributes.insert(type, a);
        break;
    }

    switch (type) {
      case UnknownType:
        // Chocolate orange
        a->setForeground(QColor(0xA0320A));
        break;

      case ArgumentType:
        // Steel
        a->setBackground(QColor(0x435361));
        break;

      case ClassType: {
        // Scarlet
        a->setForeground(QColor(0x7B0859));

        KTextEditor::Attribute::Ptr e(new KTextEditor::Attribute());
        e->setForeground(QColor(0x005500));
        a->setDynamicAttribute(Attribute::ActivateCaretIn, e);
        //a->setEffects(Attribute::EffectFadeIn | Attribute::EffectFadeOut);
        break;
      }

      case FunctionDefinitionType:
        //a->setFontBold();
        // fallthrough

      case FunctionType:
        // Navy blue
        a->setForeground(QColor(0x21005A));
        break;

      case MemberVariableType:
        // Dark Burple (blue / purple)
        a->setForeground(QColor(0x443069));
        break;

      case LocalVariableType:
        // Dark aquamarine
        a->setForeground(QColor(0x0C4D3C));
        break;

      case FunctionVariableType:
        // Less dark navy blue
        a->setForeground(QColor(0x300085));
        break;

      case NamespaceVariableType:
        // Rose
        a->setForeground(QColor(0x9F3C5F));
        break;

      case GlobalVariableType:
        // Grass green
        a->setForeground(QColor(0x12762B));
        break;

      case NamespaceType:
        // Dark rose
        a->setForeground(QColor(0x6B2840));
        break;

      case ErrorVariableType:
        // Slightly less intense red
        a->setForeground(QColor(0x9F3C5F));
        break;

      case ForwardDeclarationType:
        // Gray
        a->setForeground(QColor(0x5C5C5C));
        break;

      case ScopeType:
      case TemplateType:
      case TemplateParameterType:
      case TypeAliasType:
      case CodeType:
      case EnumType:
      case EnumeratorType:
      case FileType:
        break;
    }

    switch (context) {
      case DefinitionContext:
        a->setFontBold();
        break;

      case DeclarationContext:
        a->setFontBold();
        //a->setFontUnderline(true);
        break;

      case ReferenceContext:
        KTextEditor::Attribute::Ptr d(new KTextEditor::Attribute());
        d->setBackground(QColor(Qt::blue).light(190));
        d->setEffects(Attribute::EffectFadeIn | Attribute::EffectFadeOut);
        a->setDynamicAttribute(Attribute::ActivateMouseIn, d);
        break;
    }
    if( color ) {
      a->setForeground(QColor((color*5)/2));//0xffffff-(color)));
                       //a->setBackground(QColor((color*7)));
    }
  }

  return a;
}

void CppHighlighting::highlightTree( KTextEditor::SmartRange * range ) const
{
  int depth = range->depth();
  range->setAttribute(m_depthAttributes[depth]);
  foreach (KTextEditor::SmartRange* child, range->childRanges())
    highlightTree(child);
}

void CppHighlighting::outputRange( KTextEditor::SmartRange * range ) const
{
  kDebug( 9007 ) << range << QString(range->depth(), ' ') << *range << "attr" << range->attribute();
  Q_ASSERT(range->start() <= range->end());
  foreach (SmartRange* child, range->childRanges())
    outputRange(child);
}

ColorMap emptyColorMap() {
  ColorMap ret(numColors+1, 0);
 return ret;
}

void CppHighlighting::highlightDUChain(TopDUContext* context) const
{
  kDebug( 9007 ) << "highighting du chain";
  
  DUChainReadLocker lock(DUChain::lock());
  TopDUContext* standardCtx = CppLanguageSupport::self()->standardContext(KUrl(context->url().str()), false);

  //Only highlight if this is the standard context(we only want exactly one context highlighted at a time)
  if(context == standardCtx) {
    //Clear the highlighting of all other contexts for this file
    QList<TopDUContext*> contexts = DUChain::self()->chainsForDocument(context->url());

    foreach(TopDUContext* ctx, contexts) {
      if(ctx == context)
        continue;
      deleteHighlighting(ctx);
    }
    
    //Highlight
    highlightDUChainSimple(static_cast<DUContext*>(context));

    m_functionColorsForDeclarations.clear();
    m_functionDeclarationsForColors.clear();
  }
}

void CppHighlighting::highlightDUChainSimple(DUContext* context) const
{
  if (!context->smartRange())
    return;

  bool isInFunction = context->type() == DUContext::Function || (context->type() == DUContext::Other && context->owner());
  
  if( isInFunction && m_localColorization ) {
    highlightDUChain(context, QHash<Declaration*, uint>(), emptyColorMap());
    return;
  }
  

  foreach (Declaration* dec, context->localDeclarations()) {
    highlightDeclaration(dec, 0);
  }

  foreach (Definition* def, context->localDefinitions())
    highlightDefinition(def);

  highlightUses(context);

  foreach (DUContext* child, context->childContexts()) {
    highlightDUChainSimple(child );
  }
}

void CppHighlighting::deleteHighlighting(KDevelop::DUContext* context) const {
  if (!context->smartRange())
    return;

  foreach (Declaration* dec, context->localDeclarations())
    if(dec->smartRange())
      dec->smartRange()->setAttribute(KTextEditor::Attribute::Ptr());

  foreach (Definition* def, context->localDefinitions())
    if(def->smartRange())
      def->smartRange()->setAttribute(KTextEditor::Attribute::Ptr());

  for(int a = 0; a < context->uses().count(); ++a)
    if(context->useSmartRange(a))
      context->useSmartRange(a)->setAttribute(KTextEditor::Attribute::Ptr());

  foreach (DUContext* child, context->childContexts())
    deleteHighlighting(child);
}

void CppHighlighting::highlightDUChain(DUContext* context, QHash<Declaration*, uint> colorsForDeclarations, ColorMap declarationsForColors) const
{
  if (!context->smartRange())
    return;

  //Merge the colors from the function arguments
  foreach( DUContextPointer imported, context->importedParentContexts() ) {
    //For now it's enough simply copying them, because we only pass on colors within function bodies.
    if (m_functionColorsForDeclarations.contains(imported))
      colorsForDeclarations = m_functionColorsForDeclarations[imported];
    if (m_functionDeclarationsForColors.contains(imported))
      declarationsForColors = m_functionDeclarationsForColors[imported];
  }

  QList<Declaration*> takeFreeColors;
  
  foreach (Declaration* dec, context->localDeclarations()) {
    //Initially pick a color using the hash, so the chances are good that the same identifier gets the same color always.
    uint colorNum = dec->identifier().hash() % numColors;

    if( declarationsForColors[colorNum] ) {
      takeFreeColors << dec; //Use one of the colors that stays free
      continue;
    }

    colorsForDeclarations[dec] = colorNum;
    declarationsForColors[colorNum] = dec;
    
    highlightDeclaration(dec, colors[colorNum]);
  }

  foreach( Declaration* dec, takeFreeColors ) {
    uint colorNum = dec->identifier().hash() % numColors;
    uint oldColorNum = colorNum;
    while( declarationsForColors[colorNum] ) {
      colorNum = (colorNum+1) % numColors;
      if( colorNum == oldColorNum ) {
        //Could not allocate a unique color, what now? Just pick the black color.
        colorNum = numColors;
        break;
      }
    }
    colorsForDeclarations[dec] = colorNum;
    declarationsForColors[colorNum] = dec;
    
    highlightDeclaration(dec, colors[colorNum]);
  }

  foreach (Definition* def, context->localDefinitions())
    highlightDefinition(def);

  for(int a = 0; a < context->uses().count(); ++a) {
    if(context->uses()[a].m_declarationIndex < 0) {
      Declaration* decl = context->declarationForUse(a, context->topContext());
      uint colorNum = numColors;
      if( colorsForDeclarations.contains(decl) )
        colorNum = colorsForDeclarations[decl];
      highlightUse(context, a, colors[colorNum]);
    }
  }

  foreach (DUContext* child, context->childContexts())
    highlightDUChain(child,  colorsForDeclarations, declarationsForColors );

  m_functionColorsForDeclarations[DUContextPointer(context)] = colorsForDeclarations;
  m_functionDeclarationsForColors[DUContextPointer(context)] = declarationsForColors;
}

KTextEditor::Attribute::Ptr CppHighlighting::attributeForDepth(int depth) const
{
  while (depth >= m_depthAttributes.count()) {
    KTextEditor::Attribute::Ptr a(new KTextEditor::Attribute());
    a->setBackground(QColor(Qt::white).dark(100 + (m_depthAttributes.count() * 25)));
    a->setBackgroundFillWhitespace(true);
    if (depth % 2)
      a->setOutline(Qt::red);
    m_depthAttributes.append(a);
  }

  return m_depthAttributes[depth];
}

#include "cpphighlighting.moc"


CppHighlighting::Types CppHighlighting::typeForDeclaration(Declaration * dec) const
{
  Types type = LocalVariableType;
  if (dec->context()->scopeIdentifier().isEmpty())
    type = GlobalVariableType;
  if (dec->isForwardDeclaration())
    type = ForwardDeclarationType;
  else if (dec->type<CppClassType>())
    type = ClassType;
  else if (dec->type<CppFunctionType>())
    type = FunctionType;
  else
    switch (dec->context()->type()) {
      case DUContext::Namespace:
        type = NamespaceVariableType;
        break;
      case DUContext::Class:
        type = MemberVariableType;
        break;
      case DUContext::Function:
        type = FunctionVariableType;
        break;
      default:
        break;
    }

  return type;
}

void CppHighlighting::highlightDefinition(Definition * definition) const
{
  if (Declaration* declaration = definition->declaration())
    if (SmartRange* range = definition->smartRange())
      range->setAttribute(attributeForType(typeForDeclaration(declaration), DeclarationContext, 0));
}

void CppHighlighting::highlightDeclaration(Declaration * declaration, uint color) const
{
  if (SmartRange* range = declaration->smartRange())
    range->setAttribute(attributeForType(typeForDeclaration(declaration), DeclarationContext, color));
}

void CppHighlighting::highlightDeclaration(Declaration * declaration) const
{
  if (SmartRange* range = declaration->smartRange())
    range->setAttribute(attributeForType(typeForDeclaration(declaration), DeclarationContext, 0));
}

void CppHighlighting::highlightUse(DUContext* context, int index, uint color) const
{
  if (SmartRange* range = context->useSmartRange(index)) {
    Types type = ErrorVariableType;
    Declaration* decl = context->declarationForUse(index, context->topContext());
    if (decl)
      type = typeForDeclaration(decl);

    range->setAttribute(attributeForType(type, ReferenceContext, color));
  }
}

void CppHighlighting::highlightUses(DUContext* context) const
{
  for(int a = 0; a < context->uses().count(); ++a) {
    if (SmartRange* range = context->useSmartRange(a)) {
      Types type = ErrorVariableType;
      Declaration* decl = context->declarationForUse(a, context->topContext());
      if (decl)
        type = typeForDeclaration(decl);

      range->setAttribute(attributeForType(type, ReferenceContext, 0));
    }
  }
}
