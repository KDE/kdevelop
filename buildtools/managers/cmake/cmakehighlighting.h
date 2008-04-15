/*****************************************************************************
 * Copyright (c) 2007 Piyush verma <piyush.verma@gmail.com>                  *
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
#ifndef KDEVCMAKEHIGHLIGHTING_H
#define KDEVCMAKEHIGHLIGHTING_H

#include <QObject>
#include <QHash>
#include <QModelIndex>

#include <ktexteditor/attribute.h>
#include <icodehighlighting.h>

namespace KTextEditor
{
    class SmartRange;
}

namespace KDevelop
{
    class DUContext;
    class Declaration;
}

class CMakeHighlighting : public QObject, public KDevelop::ICodeHighlighting
{
    Q_OBJECT
    Q_INTERFACES(KDevelop::ICodeHighlighting)
public:

    enum Types
    {
        FunctionType,
        ClassType,
        NamespaceType,
        FunctionVariableType,
        ClassVariableType,
        NamespaceVariableType
    };
    enum Contexts
    {
        DefinitionContext,
        DeclarationContext,
        NamespaceContext
    };
    CMakeHighlighting(QObject* parent);
    virtual ~CMakeHighlighting();

    void highlightTree(KTextEditor::SmartRange* topRange) const;
    void highlightDUChain(KDevelop::TopDUContext* context) const;

    virtual void highlightDeclaration(KDevelop::Declaration* declaration) const;
    virtual void highlightUses(KDevelop::DUContext*) const;
    KTextEditor::Attribute::Ptr attributeForType(Types type, Contexts context) const;
private:
    void highlightDUChain(KDevelop::DUContext* context) const;
    void outputRange( KTextEditor::SmartRange * range ) const;

    Types typeForDeclaration(KDevelop::Declaration* dec) const;

    mutable QHash<Types, KTextEditor::Attribute::Ptr> m_definitionAttributes;
    mutable QHash<Types, KTextEditor::Attribute::Ptr> m_declarationAttributes;
};

#endif
// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on; auto-insert-doxygen on
