/*****************************************************************************
 * Copyright (c) 2007 Piyush verma <piyush.verma@gmail.com>                  *
 * Copyright (c) 2008 Aleix Pol <aleixpol@gmail.com>                         *
 *                                                                           *
 * Permission is hereby granted, free of charge, to any person obtaining     *
 * a copy of this software and associated documentation files (the           *
 * "Software"), to deal in the Software without restriction, including       *
 * without limitation the rights to use, copy, modify, merge, publish,       *
 * distribute, sublicense, and/or sell copies of the Software, and to        *
 * permit persons to whom the Software is furnished to do so, subject to     *
 * the following conditions:                                                 *
 *                                                                           *
 * The above copyright notice and this permission notice shall be            *
 * included in all copies or substantial portions of the Software.           *
 *                                                                           *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,           *
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF        *
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND                     *
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE    *
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION    *
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION     *
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.           *
 *****************************************************************************/
#include "cmakehighlighting.h"

#include <ktexteditor/smartrange.h>
#include <ktexteditor/smartinterface.h>
#include <ktexteditor/document.h>

#include <language/duchain/topducontext.h>
#include <language/duchain/declaration.h>
#include <language/duchain/use.h>
#include <language/duchain/duchain.h>
#include <language/duchain/duchainlock.h>

using namespace KTextEditor;
using namespace KDevelop;

#define LOCK_SMART(range) KTextEditor::SmartInterface* iface = dynamic_cast<KTextEditor::SmartInterface*>(range->document()); QMutexLocker lock(iface ? iface->smartMutex() : 0);

CMakeHighlighting::CMakeHighlighting( QObject * parent )
  : QObject(parent)
{
}

CMakeHighlighting::~CMakeHighlighting( )
{
}

KTextEditor::Attribute::Ptr CMakeHighlighting::attributeForType( Types type, Contexts ctx) const
{
    KTextEditor::Attribute::Ptr a;
    switch (ctx)
    {
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
    
    if (!a)
    {
        a = KTextEditor::Attribute::Ptr(new KTextEditor::Attribute());
        a->setBackgroundFillWhitespace(true);
        
        QColor c;
        switch (type)
        {
            case NamespaceType:
                c=QColor(Qt::green);
                break;
            case ClassType:
                c=QColor(Qt::blue);
                break;
            case FunctionType:
                c=QColor(Qt::green);
                break;
            case FunctionVariableType:
                c=QColor(Qt::blue);
                break;
             case NamespaceVariableType:
                c=QColor(Qt::red);
                break;
            case ClassVariableType:
                c=QColor(Qt::green);
                break;
            default:
                c=QColor(Qt::green);
                break;
        }
        if(ctx==ReferenceContext)
        {
            a->setForeground(c.dark());
            m_referenceAttributes.insert(type, a);
        }
        else
        {
            a->setBackground(c.light(170));
            a->setFontBold();
            m_definitionAttributes.insert(type, a);
        }
    }
    return a;
}

void CMakeHighlighting::highlightTree( KTextEditor::SmartRange * range ) const
{
    foreach (KTextEditor::SmartRange* child, range->childRanges())
        highlightTree(child);
}

void CMakeHighlighting::outputRange( KTextEditor::SmartRange * range ) const
{
    Q_ASSERT(range->start() <= range->end());
    foreach (SmartRange* child, range->childRanges())
        outputRange(child);
}

void CMakeHighlighting::highlightDUChain(KDevelop::TopDUContext* context) const
{
    DUChainReadLocker lock(DUChain::lock());
    Q_ASSERT(context->topContext() == context);
    highlightDUChain(static_cast<DUContext*>(context));
}

void CMakeHighlighting::highlightDUChain(DUContext* context) const
{

    kDebug() << "Highlighting duchain";
    if (!context->smartRange())
    {
        kDebug() << "Ooops, no smart range, somethings broken";
        return;
    }
    kDebug() << "Highlighting declarations:" << context->localDeclarations();
    foreach (Declaration* dec, context->localDeclarations())
    {
        highlightDeclaration(dec);
    }
    
    highlightUses(context);
    
    kDebug() << "Highlighting child contexts:" << context->childContexts();
    foreach (DUContext* child, context->childContexts())
        highlightDUChain(child);
}


CMakeHighlighting::Types CMakeHighlighting::typeForDeclaration(Declaration * dec) const
{
    Types type=ErrorVariableType;
    switch (dec->context()->type())
    {
      case DUContext::Class:
        type = ClassVariableType;
        break;
      case DUContext::Function:
        type = FunctionVariableType;
        break;
      case DUContext::Namespace:
        type = NamespaceVariableType;
        break;
      default:
        break;
    }
    return type;
}

void CMakeHighlighting::highlightDeclaration(Declaration * declaration) const
{
    if (SmartRange* range = declaration->smartRange())
        range->setAttribute(attributeForType(typeForDeclaration(declaration), DeclarationContext));
}

void CMakeHighlighting::highlightUses(KDevelop::DUContext* context) const
{
    for(int i=0; i<context->usesCount(); i++)
    {
        if (SmartRange* range = context->useSmartRange(i)) {
            Types type = ErrorVariableType;
            Declaration* decl = context->topContext()->usedDeclarationForIndex(context->uses()[i].m_declarationIndex);
            if (decl)
                type = typeForDeclaration(decl);

            LOCK_SMART(range);
            range->setAttribute(attributeForType(type, ReferenceContext));
        }
    }
}

#include "cmakehighlighting.moc"
// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on; auto-insert-doxygen on

