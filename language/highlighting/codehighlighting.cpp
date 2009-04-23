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

#include "codehighlighting.h"

#include <KTextEditor/SmartRange>
#include <KTextEditor/SmartInterface>
#include <KTextEditor/Document>

#include "../../interfaces/icore.h"
#include "../../interfaces/ilanguagecontroller.h"
#include "../../interfaces/icompletionsettings.h"

#include "../duchain/declaration.h"
#include "../duchain/types/functiontype.h"
#include "../duchain/types/enumeratortype.h"
#include "../duchain/types/typealiastype.h"
#include "../duchain/types/enumerationtype.h"
#include "../duchain/types/structuretype.h"
#include "../duchain/functiondefinition.h"
#include "../duchain/use.h"

#include "colorcache.h"
#include "configurablecolors.h"

using namespace KTextEditor;

#define LOCK_SMART(range) KTextEditor::SmartInterface* iface = dynamic_cast<KTextEditor::SmartInterface*>(range->document()); QMutexLocker lock(iface ? iface->smartMutex() : 0);

namespace KDevelop {

CodeHighlighting::CodeHighlighting( QObject * parent )
  : QObject(parent), m_localColorization(true), m_globalColorization(true), m_useClassCache(false)
{
  // make sure the singleton is setup
  ColorCache::initialize();

  adaptToColorChanges();

  connect(ColorCache::self(), SIGNAL(colorsGotChanged()),
           this, SLOT(adaptToColorChanges()));
}

CodeHighlighting::~CodeHighlighting( )
{
}

void CodeHighlighting::adaptToColorChanges()
{
  // disable local highlighting if the ratio is set to 0
  m_localColorization = ICore::self()->languageController()->completionSettings()->localColorizationLevel() > 0;
  // disable global highlighting if the ratio is set to 0
  m_globalColorization = ICore::self()->languageController()->completionSettings()->globalColorizationLevel() > 0;

  m_declarationAttributes.clear();
  m_definitionAttributes.clear();
  m_depthAttributes.clear();
  m_referenceAttributes.clear();
}

KTextEditor::Attribute::Ptr CodeHighlighting::attributeForType( Types type, Contexts context, const QColor &color ) const
{
  ///@todo Clear cache when the highlighting has changed
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

  if ( !a || color.isValid() ) {

    a = KTextEditor::Attribute::Ptr(new KTextEditor::Attribute(*ColorCache::self()->defaultColors()->getAttribute(type)));

    switch (context) {
      case DefinitionContext:
      case DeclarationContext:
        a->setFontBold();
        break;
    }

    if( color.isValid() ) {
      a->setForeground(color);
//       a->setBackground(QColor(mix(0xffffff-color, backgroundColor(), 255-backgroundTinting)));
    } else {
      switch (context) {
        case DefinitionContext:
          m_definitionAttributes.insert(type, a);
          break;
        case DeclarationContext:
          m_declarationAttributes.insert(type, a);
          break;
        case ReferenceContext:
          m_referenceAttributes.insert(type, a);
          break;
      }
    }
  }

  return a;
}

void CodeHighlighting::outputRange( KTextEditor::SmartRange * range ) const
{
  kDebug() << range << QString(range->depth(), ' ') << *range << "attr" << range->attribute();
  Q_ASSERT(range->start() <= range->end());
  foreach (SmartRange* child, range->childRanges())
    outputRange(child);
}

ColorMap emptyColorMap() {
 ColorMap ret(ColorCache::self()->validColorCount()+1, 0);
 return ret;
}

void CodeHighlighting::highlightDUChain(TopDUContext* context) const
{
  kDebug() << "highlighting du chain";

  DUChainReadLocker lock(DUChain::lock());

  if ( !m_localColorization && !m_globalColorization ) {
    kDebug() << "highlighting disabled";
    deleteHighlighting(context);
    return;
  }

  m_contextClasses.clear();
  m_useClassCache = true;

  //Highlight
  highlightDUChainSimple(static_cast<DUContext*>(context));

  m_functionColorsForDeclarations.clear();
  m_functionDeclarationsForColors.clear();

  m_useClassCache = false;
  m_contextClasses.clear();
}

void CodeHighlighting::highlightDUChainSimple(DUContext* context) const
{
  if (!context->smartRange()) {
    kDebug() << "not a smart range! highlighting aborted";
    return;
  }

  bool isInFunction = context->type() == DUContext::Function || (context->type() == DUContext::Other && context->owner());

  if( isInFunction && m_localColorization ) {
    highlightDUChain(context, QHash<Declaration*, uint>(), emptyColorMap());
    return;
  }


  foreach (Declaration* dec, context->localDeclarations()) {
    highlightDeclaration(dec, QColor(QColor::Invalid));
  }

  highlightUses(context);

  foreach (DUContext* child, context->childContexts()) {
    highlightDUChainSimple(child);
  }
}

void CodeHighlighting::deleteHighlighting(KDevelop::DUContext* context) const {
  if (!context->smartRange())
    return;

  {
    LOCK_SMART(context->smartRange());

    foreach (Declaration* dec, context->localDeclarations())
      if(dec->smartRange())
        dec->smartRange()->setAttribute(KTextEditor::Attribute::Ptr());

    for(int a = 0; a < context->usesCount(); ++a)
      if(context->useSmartRange(a))
        context->useSmartRange(a)->setAttribute(KTextEditor::Attribute::Ptr());
  }

  foreach (DUContext* child, context->childContexts())
    deleteHighlighting(child);
}

void CodeHighlighting::highlightDUChain(DUContext* context, QHash<Declaration*, uint> colorsForDeclarations, ColorMap declarationsForColors) const
{
  if (!context->smartRange())
    return;

  TopDUContext* top = context->topContext();

  //Merge the colors from the function arguments
  foreach( const DUContext::Import &imported, context->importedParentContexts() ) {
    if(!imported.context(top) || (imported.context(top)->type() != DUContext::Other && imported.context(top)->type() != DUContext::Function))
      continue;
    //For now it's enough simply copying them, because we only pass on colors within function bodies.
    if (m_functionColorsForDeclarations.contains(imported.context(top)))
      colorsForDeclarations = m_functionColorsForDeclarations[imported.context(top)];
    if (m_functionDeclarationsForColors.contains(imported.context(top)))
      declarationsForColors = m_functionDeclarationsForColors[imported.context(top)];
  }

  QList<Declaration*> takeFreeColors;

  foreach (Declaration* dec, context->localDeclarations()) {
    //Initially pick a color using the hash, so the chances are good that the same identifier gets the same color always.
    uint colorNum = dec->identifier().hash() % ColorCache::self()->validColorCount();

    if( declarationsForColors[colorNum] ) {
      takeFreeColors << dec; //Use one of the colors that stays free
      continue;
    }

    colorsForDeclarations[dec] = colorNum;
    declarationsForColors[colorNum] = dec;

    highlightDeclaration(dec, ColorCache::self()->generatedColor(colorNum));
  }

  foreach( Declaration* dec, takeFreeColors ) {
    uint colorNum = dec->identifier().hash() % ColorCache::self()->validColorCount();
    uint oldColorNum = colorNum;
    while( declarationsForColors[colorNum] ) {
      colorNum = (colorNum+1) % ColorCache::self()->validColorCount();
      if( colorNum == oldColorNum ) {
        //Could not allocate a unique color, what now? Just pick the black color.
        colorNum = ColorCache::self()->validColorCount();
        break;
      }
    }
    colorsForDeclarations[dec] = colorNum;
    declarationsForColors[colorNum] = dec;

    highlightDeclaration(dec, ColorCache::self()->generatedColor(colorNum));
  }

  for(int a = 0; a < context->usesCount(); ++a) {
    Declaration* decl = context->topContext()->usedDeclarationForIndex(context->uses()[a].m_declarationIndex);
    QColor color(QColor::Invalid);
    if( colorsForDeclarations.contains(decl) )
      color = ColorCache::self()->generatedColor(colorsForDeclarations[decl]);
    highlightUse(context, a, color);
  }

  foreach (DUContext* child, context->childContexts()) {
    highlightDUChain(child,  colorsForDeclarations, declarationsForColors );
  }
  if(context->type() == DUContext::Other || context->type() == DUContext::Function) {
    m_functionColorsForDeclarations[IndexedDUContext(context)] = colorsForDeclarations;
    m_functionDeclarationsForColors[IndexedDUContext(context)] = declarationsForColors;
  }
}

KTextEditor::Attribute::Ptr CodeHighlighting::attributeForDepth(int depth) const
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

KDevelop::Declaration* CodeHighlighting::localClassFromCodeContext(KDevelop::DUContext* context) const
{
  if(!context)
    return 0;

  if(m_contextClasses.contains(context))
    return m_contextClasses[context];

  DUContext* startContext = context;

  while( context->parentContext() && context->type() == DUContext::Other && context->parentContext()->type() == DUContext::Other )
  { //Move context to the top context of type "Other". This is needed because every compound-statement creates a new sub-context.
    context = context->parentContext();
  }

  ///Step 1: Find the function-declaration for the function we are in
  Declaration* functionDeclaration = 0;

  if( FunctionDefinition* def = dynamic_cast<FunctionDefinition*>(context->owner()) ) {

    if(m_contextClasses.contains(context))
      return m_contextClasses[context];

    functionDeclaration = def->declaration(startContext->topContext());
  }

  if( !functionDeclaration && context->owner() )
    functionDeclaration = context->owner();

  if(!functionDeclaration) {
    if(m_useClassCache)
      m_contextClasses[context] = 0;
    return 0;
  }

  Declaration* decl  = functionDeclaration->context()->owner();

  if(m_useClassCache)
    m_contextClasses[context] = decl;

  return decl;
}

CodeHighlighting::Types CodeHighlighting::typeForDeclaration(Declaration * dec, DUContext* context) const
{
  /**
   * We highlight in 3 steps by priority:
   * 1. Is the item in the local class or an inherited class? If yes, highlight.
   * 2. What kind of item is it? If it's a type/function/enumerator, highlight by type.
   * 3. Else, highlight by scope.
   *
   * */

//   if(ClassMemberDeclaration* classMember = dynamic_cast<ClassMemberDeclaration*>(dec))
//     if(!Cpp::isAccessible(context, classMember))
//       return ErrorVariableType;

  if(!dec)
    return ErrorVariableType;

  Types type = LocalVariableType;
  if(dec->kind() == Declaration::Namespace)
    return NamespaceType;

  if (context && dec->context()->type() == DUContext::Class) {
    //It is a use.
    //Determine the class we're in
    Declaration* klass = localClassFromCodeContext(context);
    if(klass) {
      if (klass->internalContext() == dec->context())
        type = LocalClassMemberType; //Using Member of the local class
      else if (dec->context()->type() == DUContext::Class && klass->internalContext() && klass->internalContext()->imports(dec->context()))
        type = InheritedClassMemberType; //Using Member of an inherited class
    }
  }

  if (type == LocalVariableType) {
    if (dec->kind() == Declaration::Type || dec->type<KDevelop::FunctionType>() || dec->type<KDevelop::EnumeratorType>()) {
      if (dec->isForwardDeclaration())
        type = ForwardDeclarationType;
      else if (dec->type<KDevelop::FunctionType>())
          type = FunctionType;
      else if(dec->type<StructureType>())
          type = ClassType;
      else if(dec->type<KDevelop::TypeAliasType>())
          type = TypeAliasType;
      else if(dec->type<EnumerationType>())
        type = EnumType;
      else if(dec->type<KDevelop::EnumeratorType>())
        type = EnumeratorType;
    }
  }

  if (type == LocalVariableType) {

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
  }

  return type;
}

void CodeHighlighting::highlightDeclaration(Declaration * declaration, const QColor &color) const
{
  if (SmartRange* range = declaration->smartRange()) {
    LOCK_SMART(range);
    range->setAttribute(attributeForType(typeForDeclaration(declaration, 0), DeclarationContext, color));
  }
}

void CodeHighlighting::highlightUse(DUContext* context, int index, const QColor &color) const
{
  if (SmartRange* range = context->useSmartRange(index)) {
    Types type = ErrorVariableType;
    Declaration* decl = context->topContext()->usedDeclarationForIndex(context->uses()[index].m_declarationIndex);

    type = typeForDeclaration(decl, context);

    LOCK_SMART(range);

    if(type != ErrorVariableType || ICore::self()->languageController()->completionSettings()->highlightSemanticProblems())
      range->setAttribute(attributeForType(type, ReferenceContext, color));
    else
      range->setAttribute(KTextEditor::Attribute::Ptr());
  }
}

void CodeHighlighting::highlightUses(DUContext* context) const
{
  for(int a = 0; a < context->usesCount(); ++a) {
    if (SmartRange* range = context->useSmartRange(a)) {
      Types type = ErrorVariableType;
      Declaration* decl = context->topContext()->usedDeclarationForIndex(context->uses()[a].m_declarationIndex);
      if (decl)
        type = typeForDeclaration(decl, context);

    LOCK_SMART(range);
    if(type != ErrorVariableType || ICore::self()->languageController()->completionSettings()->highlightSemanticProblems())
        range->setAttribute(attributeForType(type, ReferenceContext, QColor(QColor::Invalid)));
    else
        range->setAttribute(KTextEditor::Attribute::Ptr());
    }
  }
}

}

#include "codehighlighting.moc"

// kate: space-indent on; indent-width 2; replace-trailing-space-save on; show-tabs on; tab-indents on; tab-width 2;
