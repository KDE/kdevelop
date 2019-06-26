/*
 *   KDevelop outline view
 *   Copyright 2010, 2015 Alex Richardson <alex.richardson@gmx.de>
 *
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Lesser General Public
 *   License as published by the Free Software Foundation; either
 *   version 2.1 of the License, or (at your option) any later version.
 *
 *   This library is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *   Lesser General Public License for more details.
 *
 *   You should have received a copy of the GNU Lesser General Public
 *   License along with this library; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */
#include "outlinenode.h"

#include <language/duchain/duchainutils.h>
#include <language/duchain/ducontext.h>
#include <language/duchain/declaration.h>
#include <language/duchain/duchain.h>
#include <language/duchain/duchainlock.h>
#include <language/duchain/classfunctiondeclaration.h>
#include <language/duchain/functiondefinition.h>
#include <language/duchain/types/alltypes.h>
#include <language/duchain/namespacealiasdeclaration.h>
#include <language/duchain/classdeclaration.h>
#include <language/duchain/forwarddeclaration.h>

#include <KTextEditor/CodeCompletionModel>

#include <debug.h>

using namespace KDevelop;

OutlineNode::OutlineNode(const QString& text, OutlineNode* parent)
    : m_cachedText(text)
    , m_parent(parent)
{
}

OutlineNode::OutlineNode(DUContext* ctx, const QString& name, OutlineNode* parent)
    : m_cachedText(name)
    , m_declOrContext(ctx)
    , m_parent(parent)
{
    KTextEditor::CodeCompletionModel::CompletionProperties prop;
    switch (ctx->type()) {
        case KDevelop::DUContext::Class:
            prop |= KTextEditor::CodeCompletionModel::Class;
            break;
        case KDevelop::DUContext::Enum:
            prop |= KTextEditor::CodeCompletionModel::Enum;
            break;
        case KDevelop::DUContext::Function:
            prop |= KTextEditor::CodeCompletionModel::Function;
            break;
        case KDevelop::DUContext::Namespace:
            prop |= KTextEditor::CodeCompletionModel::Namespace;
            break;
        case KDevelop::DUContext::Template:
            prop |= KTextEditor::CodeCompletionModel::Template;
            break;
        default:
            break;
    }
    m_cachedIcon = DUChainUtils::iconForProperties(prop);
    appendContext(ctx, ctx->topContext());
}


OutlineNode::OutlineNode(Declaration* decl, OutlineNode* parent)
    : m_declOrContext(decl)
    , m_parent(parent)
{
    // qCDebug(PLUGIN_OUTLINE) << "Adding:" << decl->qualifiedIdentifier().toString() << ": " <<typeid(*decl).name();

    // TODO: properly qualified identifier for out of line function definitions
    m_cachedText = decl->identifier().toString();
    m_cachedIcon = DUChainUtils::iconForDeclaration(decl);
    if (auto* alias = dynamic_cast<NamespaceAliasDeclaration*>(decl)) {
        //e.g. C++ using namespace statement
        m_cachedText = alias->importIdentifier().toString();
    }
    else if (auto* member = dynamic_cast<ClassMemberDeclaration*>(decl)) {
        if (member->isFriend()) {
            m_cachedText = QLatin1String("friend ") + m_cachedText;
        }
    }
    if (AbstractType::Ptr type = decl->abstractType()) {
        //add the (function return) type at the end (after a colon - like UML)
        //so that the first thing seen is the name of the function/variable
        //and not the (function return) type
        AbstractType::WhichType typeEnum = type->whichType();
        switch (typeEnum) {
        case AbstractType::TypeFunction: {
            FunctionType::Ptr func = type.cast<FunctionType>();
            // func->partToString() does not add the argument names -> do it manually
            if (DUContext* fCtx = DUChainUtils::functionContext(decl)) {
                m_cachedText += QLatin1Char('(');
                bool first = true;
                const auto childDecls = fCtx->localDeclarations(decl->topContext());
                for (Declaration* childDecl : childDecls) {
                    if (first) {
                        first = false;
                    } else {
                        m_cachedText += QStringLiteral(", ");
                    }

                    if (childDecl->abstractType()) {
                        m_cachedText += childDecl->abstractType()->toString();
                    }
                    auto ident = childDecl->identifier();
                    if (!ident.isEmpty()) {
                        m_cachedText += QLatin1Char(' ') +  ident.toString();
                    }

                }
                m_cachedText += QLatin1Char(')');
            } else {
                qCWarning(PLUGIN_OUTLINE) << "Missing function context:" << decl->qualifiedIdentifier().toString();
                m_cachedText += func->partToString(FunctionType::SignatureArguments);
            }
            //constructors/destructors have no return type, a trailing semicolon would look stupid
            if (func->returnType()) {
                m_cachedText += QLatin1String(" : ") + func->partToString(FunctionType::SignatureReturn);
            }
            return; // don't append any children here!
        }
        case AbstractType::TypeEnumeration:
            //no need to append the fully qualified type
            break;
        case AbstractType::TypeEnumerator:
            //no need to append the fully qualified type
            Q_ASSERT(decl->type<EnumeratorType>());
            m_cachedText += QLatin1String(" = ") + decl->type<EnumeratorType>()->valueAsString();
            break;
        case AbstractType::TypeStructure: {
            //this seems to be the way it has to be done (after grepping through source code)
            //TODO shouldn't there be some kind of isFriend() functionality?
            static IndexedIdentifier friendIdentifier(Identifier(QStringLiteral("friend")));
            const bool isFriend = decl->indexedIdentifier() == friendIdentifier;
            if (isFriend) {
                //FIXME There seems to be no way of finding out whether the friend is class/struct/etc
                m_cachedText += QLatin1Char(' ') + type->toString();
            }
            break;
        }
        case AbstractType::TypeAlias: {
            //append the type it aliases
            TypeAliasType::Ptr alias = type.cast<TypeAliasType>();
            if (AbstractType::Ptr targetType = alias->type()) {
                m_cachedText += QLatin1String(" : ") + targetType->toString();
            }
        }
        break;
        default:
            QString typeStr = type->toString();
            if (!typeStr.isEmpty()) {
                m_cachedText += QLatin1String(" : ") + typeStr;
            }
        }
    }


    //these two don't seem to be hit
    if (decl->isAutoDeclaration()) {
        m_cachedText = QLatin1String("Implicit: ") + m_cachedText;
    }
    if (decl->isAnonymous()) {
        m_cachedText = QLatin1String("<anonymous>") + m_cachedText;
    }

    if (DUContext* ctx = decl->internalContext()) {
        appendContext(ctx, decl->topContext());
    }
    if (m_cachedText.isEmpty()) {
        m_cachedText = i18nc("An anonymous declaration (class, function, etc.)", "<anonymous>");
    }
}

std::unique_ptr<OutlineNode> OutlineNode::dummyNode()
{
    return std::unique_ptr<OutlineNode>(new OutlineNode(QStringLiteral("<dummy node>"), nullptr));
}

std::unique_ptr<OutlineNode> OutlineNode::fromTopContext(TopDUContext* ctx)
{
    auto result = dummyNode();
    result->appendContext(ctx, ctx);
    return result;
}

void OutlineNode::appendContext(DUContext* ctx, TopDUContext* top)
{
    // qDebug() << ctx->scopeIdentifier().toString() << "context type=" << ctx->type();
    const auto childDecls = ctx->localDeclarations(top);
    for (Declaration* childDecl : childDecls) {
        if (childDecl) {
            m_children.emplace_back(childDecl, this);
        }
    }
    bool certainlyRequiresSorting = false;
    const auto childContexts = ctx->childContexts();
    for (DUContext* childContext : childContexts) {
        if (childContext->owner()) {
            // if there is a onwner, this will already have been handled by the loop above
            // TODO: is this always true? With my testing so far it seems to be
            // qDebug() << childContext->scopeIdentifier(true).toString()
            //        << " has an owner declaration: " << childContext->owner()->toString() << "-> skip";
            continue;
        }
        QVector<Declaration*> decls = childContext->localDeclarations(top);
        if (decls.isEmpty()) {
            continue;
        }
        // we now know that we will have o sort since we appended a node in the wrong order
        certainlyRequiresSorting = true;
        QString ctxName = childContext->scopeIdentifier(true).toString();
        // if child context is a template context or if name is empty append to current list,
        // otherwise create a new context node
        if (childContext->type() == DUContext::Template || ctxName.isEmpty()) {
            //append all subcontexts to this node
            appendContext(childContext, top);
        } else {
            // context without matching declaration, for example the definition of
            // "class Foo::Bar if it was forward declared in a namespace before:
            // namespace Foo { class Bar; }
            // class Foo::Bar { ... };
            // TODO: icon and location for the namespace
            if (childContext->type() == DUContext::ContextType::Helper) {
                // This context could be for a definition of an existing class method.
                // If we don't merge all those context end up with a tree like this:
                //  +-+- FooClass
                //  | \-- method1()
                //  +-+- FooClass
                //  | \-- method2()
                //  \ OtherStuff
                auto it = std::find_if(m_children.begin(), m_children.end(), [childContext](const OutlineNode& node) {
                    if (auto* ctx = dynamic_cast<DUContext*>(node.duChainObject())) {
                        return ctx->equalScopeIdentifier(childContext);
                    }
                    return false;
                });
                if (it != m_children.end()) {
                    it->appendContext(childContext, top);
                }
                else {
                    // TODO: get the correct icon for the context
                    m_children.emplace_back(childContext, ctxName, this);
                }
            } else {
                // just add the context
                m_children.emplace_back(childContext, ctxName, this);
            }
        }
    }
    // we now need to sort since sometimes the elements from ctx->localDeclarations(top)
    // are not in the order they appear in the source. Additionally, if we had any child
    // contexts that were added, they will be at the end of the list
    // and need to be moved to the correct location. In that case certainlyRequiresSorting
    // will be true and we can pass it to sortByLocation() to skip the std::is_sorted() call
    sortByLocation(certainlyRequiresSorting);
}

void OutlineNode::sortByLocation(bool requiresSorting)
{

    if (m_children.size() <= 1) {
        return;
    }
    // TODO: does it make sense to cache m_declOrContext->range().start?
    // adds 8 bytes to each node, but save a lot of pointer lookups when sorting
    // qDebug("sorting children of %s (%p) by location", qPrintable(m_cachedText), this);
    auto compare = [](const OutlineNode& n1, const OutlineNode& n2) -> bool {
        // nodes without decl always go at the end
        if (!n1.m_declOrContext) {
            return false;
        } else if (!n2.m_declOrContext) {
            return true;
        }
        return n1.m_declOrContext->range().start < n2.m_declOrContext->range().start;
    };
    // since most nodes will be correctly sorted we check that before calling std::sort().
    // This saves a lot of move ctor/assignment calls in the common case.
    // If we appended a context without a Declaration* we know that it will be unsorted
    // so we can pass requiresSorting = true to skip the useless std::is_sorted() call.
    // uncomment the following qDebug() lines to see whether this optimization really makes sense
    if (requiresSorting || !std::is_sorted(m_children.begin(), m_children.end(), compare)) {
        // qDebug("Need to sort node %s(%p)", qPrintable(m_cachedText), this);
        std::sort(m_children.begin(), m_children.end(), compare);
    } else {
        // qDebug("Node %s(%p) was sorted!", qPrintable(m_cachedText), this);
    }
}

OutlineNode::~OutlineNode()
{
}
