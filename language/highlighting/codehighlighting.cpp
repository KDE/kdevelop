/*
 * This file is part of KDevelop
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

#include "codehighlighting.h"

#include <QApplication>
#include <QPalette>

#include <KColorScheme>
#include <kcolorutils.h>
#include <KTextEditor/SmartRange>
#include <KTextEditor/SmartInterface>
#include <KTextEditor/Document>

#include "../duchain/declaration.h"
#include "../duchain/types/functiontype.h"
#include "../duchain/types/enumeratortype.h"
#include "../duchain/types/typealiastype.h"
#include "../duchain/types/enumerationtype.h"
#include "../duchain/types/structuretype.h"
#include "../duchain/functiondefinition.h"
#include "../duchain/use.h"
#include "../../interfaces/icore.h"
#include "../../interfaces/ilanguagecontroller.h"
#include "../../interfaces/icompletionsettings.h"

using namespace KTextEditor;

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

QList<uint>  colors;
uint validColorCount = 0; //Must always be colors.count()-1, because the last color must be the fallback text color
uint colorOffset = 0; //Maybe make this configurable: An offset where to start stepping through the color wheel
uchar foregroundRatio = 140; ///How the color should be mixed with the foreground color. Between 0 and 255, where 255 means only foreground color, and 0 only the chosen color.
// uchar backgroundRatio = 0; ///Mixing in background color makes the colors less annoying

// uint backgroundTinting = 0;

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
  for(uint a = 0; a < totalColorInterpolationStepCount; ++a)
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
/*
uint backgroundColor() {
  ///@todo Find the correct text background color from kate
  return 0xffffff;
}
*/

uint foregroundColor() {
  ///@todo Find the correct text foreground color from kate! The palette thing below returns some strange other color.
  ///@todo store the scheme somewhere (or only the colors we need) and update when neccessary for better performance
  ///      esp. we need to adapt to changes in the colorscheme of kate => needs new interface
  return KColorScheme(QPalette::Normal, KColorScheme::View).foreground(KColorScheme::NormalText).color().rgb();
}

uint blend(uint color) {
  return KColorUtils::tint( foregroundColor(), color, float(foregroundRatio) / float(0xff) ).rgb();
}

void generateColors(int count) {
  colors.clear();
  uint step = totalColorInterpolationSteps() / count;
  uint currentPos = colorOffset;
  kDebug() << "text color:" << (void*) foregroundColor();
  for(int a = 0; a < count; ++a) {
    colors.append( blend(interpolate( currentPos )) );
    kDebug() << "color" << a << "interpolated from" << currentPos << " < " << totalColorInterpolationSteps() << ":" << (void*) colors.last();
    currentPos += step;
  }
  colors.append(foregroundColor());
  validColorCount = colors.count()-1;
}

void regenerateColors() {
  foregroundRatio = 255-ICore::self()->languageController()->completionSettings()->localVariableColorizationLevel();
  generateColors(10);
}

CodeHighlighting::CodeHighlighting( QObject * parent )
  : QObject(parent), m_localColorization(true), m_useClassCache(false)
{
  regenerateColors();
}

CodeHighlighting::~CodeHighlighting( )
{
}

class CodeHighlightingColors : public KDevelop::ConfigurableHighlightingColors {
  public:
  CodeHighlightingColors() : KDevelop::ConfigurableHighlightingColors("KDev Semantic Highlighting") {
    {
      KTextEditor::Attribute::Ptr a(new KTextEditor::Attribute);
      a->setForeground(QColor(blend(0x005912))); //Dark green
      addAttribute(CodeHighlighting::ClassType, a);
      kDebug() << "Class color: " << (void*) 0x005912 << "->" << (void*) a->foreground().color().rgb();
    }
    {
      KTextEditor::Attribute::Ptr a(new KTextEditor::Attribute);
      a->setForeground(QColor(blend(0x35938d)));
      addAttribute(CodeHighlighting::TypeAliasType, a);
      kDebug() << "TypeAlias color: " << (void*) 0x35938d << "->" << (void*) a->foreground().color().rgb();
    }
    {
      KTextEditor::Attribute::Ptr a(new KTextEditor::Attribute);
      a->setForeground(QColor(blend(0x6c101e))); //Dark red
      addAttribute(CodeHighlighting::EnumType, a);
      kDebug() << "Enum color: " << (void*) 0x6c101e << "->" << (void*) a->foreground().color().rgb();
    }
    {
      KTextEditor::Attribute::Ptr a(new KTextEditor::Attribute);
      a->setForeground(QColor(blend(0x862a38))); //Greyish red
      addAttribute(CodeHighlighting::EnumeratorType, a);
      kDebug() << "Enumerator color: " << (void*) 0x862a38 << "->" << (void*) a->foreground().color().rgb();
    }
    {
      KTextEditor::Attribute::Ptr a(new KTextEditor::Attribute);
      a->setForeground(QColor(blend(0x21005A))); // Navy blue
      addAttribute(CodeHighlighting::FunctionType, a);
      kDebug() << "Function color: " << (void*) 0x21005A << "->" << (void*) a->foreground().color().rgb();
    }
    {
      KTextEditor::Attribute::Ptr a(new KTextEditor::Attribute);
      a->setForeground(QColor(blend(0x443069)));// Dark Burple (blue / purple)
      addAttribute(CodeHighlighting::MemberVariableType, a);
      kDebug() << "MemberVariable color: " << (void*) 0x443069 << "->" << (void*) a->foreground().color().rgb();
    }
    {
      KTextEditor::Attribute::Ptr a(new KTextEditor::Attribute);
      a->setForeground(QColor(blend(0xae7d00))); //Light orange
      addAttribute(CodeHighlighting::LocalClassMemberType, a);
      kDebug() << "LocalClassMember color: " << (void*) 0xae7d00 << "->" << (void*) a->foreground().color().rgb();
    }
    {
      KTextEditor::Attribute::Ptr a(new KTextEditor::Attribute);
      a->setForeground(QColor(blend(0x705000))); //Dark orange
      addAttribute(CodeHighlighting::InheritedClassMemberType, a);
      kDebug() << "InheritedClassMember color: " << (void*) 0x705000 << "->" << (void*) a->foreground().color().rgb();
    }
    {
      KTextEditor::Attribute::Ptr a(new KTextEditor::Attribute);
      a->setForeground(QColor(blend(0x0C4D3C)));
      addAttribute(CodeHighlighting::LocalVariableType, a);
      kDebug() << "LocalVariable color: " << (void*) 0x0C4D3C << "->" << (void*) a->foreground().color().rgb();
    }
    {
      KTextEditor::Attribute::Ptr a(new KTextEditor::Attribute);
      a->setForeground(QColor(blend(0x300085))); // Less dark navy blue
      addAttribute(CodeHighlighting::FunctionVariableType, a);
      kDebug() << "FunctionVariable color: " << (void*) 0x300085 << "->" << (void*) a->foreground().color().rgb();
    }
    {
      KTextEditor::Attribute::Ptr a(new KTextEditor::Attribute);
      a->setForeground(QColor(blend(0x9F3C5F))); // Rose
      addAttribute(CodeHighlighting::NamespaceVariableType, a);
      kDebug() << "NamespaceVariable color: " << (void*) 0x9F3C5F << "->" << (void*) a->foreground().color().rgb();
    }
    {
      KTextEditor::Attribute::Ptr a(new KTextEditor::Attribute);
      a->setForeground(QColor(blend(0x12762B))); // Grass green
      addAttribute(CodeHighlighting::GlobalVariableType, a);
      kDebug() << "GlobalVariable color: " << (void*) 0x12762B << "->" << (void*) a->foreground().color().rgb();
    }
    {
      KTextEditor::Attribute::Ptr a(new KTextEditor::Attribute);
      a->setForeground(QColor(blend(0x6B2840))); // Dark rose
      addAttribute(CodeHighlighting::NamespaceType, a);
      kDebug() << "Namespace color: " << (void*) 0x6B2840 << "->" << (void*) a->foreground().color().rgb();
    }
    {
      KTextEditor::Attribute::Ptr a(new KTextEditor::Attribute(*defaultAttribute()));
//       a->setForeground(QColor(0x8b0019)); // Pure red
      a->setUnderlineColor(QColor(blend(0x8b0019))); // Pure red
      a->setUnderlineStyle(QTextCharFormat::WaveUnderline);
      addAttribute(CodeHighlighting::ErrorVariableType, a);
      kDebug() << "ErrorVariable underline color: " << (void*) 0x8b0019 << "->" << (void*) a->underlineColor().rgb();
    }
    {
      KTextEditor::Attribute::Ptr a(new KTextEditor::Attribute);
      a->setForeground(QColor(blend(0x5C5C5C))); // Gray
      addAttribute(CodeHighlighting::ForwardDeclarationType, a);
      kDebug() << "ForwardDeclaration color: " << (void*) 0x5C5C5C << "->" << (void*) a->foreground().color().rgb();
    }
/*      case ScopeType:
      case TemplateType:
      case TemplateParameterType:
      case CodeType:
      case FileType:*/

  }
};

static CodeHighlightingColors& configurableColors() {
    static CodeHighlightingColors colors;
    return colors;
}

KTextEditor::Attribute::Ptr CodeHighlighting::attributeForType( Types type, Contexts context, uint color ) const
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

    a = KTextEditor::Attribute::Ptr(new KTextEditor::Attribute(*configurableColors().getAttribute(type)));

    switch (context) {
      case DefinitionContext:
      case DeclarationContext:
        a->setFontBold();
        break;
    }

    if( color ) {
      a->setForeground(QColor(color));
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
  ColorMap ret(validColorCount+1, 0);
 return ret;
}

void CodeHighlighting::highlightDUChain(TopDUContext* context) const
{
  kDebug() << "highighting du chain";

  DUChainReadLocker lock(DUChain::lock());

  regenerateColors();

  if(!ICore::self()->languageController()->completionSettings()->semanticHighlightingEnabled()) {
    kDebug() << "highighting disabled";
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
    uint color = 0;
    if( colorsForDeclarations.contains(decl) )
      color = colors[colorsForDeclarations[decl]];
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

void CodeHighlighting::highlightDeclaration(Declaration * declaration, uint color) const
{
  if (SmartRange* range = declaration->smartRange()) {
    LOCK_SMART(range);
    range->setAttribute(attributeForType(typeForDeclaration(declaration, 0), DeclarationContext, color));
  }
}

void CodeHighlighting::highlightUse(DUContext* context, int index, uint color) const
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
        range->setAttribute(attributeForType(type, ReferenceContext, 0));
    else
        range->setAttribute(KTextEditor::Attribute::Ptr());
    }
  }
}

}

#include "codehighlighting.moc"
