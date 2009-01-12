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
#include <ktexteditor/smartinterface.h>
#include <ktexteditor/document.h>
#include <kcolorutils.h>
#include <language/duchain/topducontext.h>
#include <language/duchain/declaration.h>
#include <language/duchain/use.h>
#include <language/duchain/functiondefinition.h>
#include "cpptypes.h"
#include <language/duchain/duchain.h>
#include <language/duchain/duchainlock.h>
#include "cpplanguagesupport.h"
#include "cppduchain.h"
#include <language/editor/hashedstring.h>
#include <language/duchain/classmemberdeclaration.h>
#include <interfaces/icore.h>
#include <interfaces/ilanguagecontroller.h>
#include <interfaces/icompletionsettings.h>

using namespace KTextEditor;
using namespace KDevelop;

#define LOCK_SMART(range) KTextEditor::SmartInterface* iface = dynamic_cast<KTextEditor::SmartInterface*>(range->document()); QMutexLocker lock(iface ? iface->smartMutex() : 0);

namespace KDevelop {
class ConfigurableHighlightingColors {
  public:
    ConfigurableHighlightingColors(QString highlightingName) : m_highlightingName(highlightingName) {
      KTextEditor::Attribute::Ptr a(new KTextEditor::Attribute);
      setDefaultAttribute(a);
    }
    
    void addAttribute(int number, KTextEditor::Attribute::Ptr attribute) {
      m_attributes[number] = attribute;
    }
    
    KTextEditor::Attribute::Ptr getAttribute(int number) const {
      return m_attributes[number];
    }
    
    void setDefaultAttribute(KTextEditor::Attribute::Ptr defaultAttrib) {
      m_defaultAttribute = defaultAttrib;
    }
    
    KTextEditor::Attribute::Ptr defaultAttribute() const {
      return m_defaultAttribute;
    }
    
  private:
    KTextEditor::Attribute::Ptr m_defaultAttribute;
    QHash<int, KTextEditor::Attribute::Ptr> m_attributes;
    QString m_highlightingName;
};
}

QList<uint>  colors;
uint validColorCount = 0; //Must always be colors.count()-1, because the last color must be the fallback text color
uint colorOffset = 0; //Maybe make this configurable: An offset where to start stepping through the color wheel
uchar foregroundRatio = 140; ///@todo this needs a knob in the configuration: How the color should be mixed with the foreground color. Between 0 and 255, where 255 means only foreground color, and 0 only the chosen color.
uchar backgroundRatio = 0; ///Mixing in background color makes the colors less annoying

///@param ratio ratio between 0 and 0xff
uint mix(uint color1, uint color2, uchar ratio) {
  return KColorUtils::mix(QColor(color1), QColor(color2), float(ratio) / float(0xff)).rgb();
  //return (((quint64)color1) * (((quint64)0xff) - ratio) + ((quint64)color2) * ratio) / (quint64)0xff;
}

uint totalColorInterpolationStepCount = 6;
uint interpolationWaypoints[] = {0xff0000, 0xff9900, 0x00ff00, 0x00aaff, 0x0000ff, 0xaa00ff};
//Do less steps when interpolating to/from green: Green is very dominant, and different mixed green tones are hard to distinguish(and always seem green).
uint interpolationLengths[] = {0xff, 0xff, 0xbb, 0xbb, 0xbb, 0xff};

uint totalColorInterpolationSteps() {
  uint ret = 0;
  for(int a = 0; a < totalColorInterpolationStepCount; ++a)
    ret += interpolationLengths[a];
  return ret;
}

///Generates a color from the color wheel. @param step Step-number, one of totalColorInterpolationSteps
uint interpolate(uint step) {

  uint waypoint = 0;
  while(step > interpolationLengths[waypoint]) {
    step -= interpolationLengths[waypoint];
    ++waypoint;
  }

  uint nextWaypoint = (waypoint + 1) % totalColorInterpolationStepCount;

  return mix(interpolationWaypoints[waypoint], interpolationWaypoints[nextWaypoint], (step * 0xff) / interpolationLengths[waypoint]);
}

void generateColors(int count) {
  colors.clear();
  ///@todo Find the correct text foreground color from kate! The palette thing below returns some strange other color.
  uint standardColor(0u); //QApplication::palette().foreground().color().rgb());
  uint backgroundColor(0xffffff);
  uint step = totalColorInterpolationSteps() / count;
  uint currentPos = colorOffset;
  kDebug() << "text color:" << (void*)QApplication::palette().text().color().rgb();
  for(int a = 0; a < count; ++a) {
    kDebug() << "color" << a << "interpolated from" << currentPos << " < " << totalColorInterpolationSteps() << ":" << (void*)interpolate( currentPos );
    colors.append( mix(mix(interpolate( currentPos ), backgroundColor, backgroundRatio), standardColor, foregroundRatio) );
    //colors.append( interpolate(currentPos).rgb() );
    currentPos += step;
  }
  colors.append(standardColor);
  validColorCount = colors.count()-1;
}

void regenerateColors() {
  foregroundRatio = 255-ICore::self()->languageController()->completionSettings()->localVariableColorizationLevel();
  generateColors(10);
}

CppHighlighting::CppHighlighting( QObject * parent )
  : QObject(parent), m_localColorization(true), m_useClassCache(false)
{
  regenerateColors();
}

CppHighlighting::~CppHighlighting( )
{
}

class CppHighlightingColors : public KDevelop::ConfigurableHighlightingColors {
  public:
  CppHighlightingColors() : KDevelop::ConfigurableHighlightingColors("C++ Semantic Highlighting") {

    {
      KTextEditor::Attribute::Ptr a(new KTextEditor::Attribute);
      a->setForeground(QColor(0x005912)); //Dark green
      addAttribute(CppHighlighting::ClassType, a);
    }
    {
      KTextEditor::Attribute::Ptr a(new KTextEditor::Attribute);
      a->setForeground(QColor(0x00981e)); //Lighter greyish green
      addAttribute(CppHighlighting::TypeAliasType, a);
    }
    {
      KTextEditor::Attribute::Ptr a(new KTextEditor::Attribute);
      a->setForeground(QColor(0x6c101e)); //Dark red
      addAttribute(CppHighlighting::EnumType, a);
    }
    {
      KTextEditor::Attribute::Ptr a(new KTextEditor::Attribute);
      a->setForeground(QColor(0x862a38)); //Greyish red
      addAttribute(CppHighlighting::EnumeratorType, a);
    }
    {
      KTextEditor::Attribute::Ptr a(new KTextEditor::Attribute);
      a->setForeground(QColor(0x21005A)); // Navy blue
      addAttribute(CppHighlighting::FunctionType, a);
    }
    {
      KTextEditor::Attribute::Ptr a(new KTextEditor::Attribute);
      a->setForeground(QColor(0x443069));// Dark Burple (blue / purple)
      addAttribute(CppHighlighting::MemberVariableType, a);
    }
    {
      KTextEditor::Attribute::Ptr a(new KTextEditor::Attribute);
      a->setForeground(QColor(0xae7d00)); //Light orange
      addAttribute(CppHighlighting::LocalClassMemberType, a);
    }
    {
      KTextEditor::Attribute::Ptr a(new KTextEditor::Attribute);
      a->setForeground(QColor(0x705000)); //Dark orange
      addAttribute(CppHighlighting::InheritedClassMemberType, a);
    }
    {
      KTextEditor::Attribute::Ptr a(new KTextEditor::Attribute);
      a->setForeground(QColor(0x0C4D3C));
      addAttribute(CppHighlighting::LocalVariableType, a);
    }
    {
      KTextEditor::Attribute::Ptr a(new KTextEditor::Attribute);
      a->setForeground(QColor(0x300085)); // Less dark navy blue
      addAttribute(CppHighlighting::FunctionVariableType, a);
    }
    {
      KTextEditor::Attribute::Ptr a(new KTextEditor::Attribute);
      a->setForeground(QColor(0x9F3C5F)); // Rose
      addAttribute(CppHighlighting::NamespaceVariableType, a);
    }
    {
      KTextEditor::Attribute::Ptr a(new KTextEditor::Attribute);
      a->setForeground(QColor(0x12762B)); // Grass green
      addAttribute(CppHighlighting::GlobalVariableType, a);
    }
    {
      KTextEditor::Attribute::Ptr a(new KTextEditor::Attribute);
      a->setForeground(QColor(0x6B2840)); // Dark rose
      addAttribute(CppHighlighting::NamespaceType, a);
    }
    {
      KTextEditor::Attribute::Ptr a(new KTextEditor::Attribute(*defaultAttribute()));
//       a->setForeground(QColor(0x8b0019)); // Pure red
      a->setUnderlineColor(QColor(0x8b0019)); // Pure red
      a->setUnderlineStyle(QTextCharFormat::WaveUnderline);
      addAttribute(CppHighlighting::ErrorVariableType, a);
    }
    {
      KTextEditor::Attribute::Ptr a(new KTextEditor::Attribute);
      a->setForeground(QColor(0x5C5C5C)); // Gray
      addAttribute(CppHighlighting::ForwardDeclarationType, a);
    }
/*      case ScopeType:
      case TemplateType:
      case TemplateParameterType:
      case CodeType:
      case FileType:*/
    
  }
};

static CppHighlightingColors configurableColors;

KTextEditor::Attribute::Ptr CppHighlighting::attributeForType( Types type, Contexts context, uint color ) const
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

  if (!a || color ) {
    
    a = KTextEditor::Attribute::Ptr(new KTextEditor::Attribute(*configurableColors.getAttribute(type)));

    switch (context) {
      case DefinitionContext:
      case DeclarationContext:
        a->setFontBold();
        break;
    }

    if( color ) {
      a->setForeground(QColor(color));
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

  regenerateColors();
  
  if(!ICore::self()->languageController()->completionSettings()->semanticHighlightingEnabled()) {
    kDebug( 9007 ) << "highighting disabled";
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

void CppHighlighting::highlightDUChain(DUContext* context, QHash<Declaration*, uint> colorsForDeclarations, ColorMap declarationsForColors) const
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

  for(int a = 0; a < context->usesCount(); ++a) {
    Declaration* decl = context->topContext()->usedDeclarationForIndex(context->uses()[a].m_declarationIndex);
    uint colorNum = validColorCount;
    if( colorsForDeclarations.contains(decl) )
      colorNum = colorsForDeclarations[decl];
    highlightUse(context, a, colors[colorNum]);
  }

  foreach (DUContext* child, context->childContexts())
    highlightDUChain(child,  colorsForDeclarations, declarationsForColors );
  if(context->type() == DUContext::Other || context->type() == DUContext::Function) {
    m_functionColorsForDeclarations[IndexedDUContext(context)] = colorsForDeclarations;
    m_functionDeclarationsForColors[IndexedDUContext(context)] = declarationsForColors;
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

CppHighlighting::Types CppHighlighting::typeForDeclaration(Declaration * dec, DUContext* context) const
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
    if (dec->kind() == Declaration::Type || dec->type<KDevelop::FunctionType>() || dec->type<KDevelop::EnumeratorType>()) {
      if (dec->isForwardDeclaration())
        type = ForwardDeclarationType;
      else if (dec->type<KDevelop::FunctionType>())
          type = FunctionType;
      else if(dec->type<CppClassType>())
          type = ClassType;
      else if(dec->type<CppTypeAliasType>())
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

void CppHighlighting::highlightDeclaration(Declaration * declaration, uint color) const
{
  if (SmartRange* range = declaration->smartRange()) {
    LOCK_SMART(range);
    range->setAttribute(attributeForType(typeForDeclaration(declaration, 0), DeclarationContext, color));
  }
}

void CppHighlighting::highlightUse(DUContext* context, int index, uint color) const
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

void CppHighlighting::highlightUses(DUContext* context) const
{
  for(int a = 0; a < context->usesCount(); ++a) {
    if (SmartRange* range = context->useSmartRange(a)) {
      Types type = ErrorVariableType;
      Declaration* decl = context->topContext()->usedDeclarationForIndex(context->uses()[a].m_declarationIndex);
      if (decl)
        type = typeForDeclaration(decl, context);

    LOCK_SMART(range);
    if(type != ErrorVariableType || ICore::self()->languageController()->completionSettings()->highlightSemanticProblems())
        range->setAttribute(attributeForType(type, ReferenceContext, 0));
    else
        range->setAttribute(KTextEditor::Attribute::Ptr());
    }
  }
}

#include "cpphighlighting.moc"
