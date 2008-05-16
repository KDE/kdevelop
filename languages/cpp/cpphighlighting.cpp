/*
 * KDevelop C++ Highlighting Support
 *
 * Copyright 2007-2008 David Nolden <david.nolden.kdevelop@art-master.de>
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

#include <QApplication>
#include <QPalette>

#include <kapplication.h>
#include <ktexteditor/smartrange.h>
#include <kcolorutils.h>
#include <topducontext.h>
#include <declaration.h>
#include <use.h>
#include "cpptypes.h"
#include <duchain.h>
#include <duchainlock.h>
#include "cpplanguagesupport.h"
#include "cppduchain.h"

using namespace KTextEditor;
using namespace KDevelop;

QList<uint>  colors;
uint validColorCount = 0; //Must always be colors.count()-1, because the last color must be the fallback text color
uint totalColorInterpolationSteps = 6 * 0xff; //The total ring of all colors: 0xff0000 -> 0xffff00 -> 0x00ff00 -> 0x00ffff -> 0x0000ff -> 0xff00ff -> ...: 6 * 256 steps
uint colorOffset = 0; //Maybe make this configurable: An offset where to start stepping through the color wheel
uchar foregroundRatio = 110; ///@todo this needs a knob in the configuration: How the color should be mixed with the foreground color. Between 0 and 255, where 255 means only foreground color, and 0 only the chosen color.

///@param ratio ratio between 0 and 0xff
uint mix(uint color1, uint color2, uchar ratio) {
  return (((quint64)color1) * ((quint64)0xff - ratio) + ((quint64)color2) * ratio) / (quint64)0xff;
}

///Generates a color from the color wheel. @param step Step-number, one of totalColorInterpolationSteps
uint interpolate(uint step) {
  uint waypoint = step / 0xff;
  step -= waypoint * 0xff;
  waypoint %= 6;
  uint nextWaypoint = (waypoint + 1) % 6;
  uint interpolationWaypoints[] = {0xff0000, 0xffff00, 0x00ff00, 0x00ffff, 0x0000ff, 0xff00ff};
  return mix(interpolationWaypoints[waypoint], interpolationWaypoints[nextWaypoint], step);
}

void generateColors(int count) {
  colors.clear();
  ///@todo Find the correct text foreground color from kate! The palette thing below returns some strange other color.
  uint standardColor(0u); //QApplication::palette().foreground().color().rgb());
  uint step = totalColorInterpolationSteps / count;
  uint currentPos = colorOffset;
  kDebug() << "text color:" << (void*)QApplication::palette().text().color().rgb();
  for(int a = 0; a < count; ++a) {
    kDebug() << "color" << a << "interpolated from" << currentPos << " < " << totalColorInterpolationSteps << ":" << (void*)interpolate( currentPos );
    colors.append( mix(interpolate( currentPos ), standardColor, foregroundRatio) );
    //colors.append( interpolate(currentPos).rgb() );
    currentPos += step;
  }
  colors.append(standardColor);
  validColorCount = colors.count()-1;
}

CppHighlighting::CppHighlighting( QObject * parent )
  : QObject(parent), m_localColorization(true), m_useClassCache(false)
{
  generateColors(10);
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
//       case UnknownType:
//         // Chocolate orange
//         a->setForeground(QColor(0xA0320A));
//         break;

/*      case ArgumentType:
        // Steel
        a->setBackground(QColor(0x435361));
        break;*/
      case ClassType: {
        a->setForeground(QColor(0x005912)); //Dark green

        KTextEditor::Attribute::Ptr e(new KTextEditor::Attribute());
        e->setForeground(QColor(0x005500));
        a->setDynamicAttribute(Attribute::ActivateCaretIn, e);
        //a->setEffects(Attribute::EffectFadeIn | Attribute::EffectFadeOut);
        break;
      }
      case TypeAliasType:
        a->setForeground(QColor(0x00981e)); //Lighter greyish green
        break;
        
      case EnumType:
        a->setForeground(QColor(0x6c101e)); //Dark red
        break;
        
      case EnumeratorType:
        a->setForeground(QColor(0x862a38)); //Greyish red
        break;

      case FunctionType:
        // Navy blue
        a->setForeground(QColor(0x21005A));
        break;

      case MemberVariableType:
        // Dark Burple (blue / purple)
        a->setForeground(QColor(0x443069));
        break;

      case LocalClassMemberType:
        a->setForeground(QColor(0xae7d00)); //Light orange
        break;
        
      case InheritedClassMemberType:
        a->setForeground(QColor(0x705000)); //Dark orange
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

//       case NamespaceType:
//         // Dark rose
//         a->setForeground(QColor(0x6B2840));
//         break;

      case ErrorVariableType:
        // Pure red
        a->setForeground(QColor(0x8b0019));
        break;

      case ForwardDeclarationType:
        // Gray
        a->setForeground(QColor(0x5C5C5C));
        break;

      case ScopeType:
      case TemplateType:
      case TemplateParameterType:
      case CodeType:
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
    if( color )
      a->setForeground(QColor(color));
  }

  return a;
}

void CppHighlighting::outputRange( KTextEditor::SmartRange * range ) const
{
  kDebug( 9007 ) << range << QString(range->depth(), ' ') << *range << "attr" << range->attribute();
  Q_ASSERT(range->start() <= range->end());
  foreach (SmartRange* child, range->childRanges())
    outputRange(child);
}

ColorMap emptyColorMap() {
  ColorMap ret(validColorCount+1, 0);
 return ret;
}

void CppHighlighting::highlightDUChain(TopDUContext* context) const
{
  kDebug( 9007 ) << "highighting du chain";
  
  DUChainReadLocker lock(DUChain::lock());

  m_contextClasses.clear();
  m_useClassCache = true;
  
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
  
  m_useClassCache = false;
  m_contextClasses.clear();
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

  highlightUses(context);

  foreach (DUContext* child, context->childContexts()) {
    highlightDUChainSimple(child);
  }
}

void CppHighlighting::deleteHighlighting(KDevelop::DUContext* context) const {
  if (!context->smartRange())
    return;

  foreach (Declaration* dec, context->localDeclarations())
    if(dec->smartRange())
      dec->smartRange()->setAttribute(KTextEditor::Attribute::Ptr());

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
    if(!imported || (imported->type() != DUContext::Other && imported->type() != DUContext::Function))
      continue;
    //For now it's enough simply copying them, because we only pass on colors within function bodies.
    if (m_functionColorsForDeclarations.contains(imported))
      colorsForDeclarations = m_functionColorsForDeclarations[imported];
    if (m_functionDeclarationsForColors.contains(imported))
      declarationsForColors = m_functionDeclarationsForColors[imported];
  }

  QList<Declaration*> takeFreeColors;
  
  foreach (Declaration* dec, context->localDeclarations()) {
    //Initially pick a color using the hash, so the chances are good that the same identifier gets the same color always.
    uint colorNum = dec->identifier().hash() % validColorCount;

    if( declarationsForColors[colorNum] ) {
      takeFreeColors << dec; //Use one of the colors that stays free
      continue;
    }

    colorsForDeclarations[dec] = colorNum;
    declarationsForColors[colorNum] = dec;
    
    highlightDeclaration(dec, colors[colorNum]);
  }

  foreach( Declaration* dec, takeFreeColors ) {
    uint colorNum = dec->identifier().hash() % validColorCount;
    uint oldColorNum = colorNum;
    while( declarationsForColors[colorNum] ) {
      colorNum = (colorNum+1) % validColorCount;
      if( colorNum == oldColorNum ) {
        //Could not allocate a unique color, what now? Just pick the black color.
        colorNum = validColorCount;
        break;
      }
    }
    colorsForDeclarations[dec] = colorNum;
    declarationsForColors[colorNum] = dec;
    
    highlightDeclaration(dec, colors[colorNum]);
  }

  for(int a = 0; a < context->uses().count(); ++a) {
    Declaration* decl = context->topContext()->usedDeclarationForIndex(context->uses()[a].m_declarationIndex);
    uint colorNum = validColorCount;
    if( colorsForDeclarations.contains(decl) )
      colorNum = colorsForDeclarations[decl];
    highlightUse(context, a, colors[colorNum]);
  }

  foreach (DUContext* child, context->childContexts())
    highlightDUChain(child,  colorsForDeclarations, declarationsForColors );
  if(context->type() == DUContext::Other || context->type() == DUContext::Function) {
    m_functionColorsForDeclarations[DUContextPointer(context)] = colorsForDeclarations;
    m_functionDeclarationsForColors[DUContextPointer(context)] = declarationsForColors;
  }
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

KDevelop::Declaration* CppHighlighting::localClassFromCodeContext(KDevelop::DUContext* context) const
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

  if( context->owner() && context->owner()->isDefinition() ) {

    if(m_contextClasses.contains(context))
      return m_contextClasses[context];
    
    functionDeclaration = context->owner()->declaration(startContext->topContext());
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

CppHighlighting::Types CppHighlighting::typeForDeclaration(Declaration * dec, DUContext* context) const
{
  /**
   * We highlight in 3 steps by priority:
   * 1. Is the item in the local class or an inherited class? If yes, highlight.
   * 2. What kind of item is it? If it's a type/function/enumerator, highlight by type.
   * 3. Else, highlight by scope.
   * 
   * */

  if(!Cpp::isAccessible(context, dec))
    return ErrorVariableType;
  
  Types type = LocalVariableType;
  if (context) {
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
    if (dec->kind() == Declaration::Type || dec->type<CppFunctionType>() || dec->type<CppEnumeratorType>()) {
      if (dec->isForwardDeclaration())
        type = ForwardDeclarationType;
      else if (dec->type<CppFunctionType>())
          type = FunctionType;
      else if(dec->type<CppClassType>())
          type = ClassType;
      else if(dec->type<CppTypeAliasType>())
          type = TypeAliasType;
      else if(dec->type<CppEnumerationType>())
        type = EnumType;
      else if(dec->type<CppEnumeratorType>())
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

void CppHighlighting::highlightDeclaration(Declaration * declaration, uint color) const
{
  if (SmartRange* range = declaration->smartRange())
    range->setAttribute(attributeForType(typeForDeclaration(declaration, 0), DeclarationContext, color));
}

void CppHighlighting::highlightDeclaration(Declaration * declaration) const
{
  if (SmartRange* range = declaration->smartRange())
    range->setAttribute(attributeForType(typeForDeclaration(declaration, 0), DeclarationContext, 0));
}

void CppHighlighting::highlightUse(DUContext* context, int index, uint color) const
{
  if (SmartRange* range = context->useSmartRange(index)) {
    Types type = ErrorVariableType;
    Declaration* decl = context->topContext()->usedDeclarationForIndex(context->uses()[index].m_declarationIndex);
    if (decl)
      type = typeForDeclaration(decl, context);

    range->setAttribute(attributeForType(type, ReferenceContext, color));
  }
}

void CppHighlighting::highlightUses(DUContext* context) const
{
  for(int a = 0; a < context->uses().count(); ++a) {
    if (SmartRange* range = context->useSmartRange(a)) {
      Types type = ErrorVariableType;
      Declaration* decl = context->topContext()->usedDeclarationForIndex(context->uses()[a].m_declarationIndex);
      if (decl)
        type = typeForDeclaration(decl, context);

      range->setAttribute(attributeForType(type, ReferenceContext, 0));
    }
  }
}

#include "cpphighlighting.moc"
